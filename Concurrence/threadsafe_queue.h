#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template <typename T>
class threadsafe_queue
{

private:
	mutable std::mutex mut;
	std::queue<T> data_queue;
	std::condition_variable data_cond;

public:
	threadsafe_queue();
	threadsafe_queue(const threadsafe_queue &other);
	~threadsafe_queue();

	void push(T new_value);
	void wait_and_pop(T& value);
	std::shared_ptr<T> wait_and_pop();
	bool try_pop(T& value);
	std::shared_ptr<T> try_pop();
	bool empty() const;
};

