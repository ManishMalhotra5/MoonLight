#ifndef QUERYMANAGER_HPP
#define QUERYMANAGER_HPP

#include "sun_net.hpp"
#include "AuthManager.hpp"

namespace sun
{
	class QueryHandle
	{
	public:
		QueryHandle(Authorizer& auth)
		: authorizer(auth)
		{
			
		}
		
		void Handle(UserConnection_ptr connection, std::shared_ptr<Message> mesg, UserID& receiver_id)
		{
			if(!authorizer.isValidConnection(connection->GetID()))
			{
				std::cout<<"Invalid Connection - packet discarded\n";
				return;
			}
			
			
			Query flag;
			*mesg >> flag;

			switch(flag)
			{
				case Query::SEARCH_QUERY :
					HandleSearchQuery(connection, mesg, receiver_id);
					break;
				
				case Query::DOWNLOAD_QUERY :
					
					break;
					
				default :
					break;
			};
		}
		
		void HandleSearchQuery(UserConnection_ptr connection, std::shared_ptr<Message> mesg, UserID& receiver_id)
		{
			std::vector<std::string> most_similar;
			most_similar.reserve(20);
			
			std::string search_hint;
			uint32_t start_after;
			
			*mesg >> start_after;
			try
			{
				mesg->GetData(&search_hint);
			}
			catch(...)
			{
				std::cout <<"["<< connection->GetID() << "] Query Message extraction Failed - packet discarded\n";
				return;
			}
			authorizer.SearchUsers_hint(search_hint, most_similar, 20, start_after);
			
			mesg->header.size = 0;
			mesg->body.resize(0);
			
			for(auto& d : most_similar)
			{
				mesg->SerializeArray(d.data() , d.size());
				*mesg << (uint32_t)d.size();
			}
			
			*mesg << (uint32_t)most_similar.size() << Query::SEARCH_QUERY;
			
			receiver_id = authorizer.get<UserID>(connection->GetID());
		}
		
	private:
		Authorizer& authorizer;
	};
}

#endif