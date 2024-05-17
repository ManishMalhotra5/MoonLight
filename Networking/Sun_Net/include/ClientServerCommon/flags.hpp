#ifndef FLAGS_HPP
#define FLAGS_HPP

#include "common.hpp"

namespace sun
{
	enum Header : uint32_t
	{
	
		//server send it to client after successful connection implementation
		CONNECTION_ACK,
		
		//Query used to search for friends user_id in the server
		//Query packet have further classifications
		QUERY,
		//by string matching server will send ACK with matched user_id (message body may contain more than 1 user_ids) which further can be used to Request Rooms to interact with other people
		
		//ROOM formed by two friends for chatting
		//room_request have further classifications
		ROOM_REQUEST,
		
		// category of packet
		SYS_MESSAGE,
		FILE_MESSAGE,
		CHAT_MESSAGE
	};
	
	enum ChatMesg : uint32_t
	{
		TEXT,
		FILE_MESG
	};
	
	// • Systen headers -- set, reset or update user data
	enum SysRequest : uint32_t
	{
		//login message contains user_id and password
		LOGIN,
		// Login Ack send by server with body contains user_info
		LOG_ACK,
		//ERROR while logging in will return from server with error meesage
		LOG_NACK,
		//signing in packet contains user_id, user_name,password
		SIGNUP,
		//Sign Not Ack will return with an error message which points out what went wrong while creating mew account
		SIGN_NACK
	};
	
	enum FileRequest : uint32_t
	{
		
		FILE_DATA_PACKET,
		FILE_SEND,
		FILE_SEND_ACK,
		FILE_DOWNLOAD,
		FILE_DOWNLOAD_ACK,
		FILE_CANCEL
	};
	
	enum RoomRequest : uint32_t
	{
		FRIEND_REQUEST,
		FRIEND_REQ_ACCEPT,
		FRIEND_REQ_REJECT
	};
	enum Query : uint32_t
	{
		SEARCH_QUERY,
		DOWNLOAD_QUERY
	};
	
}

#endif
/*
			Packet's Fromat
			
universal fromat 
	|Header|sizeOfBody|Body|
	-Header represents type of packets it is.
	-Size represents the number bytes in the body.
		if sizeOfBody is zero,then its a body-less packets
	-Body contains the payload. think body as a Stack data structure, you can Serialize or DeSerialize (Push or pop) data from back
	
• |LOGIN & SYS_MESSAGE|Bodysize|password|size|user_id|size|

• |LOGIN_ACK & SYS_MESSAGE|user_info|size|

• |SIGNUP & SYS_MESSAGE|password|size|user_name|size|user_id|size|

• |SIGN_NACK & SYS_MESSAG|size|error_message|

• |QUERY & SYS_MESSAG|...|query_enum_type|

• |QUERY_ACK & SYS_MESSAG|...|query_ack_enum_type|

• |ROOM_REQUEST & SYS_MESSAG|...|request_enum_type|

• |SYS_MESSAGE & SYS_MESSAG|0|


....file system pending....
• |FILE_MESSAGE||||



• 1) |TEXT_MESSAGE|...|receiver_id|size|
  2) |TEXT_MESSAGE|...|sender_id|size|
  		server process it (1) and change the format (2) and send it to receiver
  		so that the receiver can identify who sent the message.
*/



/*
		ROOM_REQUEST Packet format
		
• |ROOM_REQUEST|Size|user_id|size|FRIEND_REQUEST|

• |ROOM_REQUEST|Size|user_id|size|FRIEND_REQ_ACCEPT|

• |ROOM_REQUEST|size|user_id|size|FRIEND_REQ_REJECT|

*/