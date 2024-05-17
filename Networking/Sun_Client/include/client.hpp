#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "sun_net.hpp"
#include "DataManager.hpp"

using namespace sun::net;
namespace sun
{
	class client :public net::client_interface<Header>
	{	
	public:
		client()
		: m_isLogged(false), m_receiver(m_isLogged, m_user, IncomingQ(), m_roomData)
		{}
		
		void SendTextPacket(std::string text)
		{
			if(!m_currentRoomData)
			{
				std::cerr << "SendTextPacket Failed - currently no room is opened\n";
				return;
			}
			
			sun::net::message<sun::Header> mesg;
			mesg.header.id = sun::Header::CHAT_MESSAGE;	
			mesg.SerializeArray(text.data(),text.size());
			mesg << (uint32_t)text.size();
			mesg.SerializeArray(m_currentRoomID.data(), m_currentRoomID.size());
			mesg << (uint32_t)m_currentRoomID.size() << ChatMesg::TEXT;
			Send(mesg);
			
			ChatMessage message(text, 0);
			m_currentRoomData->push_back(message);
		}
		
		void SendSignUpPacket(UserID uid, std::string name, std::string pass)
		{
			sun::net::message<Header> signup;
			signup.header.id = Header::SYS_MESSAGE;
			signup.SerializeArray(name.data(),name.size());
			signup << (uint32_t)name.size();
			signup.SerializeArray(pass.data(),pass.size());
			signup << (uint32_t)pass.size();
			signup.SerializeArray(uid.data(),uid.size());
			signup << (uint32_t)uid.size() << SysRequest::SIGNUP;
			Send(signup);
		}
		
		void SendLoginPacket(UserID uid,std::string pass)
		{
			net::message<Header> login;
			login.header.id = Header::SYS_MESSAGE;
			login.SerializeArray(uid.data(),uid.size());
			login << (uint32_t)uid.size() ;
			login.SerializeArray(pass.data(),pass.size());
			login << (uint32_t)pass.size() << SysRequest::LOGIN;
			Send(login);
		}
		
		void SendRoomRequestTo(UserID uid)
		{
			if(m_user.isFriendOf(uid) || m_user.isOutgoingRequest(uid)) return;
		
			m_user.StoreSentFriendRequest(uid);
			
			net::message<Header> request;
			request.header.id = Header::ROOM_REQUEST;
			request.SerializeArray(uid.data(),uid.size());
			request << (uint32_t)uid.size() << RoomRequest::FRIEND_REQUEST;
			Send(request);
		}
	
		void AcceptRoomRequestOf(UserID uid)
		{
			if(!m_user.isIncomingRequest(uid) && m_user.isFriendOf(uid))
			{
				std::cerr << "AcceptRoomRequestOf failed - accpeting request of an invalid client\n";
				return;
			}
			
			m_roomData.set_new_value(uid, std::make_shared<MessageList<ChatMessage>>());
			m_user.AddFriend(uid);
			
			net::message<Header> request;
			request.header.id = Header::ROOM_REQUEST;
			request.SerializeArray(uid.data(),uid.size());
			request << (uint32_t)uid.size() << RoomRequest::FRIEND_REQ_ACCEPT;
			Send(request);
		}
		
		void RejectRoomRequestOf(UserID uid)
		{
			
		}
		
		
		
		std::vector<std::string> GetQueryResultFor(std::string query_message, Query query_type = Query::SEARCH_QUERY)
		{
			SendQuery(query_message, query_type);
			uint32_t tries = 30;
			while(!m_receiver.isQueryDataReady())
			{
                if(!tries--) return std::vector<std::string>();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			
			return std::move(m_receiver.GetQueryResult());
		}
		
		bool EnterRoom(UserID uid)
		{
			if(!m_user.isFriendOf(uid))
			{
				return false;
			}
			
			m_currentRoomID = uid;
			m_currentRoomData = m_roomData.get<std::shared_ptr<MessageList<ChatMessage>>>(uid);
			m_currentRoomData->reset_cur_ptr();
			return true;
			
		}
		
		void ExitRoom()
		{
			if(m_currentRoomData)
				m_currentRoomData->reset_cur_ptr();
			
			m_currentRoomID.resize(0);
			m_currentRoomData = nullptr;
		}
		
		bool isLogged()
		{
			uint32_t tries = 10;
			
			while(tries-- && !m_isLogged)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(300));
			}
			
			return m_isLogged;
		}
		
		void Logout()
		{
			m_isLogged = false;
			m_user.clear();
		}
		
		std::shared_ptr<ChatMessage> CurrentRoomNextMessage()
		{
			if(!m_currentRoomData)
				return std::shared_ptr<ChatMessage>();
			
			return m_currentRoomData->get_next_value_until(500);
		}
		
		const std::list<std::string>& GetRoomList()
		{
			return m_user.m_rooms;
		}
		
		const std::list<std::string>& GetRecvRoomRequestList()
		{
			return m_user.m_roomRequestIn;
		}
		
		const std::list<std::string>& GetSendRoomRequestList()
		{
			return m_user.m_roomRequestOut;
		}
		
		std::string CurrentRoomID()
		{
			return m_currentRoomID;
		}
		std::string MyID()
		{
			return m_user.m_user_id;
		}

		std::string MyName()
		{
			return m_user.m_userName;
		}

	private:
		void SendQuery(std::string query_message, Query query_type = Query::SEARCH_QUERY)
		{
			if(!m_isLogged)
			{
				return;
			}
			
			net::message<Header> query_request;
			query_request.header.id = Header::QUERY;
			query_request.SerializeArray(query_message.data(),query_message.size());
			query_request << (uint32_t)query_message.size() << uint32_t(0) << query_type;
			Send(query_request);
			
		}
	
	private:
		std::atomic<bool> m_isLogged;
		
		UserID m_currentRoomID;
		std::shared_ptr<MessageList<ChatMessage>> m_currentRoomData;
		
		user_info<Header> m_user;
		Database<UserID, std::shared_ptr<MessageList<ChatMessage>>> m_roomData;
		
		DataReceiver m_receiver;
		
		
	};
}

#endif
