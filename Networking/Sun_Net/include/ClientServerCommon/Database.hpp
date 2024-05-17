#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "common.hpp"

namespace sun
{
	template <typename Key, class... Value>
	class Database
	{
	public:
	
		struct value_type
		{
		public:
			
			value_type()
			: data()
			{}
			
			template <class... Args>
			value_type(const Args&... args)
			: data()
			{
				set(args...);
			}
			
			template<typename T>
			T& get()
			{
				return std::get<T>(data);
			}
			
			template <typename First, class... Rest>
			void set(First first, const Rest&... rest)
			{
				set(first);
				set(rest...);
			}
			
			template<typename T>
			void set(const T& val)
			{
				std::get<T>(data) = val;
			}
			
			std::tuple<Value...>& GetData()
			{
				return data;
			}
			
		private:
			std::tuple<Value...> data;
			
		};
	
		typedef typename std::unordered_map<Key,value_type>::iterator db_iterator;
	
	public:
	
		template <class... types>
		void set_new_value(Key key,const types&... args)
		{
			map.emplace(key, value_type(args...));
		}
		
		template <typename type>
		void set(Key key, const type& val)
		{
			map[key].template set<type>(val);
		}
		
		template <typename T>
		T& get(Key key)
		{
			return map[key].template get<T>();
		}
		
		bool contains(Key key)
		{
			return map.contains(key);
		}
		
		db_iterator find(Key key)
		{
			 return map.find(key);
		}
		
		db_iterator end()
		{
			return map.end();
		}
		
		void erase(Key key)
		{
			map.erase(key);
		}
		void erase(db_iterator it)
		{
			map.erase(it);
		}
		
	private:
		std::unordered_map<Key, value_type> map; 
	};
	
}


#endif