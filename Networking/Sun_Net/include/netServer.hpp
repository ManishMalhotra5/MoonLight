#ifndef NETSERVER_HPP
#define NETSERVER_HPP

#include "common.hpp"
#include "ThreadSafeQueue.hpp"
#include "connection.hpp"

namespace sun
{
	namespace net
	{
		template <typename T>
		class server_interface
		{
	
		public:
			server_interface(uint16_t port):
			m_acceptor(m_context,asio::ip::tcp::endpoint(asio::ip::tcp::v4(),port))
			{
				
			}
			virtual ~server_interface()
			{
				Stop();
			}
			bool Start()
			{
				try
				{
					WaitForClientConnection();
					
					m_contextThread = std::thread(
					[this](){
						m_context.run();
					});
					
				}catch(std::exception& e)
				{
					std::cout<<"[Server] Exception : " << e.what()<<"\n";
					return false;
				}
				std::cout<<"[Server] Sarted\n";
				return true;
			}
			void Stop()
			{
				m_context.stop();
				
				if(m_contextThread.joinable())
					m_contextThread.join();
				std::cout<<"[Server] Stopped\n";
			}
			
			void WaitForClientConnection()
			{
				m_acceptor.async_accept(
				[this](asio::error_code ec,asio::ip::tcp::socket socket){
					
					if(!ec)
					{
						std::cout<<"[Server] New Connection " << socket.remote_endpoint()<<"\n";
	
						std::shared_ptr<connection<T>> newcon = std::make_shared<connection<T>>(connection<T>::owner::server, m_context,std::move(socket),m_qMesgIn);
						
						if(OnClientConnect(newcon))
						{
							m_deqNConnections.push_back(std::move(newcon));
							m_deqNConnections.back()->ConnectToClient(this,m_IDcounter++);
							std::cout<<"[Server] Connection Approved ID "<<m_deqNConnections.back()->GetID()<<"\n";
						}else{
							std::cout<<"[Server] : --- New Connetion Denied\n";
						}	
							
					}else{
						std::cout<<"[Server] Accepting New Connection Error: "<<ec.message()<<"\n";
					}
					
					WaitForClientConnection();
				});
			}
			bool MessageClient(std::shared_ptr<connection<T> > client,message<T> mesg)
			{
				if(client && client->IsConnected())
				{
					client->Send(mesg);
					return true;
				}else
				{
					OnClientDisconnect(client);
					client.reset();
					m_deqNConnections.erase(std::remove(m_deqNConnections.begin(),m_deqNConnections.end(),client),m_deqNConnections.end());
				}
				return false;
			}
			
			void MessageAllClient(message<T> mesg,std::shared_ptr<connection<T> > exceptThisClient = nullptr)
			{
				bool invalidClientExists = false;
				
				for(auto& client : m_deqNConnections)
				{
					if(client && client->IsConnected())
					{
						if(client != exceptThisClient)
							client->Send(mesg);
					}else{
						OnClientDisconnect(client);
						client->reset();
						invalidClientExists = true;
					}
				}
				if(invalidClientExists)
				{
					m_deqNConnections.erase(std::remove(m_deqNConnections.begin(),m_deqNConnections.end(),nullptr),m_deqNConnections.end());
				}
			}
			
			void Update(uint32_t max = -1)
			{
				int processedMesg = 0;
				while(processedMesg < max)
				{
					std::shared_ptr<owned_mesg<T>> mesg = m_qMesgIn.wait_and_pop();
					if(!mesg) break;
					OnMessage(mesg->remote,std::make_shared<message<T>>(mesg->mesg));
					processedMesg++;
				}
			}
			
		protected:
			virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
			{
				return false;
			}
			virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
			{
				
			}
			virtual void OnMessage(std::shared_ptr<connection<T>> client,std::shared_ptr<message<T>> mesg)
			{
				
			}
		public:
			virtual void OnClientValidation(std::shared_ptr<connection<T>> client)
			{
				
			}
		private:
			asio::io_context m_context;
			std::thread m_contextThread;
			asio::ip::tcp::acceptor m_acceptor;
			
			uint32_t m_IDcounter = 10001;
			ThreadSafeQueue<owned_mesg<T>> m_qMesgIn;
			
			std::deque<std::shared_ptr<connection<T>> > m_deqNConnections;
		};
	}
}

#endif