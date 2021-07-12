#include "net_common.h"
#include "client.h"
#include "message.h"

net::message temp;



int main()
{
	string server = "192.168.31.12";
	//cout << "ENTER PRIVATE IP ADRESS OF SERVER IN YOUR LAN\n";
	int port = 60001;
	net::Client c(server, port);
	//std::thread t(input,c);
	c.start();
	temp.header.sender_id = c.GetId();
	cout << "YOUR ID IS : " << c.GetId()<<"\nENTER YOR NAME : ";
	while (true)
	{
		string s;
		getline(cin, s);
		vector<char> v(s.begin(), s.end());
		temp.header.message_size = v.size();
		temp.msg = v;
		c.send(temp);
	}
	//t.join();
	return 0;
}