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
	T& front();
	const T& front() const;
	T& back();
	const T& back() const;
	
	void push(const T x) {
		std::lock_guard<std::mutex> guard(m);
		queue.push(x);
		cond.notify_one();
	}

	void pop(T& x) {
		std::unique_lock<std::mutex> lock(m);
		cond.wait(lock, [this]{return !queue.empty();});

		x = queue.front();
	    queue.pop();
	}

private:

	std::queue<T> queue;
	mutable std::mutex m;
	std::condition_variable cond;
};
