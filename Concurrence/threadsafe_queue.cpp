#include "threadsafe_queue.h"

using namespace std;

template<typename T> threadsafe_queue<T>::threadsafe_queue()
{}

template<typename T> threadsafe_queue<T>::~threadsafe_queue()
{}

template<typename T> threadsafe_queue<T>::threadsafe_queue(const threadsafe_queue &other)
{
	lock_guard<mutex> lk(mut);
	data_queue = other.data_queue;
}

template<typename T> void threadsafe_queue<T>::push(T new_value)
{
	lock_guard<mutex> lk(mut);
	data_queue.push(new_value);
	data_cond.notify_one();
}

template<typename T> void threadsafe_queue<T>::wait_and_pop(T& value)
{
	unique_lock<mutex> lk(mut);
	data_cond.wait(lk, [this]{return !data_queue.empty(); });
	value = data_queue.front();
	data_queue.pop();
}

template<typename T> shared_ptr<T> threadsafe_queue<T>::wait_and_pop()
{
	unique_lock<mutex> lk(mut);
	data_cond.wait(lk, [this]{return !data_queue.empty(); });

	shared_ptr<T> res(make_shared<T>(data_queue.front()));
	data_queue.pop();
	return res;
}

template<typename T> bool threadsafe_queue<T>::try_pop(T& value)
{
	lock_guard<mutex> lk(mut);
	if (data_queue.empty())
		return false;

	value = data_queue.front();
	data_queue.pop();
	return true;
}

template<typename T> shared_ptr<T> threadsafe_queue<T>::try_pop()
{
	lock_guard<mutex> lk(mut);
	if (data_queue.empty())
		return shared_ptr<T>();

	shared_ptr<T> res(make_shared<T>(data_queue.front()));
	data_queue.pop();
	return res;
}

template<typename T> bool threadsafe_queue<T>::empty() const
{
	lock_guard<mutex> lk(mut);
	return data_queue.empty();
}


