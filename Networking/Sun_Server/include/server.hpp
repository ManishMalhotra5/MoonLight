#ifndef SERVER_HPP
	#define SERVER_HPP

// from Sun-Net Project
#include "sun_net.hpp"

#include "AuthManager.hpp"
#include "SysManager.hpp"
#include "ChatManager.hpp"
#include "QueryManager.hpp"

namespace sun
{
	namespace net
	{
		class server : public server_interface<Header>
		{
		public:
			server(uint16_t port)
			:server_interface<Header>(port), authorizer(), sys_handler(authorizer), chat_handler(authorizer), query_handler(authorizer)
			{
			}
			
			void SendorStore(UserID uid, std::shared_ptr<Message> mesg)
			{
				auto d = authorizer.template get<UserConnection_ptr>(uid);
				
				if(!MessageClient(d, *mesg))
				{
					authorizer.template get<UserDataQueue_ptr>(uid)->push(mesg);
				}
			}
			
			void Send(UserID uid, std::shared_ptr<Message> mesg)
			{
				MessageClient(authorizer.template get<UserConnection_ptr>(uid), *mesg);
			}
			
		protected:	
			virtual bool OnClientConnect(std::shared_ptr<connection<Header>> client)
			{
		
				return true;
			}
			virtual void OnClientDisconnect(std::shared_ptr<connection<Header>> client)
			{
				//update map
				if(!client)
				{
					return;
				}
				authorizer.InvalidateConnection(client->GetID());
				
			}
	
			virtual void OnMessage(UserConnection_ptr connection,std::shared_ptr<Message> mesg)
			{
				
				asio::post(
					[this,connection,mesg]()
					{
						UserID send_store_uid, send_uid;
						switch(mesg->header.id)
						{
							case Header::SYS_MESSAGE:
								sys_handler.HandlePacket(connection, mesg);
								break;
						
							case Header::FILE_MESSAGE:
								// implementation pending
								// m_fileManager.Handler(connection, mesg);
								break;
						
							case Header::CHAT_MESSAGE:
								chat_handler.HandleTextPacket(connection, mesg, send_store_uid);
								break;
								
							case Header::ROOM_REQUEST:
								chat_handler.HandleRoomRequestPacket(connection,mesg, send_uid);
								break;
							
							case Header::QUERY :
								query_handler.Handle(connection, mesg, send_uid);
								break;
							default :
								std::cout << "unknown Header\n";
								break;
						}
						
						if(send_store_uid.size())
							SendorStore(send_store_uid, mesg);
							
						else if(send_uid.size())
							Send(send_uid, mesg);
						
					});
			}
		public:
			virtual void OnClientValidation(std::shared_ptr<connection<Header>> client)
			{
				//send CONNECTION_ACK packet on connection validation
				message<Header> ackPacket;
				ackPacket.header.id = Header::CONNECTION_ACK;
				client->SendIndependentMesg(ackPacket);
			}
			
		public:
			Authorizer authorizer;
			SysHandler sys_handler;
			ChatHandler chat_handler;
			QueryHandle query_handler;
		 //  FileManager m_fileManager ;
		};
	}	
}
#endif 