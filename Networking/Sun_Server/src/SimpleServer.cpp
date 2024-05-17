#include "server.hpp"

int main(int argc, char *argv[])
{
	
	sun::net::server sv(60000);
	
	
	sv.Start();
	 
	while(1)
	{
		sv.Update();
	}
	
	return 0;
}