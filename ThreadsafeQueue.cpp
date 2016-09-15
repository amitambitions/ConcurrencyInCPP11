#include <stack>
#include <mutex>
#include <memory>
#include <queue>
#include <iostream>

using namespace std;

template<typename T>
// TODO: provide support for remaining functions as well.
// TODO: complete the code.
class ThreadSafeQueue {

	bool empty();
	int size();
	// Check that it is imp to write these "const" version of the functions.
	T& front();
	const T& front() const;
	T& back();
	const T& back() const;

	// Check that we dont check here whether the queue is full or not because we use
	// std::queue internally, and we leave upto this container to worry about memory
	// to store new items.
	// TODO handle this case when we make our own queue rather than using std::queue
	void push(const T x) {
		std::lock_guard<std::mutex> guard(m);
		queue.push(x);
		cond.notify_one();
	}

	// It is imp to understand that why we don't want pop() to return the element as
	// well as remove it from the queue. Sutter explained that better in exception
	// safety case, refer that but here is the summary. If the copy constructor of T
	// throws on return, you have already altered the state of the queue (top_position
	// in my naive implementation) and the element is removed from the queue (and not
	// returned). For all intents and purposes (no matter how you catch the exception
	// in client code) the element at the top of the queue is lost.
	//
	// This implementation is also inefficient in the case when you do not need the popped
	// value (i.e. it creates a copy of the element that nobody will use). Now we can say
	// that we can make it a requirement that copy constructor of T should not throw an
	// exception.
	// TODO: check this point that why this is not feasible.
	// T pop();
	void pop(T& x) {
		// We need unique_lock not lock_guard because if the condition is not true, then
		// we need to unlock the mutex, and only unique_lock provides this support.
		std::unique_lock<std::mutex> lock(m);
		cond.wait(lock, [this]{return !queue.empty();});

		x = queue.front();
	    queue.pop();
	}

private:
	std::queue<T> queue;

	// It is imp to make sure that the mutex here is mutable because of concurrency reasons.
	mutable std::mutex m;

	// Since we expect this queue to be used as a message passing system between two threads
	// we need a signaling mechanism.
	// TODO:  why is mutex mutable but not the condition variable?
	std::condition_variable cond;
};
