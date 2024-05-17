#ifndef NETCLIENT_HPP
#define NETCLIENT_HPP

#include "common.hpp"
#include "ThreadSafeQueue.hpp"
#include "message.hpp"
#include "connection.hpp"

namespace sun
{
	namespace net
	{
		template <typename T>
		class client_interface
		{
		public:
			client_interface()
			{
				
			}
			
			virtual ~client_interface()
			{
				Disconnect();
			}
			
			
			bool connect(const std::string host,const uint16_t port)
			{
				try{
					
					asio::ip::tcp::resolver resolver(m_context);
					asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host,std::to_string(port));
					
					m_connection = std::make_unique<connection<T>>(connection<T>::owner::client,
												m_context,asio::ip::tcp::socket(m_context),m_qMesgIn);
					m_connection->SetOutQueue(std::make_shared<ThreadSafeQueue<message<T>>>());
					m_connection->ConnectToServer(endpoints);
					
					m_contextThread = std::thread([this](){
						m_context.run();
					});
					
				}catch(std::exception& e){
					m_connection->SetOutQueue(nullptr);
					m_connection.reset();
					std::cout<<"[Client] connect failed :"<<e.what()<<"\n";
					return false;
				}
				
				return true;
			}
			
			void Disconnect()
			{
				if(IsConnected()){
					m_connection->Disconnect();
				}
				
				m_context.stop();
				
				if(m_contextThread.joinable())
					m_contextThread.join();
				m_connection.release();
			}
			
			bool IsConnected()
			{
				if(m_connection)
					return m_connection->IsConnected();
				
				return false;
			}
			
			void Send(const message<T>& mesg)
			{
				m_connection->Send(mesg);
			}
			
			ThreadSafeQueue<owned_mesg<T>>& IncomingQ(){
				return m_qMesgIn;
			}
		protected:
			
			asio::io_context m_context;
			std::thread m_contextThread;
			std::unique_ptr<connection<T>> m_connection;
		
		private:
			ThreadSafeQueue<owned_mesg<T>> m_qMesgIn;
		};
	}
}
	
#endif 