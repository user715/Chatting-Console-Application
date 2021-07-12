#include "net_common.h"
#include "connection.h"
#include <unordered_set>
#include <message.h>
#include <unordered_map>

namespace net
{
	class Server
	{
	protected:
		asio::io_context s_context;
		asio::ip::tcp::acceptor s_acceptor;
		unordered_set<std::unique_ptr<net::connection>> clients;
		deque<net::message> q_msg_in;
		std::thread s_thr;
		net::message temp_msg;
		int id = 10;
		unordered_map<int,string> naming;

	public:
		Server(int port) : s_acceptor(s_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{
			
		}

		void start()
		{
			WaitForNewClient();
			std::cout << "SERVER STARTED\n";
			cout << s_acceptor.local_endpoint().address() << endl;
			s_thr = std::thread([this]() {s_context.run(); });
		}

		void WaitForNewClient()
		{
			s_acceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket soc)
				{
					if (!ec)
					{
						cout << soc.remote_endpoint().address()<<" : "<< soc.remote_endpoint().port() << endl;
						std::unique_ptr<net::connection> new_conn = 
							std::make_unique<connection>(s_context, std::move(soc), q_msg_in,naming);
						new_conn->ConnectToClient(id++);
						clients.insert(std::move(new_conn));
					}
					WaitForNewClient();
				}
			);
		}

		void MessageClient(std::unique_ptr<net::connection> client, net::message msg)
		{
			if (client->IsConnected())
			{
				client->send(msg);
			}
			else
			{
				clients.erase(client);
			}
		}

		void MessageAll(net::message msg, int ignoreClientId = NULL )
		{
			bool InvalidClientExists = false;
			for (auto& client : clients)
			{
				if (client && client->IsConnected())
				{
					if (client->GetId() != ignoreClientId)client->send(msg);
				}
				else
				{
					InvalidClientExists = true;
				}
			}
			if (InvalidClientExists)
			{

			}

		}

		void process_msgIn(int cnt=1)
		{
			int count = 0;
			while (count < cnt && !q_msg_in.empty())
			{
				temp_msg = q_msg_in.front();
				string new_msg = naming[temp_msg.header.sender_id]+ " : " + string(temp_msg.msg.begin(), temp_msg.msg.end());
				temp_msg.header.message_size=(new_msg.size());
				temp_msg.msg.resize(new_msg.size());
				temp_msg.msg = vector<char>(new_msg.begin(), new_msg.end());
				q_msg_in.pop_front();
				MessageAll(temp_msg,temp_msg.header.sender_id);
			}
		}
		
	};
}