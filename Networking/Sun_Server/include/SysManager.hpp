#ifndef SYSMANAGER_HPP
#define SYSMANAGER_HPP

#include "sun_net.hpp"
#include "AuthManager.hpp"

namespace sun
{
	class SysHandler
	{
	public:
		SysHandler(Authorizer& auth)
		:authorizer(auth)
		{}
		
		void HandlePacket(UserConnection_ptr connection,std::shared_ptr<Message> mesg)
		{
			SysRequest flag ;
			*mesg >> flag;
			
			switch(flag)
			{
				case SysRequest::LOGIN :
					LoginUser(connection, mesg);
					break;
				
				case SysRequest::SIGNUP :
					SignupUser(connection, mesg);
					break;
					
				default :
					break;
			};
		}
		
		void LoginUser(UserConnection_ptr connection,std::shared_ptr<Message> mesg)
		{
			UserID uid;
			Password pass;
			try
			{
				mesg->GetData(&pass, &uid);
			}
			catch(std::exception& e)
			{
				SysRequestNotAck(connection, "log-in failed for unknown reasons", SysRequest::LOG_NACK);
				return;
			}
			
			if(!authorizer.isAuthenticated(uid))
			{
				//send login_not_ack with error message
				SysRequestNotAck(connection, "mistake in user_id", SysRequest::LOG_NACK);
				return;
			}
			
			//auto& tuple_data  = authorizer.GetUserDetails<UserConnection_ptr, UserDataQueue_ptr, UserInfo_ptr, Password>(uid);
			
			if(authorizer.get<Password>(uid) != pass)
			{
				//send login_not_ack with message wrong password
				SysRequestNotAck(connection, "wrong password", SysRequest::LOG_NACK);
				return;
			}
			
			//send login_ack with account data
			authorizer.set<UserConnection_ptr>(uid, connection);
			connection->SetOutQueue(authorizer.get<UserDataQueue_ptr>(uid));
			authorizer.ValidateConnection(connection->GetID(),uid);
			
			net::message<Header> log_ack_packet;
			log_ack_packet.header.id = Header::SYS_MESSAGE;
			authorizer.get<UserInfo_ptr>(uid)->Serialize(log_ack_packet);
			log_ack_packet << SysRequest::LOG_ACK;
			
			connection->SendPriorityMesg(log_ack_packet);
		}
		
		void SysRequestNotAck(UserConnection_ptr connection, std::string error_mesg, SysRequest nack_flag)
		{
			net::message<Header> not_ack_packet;
			not_ack_packet.header.id = Header::SYS_MESSAGE;
			
			not_ack_packet.SerializeArray(error_mesg.data(), error_mesg.size());
			not_ack_packet << (uint32_t)error_mesg.size() << nack_flag;
			
			connection->SendIndependentMesg(not_ack_packet);
		}
		
		void SignupUser(UserConnection_ptr connection, std::shared_ptr<Message> mesg)
		{
			UserID uid;
			Password pass;
			std::string user_name;
			std::cout<<"body->"<<mesg->body.data()<<"\n";
			try
			{
				mesg->GetData(&uid, &pass, &user_name);
			}
			catch(...)
			{
				SysRequestNotAck(connection, "sign-up failed for unknown reasons", SysRequest::SIGN_NACK);
				return;
			}
			
			std::cout <<"signUp-> uid-"<<uid<<" name-"<<user_name<<" pass-"<<pass<<"\n";
			
			if(authorizer.isAuthenticated(uid))
			{
				SysRequestNotAck(connection, "try different user_id", SysRequest::SIGN_NACK);
				return;
			}
			
			UserDataQueue_ptr new_queue(new UserDataQueue());
			UserInfo_ptr user_details(new UserInfo(uid, user_name));
			
			authorizer.AuthenticateUser(uid, connection, new_queue, user_details, pass);
			
			net::message<Header> log_ack_packet;
			log_ack_packet.header.id = Header::SYS_MESSAGE;
			user_details->Serialize(log_ack_packet);
			log_ack_packet << SysRequest::LOG_ACK;
			connection->SetOutQueue(new_queue);
			connection->SendPriorityMesg(log_ack_packet);
		}
		
	private:
		Authorizer& authorizer;
	};  
}

#endif