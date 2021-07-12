#include "net_common.h"
#include "server.h"

int main()
{
	net::Server s(60001);
	//"SERVER ACCEPTED YOUR CONNECTION\n"
	s.start();
	while (true)
	{
		s.process_msgIn(10);
	}
		
	return 0;
}