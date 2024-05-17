#include "sun_net.hpp"

struct db
{
	std::shared_ptr<int> ptr;
};

class password : std::string
{
	
};

struct UserConnection : std::shared_ptr<sun::net::connection<sun::Header>>
{
	
};

typedef sun::Database<std::string,std::shared_ptr<int>,int> database;

int main(){
	database b;
	
	std::shared_ptr<int> s(new int(30));
	std::shared_ptr<int> s1(new int(60));
	
	b.set_new_value(std::string("a"),s, 6);
	
	database::db_iterator it = b.find(std::string("a"));
	
	if(it != b.end())
	{
		*(it->second.get<std::shared_ptr<int>>()) = 10000;
	}
	
	if(b.contains("a"))
		std::cout<<*(b.get<std::shared_ptr<int>>(std::string("a")));
	
	return 0;
}