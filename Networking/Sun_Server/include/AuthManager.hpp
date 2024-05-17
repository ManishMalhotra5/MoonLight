#ifndef AUTHMANAGER_HPP
#define AUTHMANAGER_HPP

#include "sun_net.hpp"

namespace sun
{
	class Authorizer 
	{
	public:
		Authorizer()
			: m_userDB()
		{

		}

		bool isAuthenticated(UserID& uid)
		{
			return m_userDB.contains(uid);
		}

		bool isValidConnection(int connection_id)
		{
			return m_validConnection.contains(connection_id);
		}
		
		template <typename Key, class... Value>
		void AuthenticateUser(Key key, Value&... args)
		{
			m_userDB.set_new_value(key, args...);
			int connection_id = m_userDB.template get<UserConnection_ptr>(key)->GetID();
			
			m_userSet.insert(key);
			
			ValidateConnection(connection_id, key);
		}
		
		void ValidateConnection(int con_id, const UserID& key)
		{
			m_validConnection.set_new_value(con_id, key);
		}
		
		
		template <typename Value>
		Value& get(int connection_id)
		{
			return (m_validConnection.get<Value>(connection_id));
		}
		template <typename Value>
		Value& get(UserID uid)
		{
			return (m_userDB.get<Value>(uid));
		}
		
		template <typename Value>
		void set(int connection_id, const Value& arg)
		{
			m_validConnection.set<Value>(connection_id, arg);
		}
		
		template <typename Value>
		void set(UserID uid, Value arg)
		{
			m_userDB.set<Value>(uid, arg);
		}
		
		
		void InvalidateConnection(int connection_id)
		{
			auto it = m_validConnection.find(connection_id);

			if (it == m_validConnection.end())
				return;
			UserID key = it->second.template get<UserID>();
			m_validConnection.erase(it);
			m_userDB.template set<UserConnection_ptr>(key,nullptr);

		}
		template <class... Value>
		std::tuple<Value...>& GetUserDetails(UserID uid)
		{
			return m_userDB.find(uid)->second.GetData();
		}
		
		template <typename Key>
		void SearchUsers_hint(const Key& target, std::vector<Key>& keys, uint32_t at_most_users = 20, uint32_t start_point = 0)
		{
			auto it = std::find_if(m_userSet.begin(), m_userSet.end(), [&target](const std::string& value)
			{
				if(target.size() && target == std::string(value.data(), target.size()))
					return true;
				return false;
			});
			
			if(std::distance(it, m_userSet.end()) > at_most_users*start_point)
				std::advance(it, at_most_users * start_point);
			else
				return;
				
			for( ; it != m_userSet.end() && keys.size() <= at_most_users ; it++)
			{
				if(target != std::string(it->data(), target.size()))
					break;
				keys.push_back(*it);
			}
			
		}
		
		
	private:
		Database<UserID, UserConnection_ptr, UserDataQueue_ptr, UserInfo_ptr,Password> m_userDB;
		Database<int, UserID> m_validConnection;
		
		std::set<UserID> m_userSet;
		
	};
}

#endif