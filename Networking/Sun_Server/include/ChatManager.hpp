#ifndef CHATMANAGER_HPP
#define CHATMANAGER_HPP

#include "sun_net.hpp"
#include "AuthManager.hpp"

namespace sun
{
	class ChatHandler
	{
	public:
		ChatHandler(Authorizer& auth)
		:authorizer(auth)
		{}
		
		void HandleTextPacket(UserConnection_ptr connection, std::shared_ptr<Message> mesg, UserID& receiver_id)
		{
			if(!authorizer.isValidConnection(connection->GetID()))
			{
				std::cout<<"["<<connection->GetID()<<"] Invalid Connection packet discarded\n";
				return;
			}
			
			
			ChatMesg flag;
			*mesg >> flag;

			switch(flag)
			{
				case ChatMesg::TEXT :
					HandleTextMesg(connection, mesg, receiver_id);
					break;
				
				case ChatMesg::FILE_MESG :
					break;
					
				default :
					break;
			};
		}
		
		void HandleRoomRequestPacket(UserConnection_ptr connection, std::shared_ptr<Message> mesg, UserID& receiver_id)
		{
			
			if(!authorizer.isValidConnection(connection->GetID()))
			{
				std::cout<<"["<<connection->GetID()<<"] Invalid Connection packet discarded\n";
				return;
			}
			
			
			RoomRequest flag;
			*mesg >> flag;
			
			switch(flag)
			{
				case RoomRequest::FRIEND_REQUEST :
					HandleFriendRequest(connection, mesg, receiver_id);
					break;
				case RoomRequest::FRIEND_REQ_ACCEPT:
					HandleReqAccepted(connection, mesg, receiver_id);
					break;
				case RoomRequest::FRIEND_REQ_REJECT:
					HandleReqRejected(connection, mesg, receiver_id);
					break;
					
				default :
					break;
			};
			
		}
		
		
	private:
		void HandleTextMesg(UserConnection_ptr connection, std::shared_ptr<Message> mesg, UserID& receiver_id)
		{
			
			UserID const sender_id = authorizer.get<UserID>(connection->GetID());
			
			
			mesg->GetData(&receiver_id);
			
			if(!authorizer.isAuthenticated(receiver_id))
			{
				std::cout <<"["<<connection->GetID()<<"] mesg to unkown user - packet discarded\n";
				return;
			}
			
			UserInfo_ptr receiver_info = authorizer.template get<UserInfo_ptr>(receiver_id);
			
			if(!receiver_info->isFriendOf(sender_id))
			{
				std::cout <<"["<<connection->GetID()<<"] ender is not a friend of receiver - packet discarded\n";
				return;
			}
			
			mesg->SerializeArray(sender_id.data(),sender_id.size());
			*mesg << (uint32_t)sender_id.size() << ChatMesg::TEXT;
		
			return;
		}
		
		void HandleFriendRequest(UserConnection_ptr connection, std::shared_ptr<Message> mesg, UserID& receiver_id)
		{
			UserID const sender_id = authorizer.get<UserID>(connection->GetID());
			try
			{
				mesg->GetData(&receiver_id);
			}
			catch(...)
			{
				std::cout <<"["<<connection->GetID()<<"] HandleFriendRequest failed\n";
			}
			
			if(!authorizer.isAuthenticated(receiver_id))
			{
				std::cout <<"["<<connection->GetID()<<"] Friend_Req to unknown user - packet discarded\n";
				return;
			}
			
			mesg->SerializeArray(sender_id.data(),sender_id.size());
			*mesg << (uint32_t)sender_id.size() << RoomRequest::FRIEND_REQUEST; 
			
			authorizer.template get<UserInfo_ptr>(sender_id)->StoreSentFriendRequest(receiver_id);
			authorizer.template get<UserInfo_ptr>(receiver_id)->StoreRevcFriendRequest(sender_id);
			
		}
		
		void HandleReqAccepted(UserConnection_ptr connection, std::shared_ptr<Message> mesg, UserID& receiver_id)
		{
			UserID const sender_id = authorizer.get<UserID>(connection->GetID());
			
			try
			{
			mesg->GetData(&receiver_id);
			}
			catch(...)
			{
				std::cout <<"["<<connection->GetID()<<"] HandleReqAccepted failed \n";
			}
			
			if(!authorizer.isAuthenticated(receiver_id))
			{
				std::cout <<"["<<connection->GetID()<<"] HandleReqAccepted receiver not authorized \n";
				return;
			}
			
			mesg->SerializeArray(sender_id.data(), sender_id.size());
			*mesg << (uint32_t)sender_id.size();
			*mesg << RoomRequest::FRIEND_REQ_ACCEPT;
			authorizer.template get<UserInfo_ptr>(receiver_id)->AddFriend(sender_id);
			authorizer.template get<UserInfo_ptr>(sender_id)->AddFriend(receiver_id);
			
		}
		
		void HandleReqRejected(UserConnection_ptr connection, std::shared_ptr<Message> mesg, UserID& receiver_id)
		{
			//pending......
		}
		
		
	private:
		Authorizer& authorizer;
	};
}

#endif 