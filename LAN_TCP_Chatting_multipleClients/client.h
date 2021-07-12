#include "net_common.h"
#include "message.h"

namespace net
{
	class Client
	{
	protected:
		asio::io_context c_context;
		asio::ip::tcp::socket c_socket;
		net::message temp,temp_write;
		deque<net::message> q_msg_out;
		std::thread c_thr;
		int id;
		bool id_recieved = false;
	public:

		void send(net::message s)
		{
			q_msg_out.push_back(s);
			if (!q_msg_out.empty())writeHeader();
		}

		int GetId()
		{
			while (true)
			{

				if (id_recieved == true)
				{
					return id;
				}
				else
				{
					cout << "";
				}
			}
		}

		Client(string server, int port) :c_socket(c_context),id(0)
		{
			temp_write.header.message_size = 0;
			temp_write.msg.clear();
			asio::ip::tcp::resolver c_resolver(c_context);
			asio::ip::tcp::resolver::results_type endPoints = c_resolver.resolve(server, to_string(port));
			asio::async_connect(c_socket, endPoints,
				[this](std::error_code ec, asio::ip::tcp::endpoint ep)
				{
					if (!ec)
					{
						cout << "NEW CONNECTION\n" << ep.address() << ":" << ep.port() << endl;;
						readHeader();
					}
					else cout << ec.message();
				}
			);

		}

		void start()
		{
			c_thr = std::thread([this]() {c_context.run(); });
		}

		void readHeader()
		{
			c_socket.async_read_some(asio::buffer(&temp.header, sizeof(net::messageHeader)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (temp.header.message_size > 0)
						{
							temp.msg.resize(temp.header.message_size);
							readBody();
						}
					}
					else
					{
						cout << ec.message() << endl;
					}
				}
			);
		}

		void readBody()
		{
			c_socket.async_read_some(asio::buffer(temp.msg.data(), temp.header.message_size),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						string s(temp.msg.begin(), temp.msg.end());
						if (temp.header.sender_id == -1)
						{
							id = stoi(s);
							id_recieved = true;
						}
						else cout << s << endl;
					}
					readHeader();
				}
			);
		}

		void writeHeader()
		{
			asio::async_write(c_socket, asio::buffer(&q_msg_out.front().header, sizeof(net::messageHeader)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (q_msg_out.front().header.message_size > 0) writeBody();
						else
						{
							q_msg_out.pop_front();
							if (!q_msg_out.empty())writeHeader();
						}
					}
					else
					{
						cout << ec.message() << endl;
					}
				}
			);
		}

		void writeBody()
		{
			asio::async_write(c_socket, asio::buffer(q_msg_out.front().msg.data(), q_msg_out.front().header.message_size),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						q_msg_out.pop_front();
						if (!q_msg_out.empty())writeHeader();
					}
					else
					{
						cout << ec.message() << endl;
					}
				}
			);
		}

	};
}