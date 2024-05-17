#pragma once
#ifndef NETATTRIBUTES_HPP
#define NETATTRIBUTES_HPP

#include "sun_net.hpp"

namespace sun
{
		typedef std::string UserID;

		struct Password : std::string
		{

		};
		
		typedef net::user_info<Header> UserInfo;
		
		typedef std::shared_ptr<UserInfo> UserInfo_ptr ;

		typedef std::shared_ptr<net::connection<Header>> UserConnection_ptr ;
		
		typedef net::message<Header> Message;
		
		typedef ThreadSafeQueue<Message> UserDataQueue ;
		
		typedef std::shared_ptr<UserDataQueue> UserDataQueue_ptr ;
	
		
}


#endif