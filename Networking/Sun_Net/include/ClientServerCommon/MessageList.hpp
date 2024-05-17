#ifndef MESSAGELIST_HPP
#define MESSAGELIST_HPP

#include "common.hpp"

namespace sun
{
	template <typename T>
	class MessageList
	{
	private:
		struct node
		{
			T data;
			std::shared_ptr<node> next;
			std::mutex node_mutex;
		
			node()
			{}
			
			node(T val)
			: data(val)
			{}
		};
		
	public:
		MessageList()
		: head(std::make_unique<node>()), tail(head.get()), current_ptr(head.get())
		{}
	
		void push_front(T val)
		{
			
		}
	
		void push_back(T& val)
		{
			std::shared_ptr<node> new_tail = std::make_shared<node>();
			{
				std::lock_guard<std::mutex> tail_lock(tail->node_mutex);
				tail->data = std::move(val);
				tail->next = new_tail;
				tail = new_tail.get();
			}
			cv.notify_all();
		}
		
		
		void erase(uint32_t index)
		{
			
		}
		
		void update(T new_val)
		{
			
		}
		
		void reset_cur_ptr()
		{
			current_ptr = head.get();
		}
		
		std::shared_ptr<T> get_next_value()
		{
			std::unique_lock<std::mutex> tail_lock(tail->node_mutex);
			
			cv.wait(tail_lock, [this]{
					return (current_ptr != tail);
				});
			
			T data = current_ptr->data;
			
			current_ptr = current_ptr->next.get();
			return std::make_shared<T>(data);
		}
		
		std::shared_ptr<T> get_next_value_until(uint32_t ms = 1000)
		{
			std::unique_lock<std::mutex> tail_lock(tail->node_mutex);
			
			bool status = cv.wait_until(tail_lock, std::chrono::system_clock::now() + std::chrono::milliseconds(ms), [this]{
					return (current_ptr != tail);
				});
			
			if(!status)
				return nullptr;
			
			T data = current_ptr->data;
			
			current_ptr = current_ptr->next.get();
			return std::make_shared<T>(data);
		}
		
		
		std::shared_ptr<T> try_get_next_value()
		{
			std::lock_guard<std::mutex> tail_lock(tail->node_mutex);
			
			if(current_ptr == tail)
				return std::shared_ptr<T>();
				
			T data = current_ptr->data;
			
			current_ptr = current_ptr->next.get();
			return std::make_shared<T>(data);
		}
		
	private:
		std::unique_ptr<node> head;
		node* tail;
		
		node* current_ptr;
		
		std::condition_variable cv;
	};
}


#endif 