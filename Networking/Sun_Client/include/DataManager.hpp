#ifndef DATAMANAGER_HPP
#define DATAMANAGER_HPP

#include "sun_net.hpp"

namespace sun
{
	struct ChatMessage
	{
		std::string message;
		bool status; // true for receive    false for send
		
		ChatMessage()
		{}
		ChatMessage(std::string& text, bool status)
		: message(std::move(text)), status(status)
		{}
	};
	
	class DataReceiver
	{
	public:
		DataReceiver(std::atomic<bool>& log_status, net::user_info<Header>& user,
		ThreadSafeQueue<net::owned_mesg<Header>>& qMesgIn, 
		Database<UserID, std::shared_ptr<MessageList<ChatMessage>>>& roomData)
		: m_isLogged(log_status), m_user(user), m_qMesgIn(qMesgIn), m_roomData(roomData)
		{
			ConsumeIncomingMessages();
		}
		
		
		void ConsumeIncomingMessages()
		{
			asio::post(
			[this]()
			{
				std::shared_ptr<net::owned_mesg<Header>> item = m_qMesgIn.wait_and_pop();
				net::message<Header> mesg = std::move(item->mesg);
				
				switch(mesg.header.id)
				{
					case Header::QUERY:
						HandleQuery(mesg);
						break;
					
					case Header::ROOM_REQUEST:
						HandleRoomRequest(mesg);
						break;
					
					case Header::SYS_MESSAGE:
						HandleSystem(mesg);
						break;
					
					case Header::FILE_MESSAGE:
						//HandleFileMessage(mesg);
						break;
					
					case Header::CHAT_MESSAGE:
						HandleChatMessage(mesg);
						break;
					
					default:
						
						break;
				};
				ConsumeIncomingMessages();
			});
		}
		
		void HandleRoomRequest(net::message<Header>& mesg)
		{
			RoomRequest request;
			mesg >> request;
			
			UserID uid;
			try{
				mesg.GetData(&uid);
			}
			catch(...)
			{
				std::cerr <<"HandleRoomRequest GetData failed\n";
				return;
			}
			switch(request)
			{
				case RoomRequest::FRIEND_REQUEST:
					m_user.StoreRevcFriendRequest(uid);
					break;
					
				case RoomRequest::FRIEND_REQ_ACCEPT:
					m_roomData.set_new_value(uid, std::make_shared<MessageList<ChatMessage>>());
					m_user.AddFriend(uid);
					break;
					
				case RoomRequest::FRIEND_REQ_REJECT:
					m_user.StoreFriendRequestReject(uid);
					break;
					
				default :
					break;
			};
		}
		
		void HandleSystem(net::message<Header>& mesg)
		{
			SysRequest req;
			mesg >> req;
			
			std::string error_mesg;
			
			switch(req)
			{
				case SysRequest::LOG_NACK:
					
				case SysRequest::SIGN_NACK:
					mesg.GetData(&error_mesg);
					std::cerr << error_mesg <<"\n";
					break;
				
				case SysRequest::LOG_ACK:
					m_user.DeSerilaize(mesg);
					
					for(const std::string& uid : m_user.m_rooms)
					{
						m_roomData.set_new_value(uid, std::make_shared<MessageList<ChatMessage>>());
					}
					
					m_isLogged = true;
					break;
				
				default:
					break;
			};
			
		}
		
		void HandleChatMessage(net::message<Header>& mesg)
		{
			uint32_t tries = 10;
			while(!m_isLogged && tries--)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			
			if(!m_isLogged)
			{
				std::cerr << "Chat Message Discarded Reason - user not logged\n";
				return;
			}
			
			ChatMesg mesg_type;
			mesg >> mesg_type;
			if(mesg_type != ChatMesg::TEXT)	return;
			
			UserID sender_uid;
			
			try{
				mesg.GetData(&sender_uid);
			}
			catch(...)
			{
				std::cerr <<"HandleChatMessage GetData1 failed\n";
				return;
			}
			
			if(!m_user.isFriendOf(sender_uid)) return;
			
			ChatMessage chat_mesg;
			chat_mesg.status = true;
			
			try{
				mesg.GetData(&chat_mesg.message);
			}
			catch(...)
			{
				std::cerr <<"HandleChatMessage GetData2 failed\n";
				return;
			}
			
			m_roomData.get<std::shared_ptr<MessageList<ChatMessage>>>(sender_uid)->push_back(chat_mesg);
			
		}
		
		void HandleQuery(net::message<Header>& mesg)
		{
			Query query;
			mesg >> query;
			if(query != Query::SEARCH_QUERY)	return;
		
			uint32_t size;
			mesg >> size;
			m_queryResult.resize(size);
			
			std::string temp;
			
			try{
				for(uint32_t i = 0; i<size; i++)
				{
					mesg.GetData(&temp);
					m_queryResult[i] = std::move(temp);
				}
			}
			catch(...)
			{
				std::cerr <<"HandleQuery GetData failed\n";
				return;
			}
			
			m_isQueryDataReady = true;
		}
		
		
		bool isQueryDataReady()
		{
			return m_isQueryDataReady;
		}
		
		std::vector<std::string>& GetQueryResult()
		{
			m_isQueryDataReady = false;
			return m_queryResult;
		}
		
	private:
		std::atomic<bool>& m_isLogged;
		net::user_info<Header>& m_user;
		
		ThreadSafeQueue<net::owned_mesg<Header>>& m_qMesgIn;
		Database<UserID, std::shared_ptr<MessageList<ChatMessage>>>& m_roomData;
		
		bool m_isQueryDataReady = false;
		std::vector<std::string> m_queryResult;
	
	};
}
#endif