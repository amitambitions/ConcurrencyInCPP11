// This is part of some learnings from Concurrency book.

#include <stack>
#include <mutex>
#include <memory>
#include <iostream>

using namespace std;

// Check that C++ STL containers does not provide thread safety, so if
// the containers need to be accessed by multiple threads then we have
// two options.
// 1) We let each thread exclusively lock before calling the APIs of
// std::stack. This way we don't need any wrapper around the stack class.
// But this is error prone because the caller may forget to lock before
// accessing the stack or it may result into some deadlock if not carefully
// worked on. One possible deadlock can result from the below steps:
//
// 1-a) If the caller forgets to lock then he can call following APIs in the
// given order
//		is_empty()
//		x = top()
//		.....
//		pop()
//
// Thread 2 does a pop() between is_empty() check and top() call by above
// thread, and now when above thread will execute, it will result into
// undefined behavior because top() on an empty item is undefined behavior.
//
// 1-b) Caller knows to lock but interleaves the statements and takes and release
// lock at different moments during execution, which can create potential race
// condition.
// thread 1 does following:
//		lock(m)
//			is_empty() .. suppose it is not empty
//		unlock(m)
//
//		.... some execution in between
//		lock(m)
//			x = top()
//			pop()
//		unlock(m)
//
// Now also same problem can happen because the caller was lousy with proper
// way of locking.
//
// 1-c) Now suppose we provide a wrapper class that locks every API before
// executing, so that the caller does not need to provide worry about locking
// and for the caller interface remains almost same as that of std::stack.
// So now all APIs internally acquire the locks, but even now the same problem
// as case a can happen because another thread can intervene. So the issue here
// is that the interface itself is not good enough.
//
// Solution: merge problematic APIS.
template<typename T>
class threadsafe_stack
{
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack& other)
    {
        std::lock_guard<std::mutex> lock(other.m);
        data=other.data;
    }
    threadsafe_stack& operator=(const threadsafe_stack&) = delete;

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(m);
        data.push(new_value);
    }

    // TODO: normally we dont prefer to use exceptions in production code, so what is the ideal way
    // of informing to client when the stack is empty?
    // 1) maybe return a nullptr here?
    std::shared_ptr<T> pop()
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty())
        	cout << "You are accessing an empty stack.";

        std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
        data.pop();
        return res;
    }
    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m);
        if(data.empty())
        	cout << "You are accessing an empty stack.";

        // See that now the pop() API, which is exposed to the user has merged top()
        // and pop() actions done on std::stack so that the caller does not see an
        // intermediate state.
        value=data.top();
        data.pop();
    }
    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};

int main()
{
    threadsafe_stack<int> si;
    si.push(5);
    si.pop();
    if(!si.empty())
    {
        int x;
        si.pop(x);
    }
}
