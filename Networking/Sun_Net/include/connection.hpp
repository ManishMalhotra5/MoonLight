#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "common.hpp"
#include "ThreadSafeQueue.hpp"
#include "message.hpp"

namespace sun
{
	namespace net
	{
		template <typename T>
		class server_interface;
		
		template <typename T>
		class connection :public std::enable_shared_from_this<connection<T>>
		{
		public:
			enum owner
			{
				client,
				server
			};
			connection(owner parent,
						asio::io_context& context,
						asio::ip::tcp::socket&& socket,
						ThreadSafeQueue<owned_mesg<T>>& incoming):
							m_socket(std::move(socket)),
							m_context(context), 
							m_qMesgIn(incoming)
			{
				m_isWriteHeadBusy.store(false, std::memory_order_relaxed);
				
				m_ownerType = parent;
				
				if(m_ownerType == owner::server)
				{
					m_handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());
					m_handshakeCheck = scramble(m_handshakeOut);
				}
				
			}
			virtual ~connection()
			{
				
			}
			
			void ConnectToClient(server_interface<T>* server,uint32_t uid = 0)
			{
				if(m_ownerType == owner::server && m_socket.is_open())
				{
					m_id = uid;
					WriteValidation();
					ReadValidation(server);
				}
			}
		
			void ConnectToServer(asio::ip::tcp::resolver::results_type& endpoints)
			{
				asio::async_connect(m_socket,endpoints,
				[this](asio::error_code ec,asio::ip::tcp::endpoint endpoints )
				{
					if(!ec)
					{
						ReadValidation();
					}
					else
					{
						std::cout<<"[Client] Connection To Server Failed : "<<ec.message()<<"\n";
					}
				});
			}
			
			void Disconnect()
			{
				if(IsConnected())
					asio::post(
					[this]()
					{
						m_socket.close();
					});
			}
			bool IsConnected() const
			{
				return m_socket.is_open();
			}
			
			uint32_t GetID()
			{
				return m_id;
			}
			
			void Send(const message<T>& mesg)
			{
				//author may upgrade this routine to throw an exception
				if (!m_qMesgOut)
				{
					return;
				}
				asio::post(
				[this, mesg]()
				{
					m_qMesgOut->push(mesg);
					TriggerOutQueue();
				});
			}
			
			void Send(std::shared_ptr<message<T>> const mesg)
			{
				if (!m_qMesgOut)
				{
					return;
				}
				asio::post(
				[this, mesg]()
				{
					m_qMesgOut->push(mesg);
					TriggerOutQueue();
				});
			}
			
			void ReadHead()
			{
				asio::async_read(m_socket,asio::buffer(&m_tempMesgIn.header,sizeof(mesg_header<T>)),
				[this](asio::error_code ec,uint32_t Readlenght)
				{
					if(!ec)
					{
						if(m_tempMesgIn.header.size > 0)
						{
							m_tempMesgIn.body.resize(m_tempMesgIn.header.size);
							ReadBody();
						}else
						{
							PushIncomingQMesg();
						}
					}
					else
					{
						std::cout<<"["<<GetID()<<"] Async ReadHead Failed :"<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void ReadBody()
			{
				asio::async_read(m_socket,asio::buffer(m_tempMesgIn.body.data(),m_tempMesgIn.body.size()),
				[this](asio::error_code ec,uint32_t Readlenght)
				{
					if(!ec)
					{
						PushIncomingQMesg();
					}
					else
					{
						std::cout<<"["<<GetID()<<"] Async ReadBody Failed :"<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void SendPriorityMesg(message<T>& mesg)
			{
				if (!m_qMesgOut)
				{
					return;
				}
				asio::post(
					[this, mesg]()
					{
						m_qMesgOut->push_front(mesg);
						TriggerOutQueue();
					});
			}
			
			void SendPriorityMesg(std::shared_ptr<message<T>> const mesg)
			{
				if (!m_qMesgOut)
				{
					return;
				}
				asio::post(
					[this, mesg]()
					{
						m_qMesgOut->push_front(*mesg);
						TriggerOutQueue();
					});
			}
			
			
			void TriggerOutQueue() 
			{
				bool exp_val = false, desired_val = true;
				if(m_isWriteHeadBusy.compare_exchange_strong(exp_val, desired_val))
				{
					WriteHead();
				}
			}

			void SendIndependentMesg(message<T>& mesg)
			{
				asio::async_write(m_socket,asio::buffer(&mesg.header,sizeof(mesg_header<T>)),
				[this,mesg](asio::error_code ec,uint32_t writelenght)
				{
					if(!ec)
					{
						if(mesg.header.size > 0)
						{
							asio::error_code error;
							asio::write(m_socket,asio::buffer(mesg.body.data(),mesg.body.size()),error);
							if(error)
							{
							std::cout<<"["<<GetID()<<"] Async SendIndependentMesg Failed :"<<error.message()<<"\n";
							m_socket.close();
							}
						}
						
					}
					else
					{
						std::cout<<"["<<GetID()<<"] Async SendIndependentMesg Failed :"<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			
			void WriteHead()
			{
				if(!m_qMesgOut)	return;
				
				auto tempMesgOut = m_qMesgOut->wait_and_pop();
				asio::async_write(m_socket,asio::buffer(&tempMesgOut->header,sizeof(mesg_header<T>)),
				[this,tempMesgOut](asio::error_code ec,uint32_t writelenght)
				{
					if(!ec)
					{
						if(tempMesgOut->header.size > 0)
						{
							WriteBody(tempMesgOut);
						}
						else
						{
							if(!m_qMesgOut->empty())	WriteHead();
							else	m_isWriteHeadBusy.store(false, std::memory_order_release);
						}
						
					}
					else
					{
						m_isWriteHeadBusy.store(false, std::memory_order_release);
						std::cout<<"["<<GetID()<<"] Async WriteHead Failed :"<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void WriteBody(std::shared_ptr<message<T>> MesgOut)
			{
				
				asio::async_write(m_socket,asio::buffer(MesgOut->body.data(),MesgOut->body.size()),
				[this](asio::error_code ec,uint32_t writelenght)
				{
					if(!ec)
					{
						if(!m_qMesgOut->empty())	WriteHead();
						else	m_isWriteHeadBusy.store(false, std::memory_order_release);
					}
					else
					{
						m_isWriteHeadBusy.store(false, std::memory_order_release);
						std::cout<<"["<<GetID()<<"] Async WriteBody Failed :"<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void PushIncomingQMesg()
			{
				if(m_ownerType == owner::server)
				{
					m_qMesgIn.push(owned_mesg<T>(this->shared_from_this(),m_tempMesgIn));
				}
				else
				{
					m_qMesgIn.push(owned_mesg<T>(nullptr,m_tempMesgIn));
				}
				ReadHead();
			}
			
			void ReadValidation(server_interface<T>*server = nullptr)
			{
				asio::async_read(m_socket,asio::buffer(&m_handshakeIn,sizeof(uint64_t)),
				[this,server](asio::error_code ec, uint64_t lenght){
					if(!ec)
					{
						if(m_ownerType == owner::server)
						{
							if(m_handshakeIn == m_handshakeCheck)
							{
								std::cout<<"["<<GetID()<<"] Cleint Valdated\n";
								server->OnClientValidation(this->shared_from_this());
								ReadHead();
							}
							else
							{
								std::cout<<"["<<GetID()<<"] Handshake check Failed\n";
								m_socket.close();
							}
						}
						else
						{
							//client
							m_handshakeOut = scramble(m_handshakeIn);
							WriteValidation();
						}
					}
					else
					{
						std::cout<<"["<<GetID()<<"] ReadValidation Failed : "<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			void WriteValidation()
			{
				asio::async_write(m_socket,asio::buffer(&m_handshakeOut,sizeof(uint64_t)),
				[this](asio::error_code ec, uint64_t lenght)
				{
					if(!ec)
					{
						if(m_ownerType == owner::client)
						{
							ReadHead();
						}
					}
					else
					{
						std::cout<<"["<<GetID()<<"] WriteValidation Failed : "<<ec.message()<<"\n";
						m_socket.close();
					}
				});
			}
			
			uint64_t scramble(uint64_t input)
			{
				uint64_t out = ! (input & 123468294992);
				out = ((out | 1<<12) & !(out << 12));
				return out;
			}
			void SetOutQueue(std::shared_ptr<ThreadSafeQueue<message<T>>> qMesgOut)
			{
				m_qMesgOut = qMesgOut;
			}
		protected:
			asio::ip::tcp::socket m_socket;
			asio::io_context& m_context;
			
			//client - intitilaized in constructor
			//server - new queue created in case signup,  old queue from map is used in case login
			std::shared_ptr<ThreadSafeQueue<message<T>>> m_qMesgOut;
			ThreadSafeQueue<owned_mesg<T>>& m_qMesgIn;
			
			message<T> m_tempMesgIn;
			
			std::atomic<bool> m_isWriteHeadBusy;
			
			owner m_ownerType;
			uint32_t m_id = 0;
			
			uint64_t m_handshakeIn = 0;
			uint64_t m_handshakeOut = 0;
			uint64_t m_handshakeCheck = 0;
			
		};
	}
}

#endif