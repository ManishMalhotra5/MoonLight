#include "client.hpp"


int main(int argc, char *argv[])
{
	

	std::cout <<" simpleClient\n";
	sun::client c;
	int attempts = 6;
	if(argc>1)
		c.connect(argv[1],60000);

	while(attempts && c.IsConnected())
	{
		c.CurrentRoomNextMessage();
	}
	
	return 0;
}