#ifndef USER_INFO_CONTAINER_HPP
#define USER_INFO_CONTAINER_HPP

#include "sun_net.hpp"

namespace sun
{
	namespace net
	{
		template <typename T>
		class user_info
		{
		public:
			user_info()
			{}
			
			user_info(std::string id, std::string name)
			: m_user_id(id), m_userName(name)
			{}
			
			~user_info()
			{}
			
			void Serialize(message<T>& mesg)
			{
				
				for(auto& d : m_roomRequestOut)
				{
					mesg.SerializeArray(d.data(), d.size());
					mesg << (uint32_t)d.size();
				}
				mesg << (uint32_t)m_roomRequestOut.size();
				
				for(auto& d : m_roomRequestIn)
				{
					mesg.SerializeArray(d.data(), d.size());
					mesg << (uint32_t)d.size();
				}
				mesg << (uint32_t)m_roomRequestIn.size();
			
				for(auto& d : m_rooms)
				{
					mesg.SerializeArray(d.data(), d.size());
					mesg << (uint32_t)d.size();
				}
				mesg << (uint32_t)m_rooms.size();
				
				mesg.SerializeArray(m_userName.data(), m_userName.size());
				mesg << (uint32_t)m_userName.size();
				
				mesg.SerializeArray(m_user_id.data(), m_user_id.size());
				mesg << (uint32_t)m_user_id.size();
				
			}
			
			void DeSerilaize(message<T>& mesg)
			{
				try
				{
					mesg.GetData(&m_user_id);
					mesg.GetData(&m_userName);
					
					int size = 0;
					mesg >> size;
					std::string temp_data;
					for(int i = 0; i < size; i++)
					{
						mesg.GetData(&temp_data);
						m_rooms.push_back(temp_data);
					}
					
					mesg >> size;
					for(int i = 0; i < size; i++)
					{
						mesg.GetData(&temp_data);
						m_roomRequestIn.push_back(temp_data);
					}
					
					mesg >>size;
					for(int i = 0; i < size; i++)
					{
						mesg.GetData(&temp_data);
						m_roomRequestOut.push_back(temp_data);
					}
				
				}
				catch(...)
				{
					std::cout <<"Deserialization failed\n";
				}			
				
			}
			
			bool isFriendOf(const std::string& uid)
			{
				return (std::find(m_rooms.begin(), m_rooms.end(), uid) != std::end(m_rooms));
			}
			
			void AddFriend(const std::string& new_friend)
			{
				if(auto d = std::find(m_roomRequestIn.begin(), m_roomRequestIn.end(), new_friend); d != std::end(m_roomRequestIn))
					m_roomRequestIn.erase(d);
				
				if(auto d = std::find(m_roomRequestOut.begin(), m_roomRequestOut.end(), new_friend); d != std::end(m_roomRequestOut))
					m_roomRequestOut.erase(d);
				
				m_rooms.push_back(new_friend);
			}
			
			void RemoveFriend(const std::string& friend_id)
			{
				if(auto d = std::find(m_rooms.begin(), m_rooms.end(), friend_id); d != std::end(m_rooms))
					m_rooms.erase(d);
			}
			
			void StoreRevcFriendRequest(const std::string& request_revc)
			{
				m_roomRequestIn.push_back(request_revc);
			}
			void StoreSentFriendRequest(const std::string& request_sent)
			{
				m_roomRequestOut.push_back(request_sent);
			}
			
			void StoreFriendRequestReject(const std::string& user_id)
			{
				if(auto d = std::find(m_roomRequestIn.begin(), m_roomRequestIn.end(), user_id); d != std::end(m_roomRequestIn))
				m_roomRequestIn.erase(d);
				
				if(auto d = std::find(m_roomRequestOut.begin(), m_roomRequestOut.end(), user_id); d != std::end(m_roomRequestOut))
				m_roomRequestOut.erase(d);
			}
			
			bool isIncomingRequest(const std::string& uid)
			{
				return (std::find(m_roomRequestIn.begin(), m_roomRequestIn.end(), uid) != std::end(m_roomRequestIn));
			}
			
			bool isOutgoingRequest(const std::string& uid)
			{
				return (std::find(m_roomRequestOut.begin(), m_roomRequestOut.end(), uid) != std::end(m_roomRequestOut));
			}
			
			void clear()
			{
				m_user_id.clear();
				m_userName.clear();
				m_rooms.clear();
				m_roomRequestIn.clear();
				m_roomRequestOut.clear();
			}
			
		public:
			std::string m_user_id;
			std::string m_userName;
			std::list<std::string> m_rooms;
			std::list<std::string> m_roomRequestIn;
			std::list<std::string> m_roomRequestOut;
			
		};
	}
}

#endif 