#ifndef THREADSAFEQUEUE_HPP
#define THREADSAFEQUEUE_HPP

#include "common.hpp"

template <typename T>
class ThreadSafeQueue
{
	struct node
	{
		std::shared_ptr<T> data;
		std::unique_ptr<node> next;
	};
	
	std::unique_ptr<node> head;
	std::mutex head_mutex;
	node* tail;
	std::mutex tail_mutex;
	std::condition_variable cv_data;
	
	node* get_tail()
	{
		std::lock_guard<std::mutex> tail_lock(tail_mutex);
		return tail;
	}
	
	std::unique_ptr<node> pop_head()
	{
		std::unique_ptr<node> old_head(std::move(head));
		head = std::move(old_head->next);		
		return old_head;
	}
	
	std::unique_ptr<node> try_pop_head()
	{
		std::lock_guard<std::mutex> lock_head(head_mutex);
		if(head.get() == get_tail())
			return std::unique_ptr<node>();
		
		return pop_head();
	}
	
	std::unique_lock<std::mutex> wait_for_data(){
		std::unique_lock<std::mutex> head_lock(head_mutex);
		cv_data.wait(head_lock,[&]{
			return head.get() != get_tail();
		});
		return std::move(head_lock);
	}
	
	std::unique_ptr<node> wait_pop_head()
	{
		std::unique_lock<std::mutex> head_lock(wait_for_data());
		return pop_head();
	}
	
public:
	
	ThreadSafeQueue()
	: head(new node), tail(head.get())
	{}
	
	std::shared_ptr<T> try_pop();
	std::shared_ptr<T> wait_and_pop();
	void push(T new_val);
	void push(std::shared_ptr<T> new_data);
	void push_front(T new_val);
	bool empty();
};

template<typename T>
std::shared_ptr<T> ThreadSafeQueue<T>::wait_and_pop()
{
	std::unique_ptr<node> old_head(std::move(wait_pop_head()));
	return old_head->data;
}


template <typename T>
std::shared_ptr<T> ThreadSafeQueue<T>::try_pop()
{
	std::unique_ptr<node> old_head(std::move(try_pop_head()));
	return old_head?old_head->data:std::shared_ptr<T>();
}

template <typename T>
void ThreadSafeQueue<T>::push(T new_val)
{
	
	std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_val)));
		
	std::unique_ptr<node> p(new node);
	node* new_tail = p.get();
	{
		std::lock_guard<std::mutex> tail_lock(tail_mutex);
		tail->data = new_data;
		tail->next = std::move(p);
		tail = new_tail;
	}
	cv_data.notify_one();
}

template <typename T>
void ThreadSafeQueue<T>::push(std::shared_ptr<T> new_data)
{
	
	//std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_val)));
		
	std::unique_ptr<node> p(new node);
	node* new_tail = p.get();
	{
		std::lock_guard<std::mutex> tail_lock(tail_mutex);
		tail->data = new_data;
		tail->next = std::move(p);
		tail = new_tail;
	}
	cv_data.notify_one();
}



template <typename T>
void ThreadSafeQueue<T>::push_front(T new_val)
{

	std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_val)));
	std::unique_ptr<node> new_head(new node);
	new_head->data = new_data;
	
	{
		std::lock_guard<std::mutex> head_lock(head_mutex);
		new_head->next = std::move(head);
		head = std::move(new_head);
	}
	cv_data.notify_one();
}

template <typename T>
bool ThreadSafeQueue<T>::empty()
{
	std::lock_guard<std::mutex> headLock(head_mutex);
	return (head.get() == get_tail());
}

#endif