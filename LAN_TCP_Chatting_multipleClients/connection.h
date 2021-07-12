#pragma once
#include "net_common.h"
#include "message.h"
#include <unordered_map>

namespace net
{
	class connection
	{
	protected:
		asio::ip::tcp::socket soc;
		asio::io_context& conn_context;
		deque<net::message> q_msg_out;
		deque<net::message>& q_msg_in;
		net::message temp;
		int id = 0;
		bool name_recieved = false;
		unordered_map<int, string>& name;
		
	public:
		connection(asio::io_context& context, asio::ip::tcp::socket socket, deque<net::message>& q_msg,unordered_map<int, string>& naming)
			:conn_context(context), soc(std::move(socket)), q_msg_in(q_msg),name(naming)
		{

		}

		~connection()
		{
			Disconnect();
		}

		int GetId()
		{
			return id;
		}

		void send(net::message msg)
		{
			q_msg_out.push_back(msg);
			if (!q_msg_out.empty())writeHeader();
		}

		bool IsConnected()
		{
			return soc.is_open();
		}

		void Disconnect()
		{
			if (IsConnected())
			{
				asio::post(conn_context, [this]() { soc.close(); });
			}
		}

		void ConnectToClient(int c_id = 0)
		{
			id = c_id;
			net::message temp_id;
			temp_id.header.sender_id = -1;
			temp_id.header.message_size = sizeof(int);
			string s = to_string(id);
			vector<char> v(s.begin(), s.end());
			temp_id.msg=v;
			send(temp_id);
			readHeader();
		}

		void readHeader()
		{
			soc.async_read_some(asio::buffer(&temp.header, sizeof(net::messageHeader)),
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
			soc.async_read_some(asio::buffer(temp.msg.data(), temp.header.message_size),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (!name_recieved)
						{
							name_recieved = true;
							string s(temp.msg.begin(), temp.msg.end());
							name[id] = s;
						}
						else q_msg_in.push_back(temp);
					}
					readHeader();
				}
			);
		}

		void writeHeader()
		{
			asio::async_write(soc, asio::buffer(&q_msg_out.front().header, sizeof(net::messageHeader)),
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
			asio::async_write(soc,asio::buffer(q_msg_out.front().msg.data(), q_msg_out.front().header.message_size),
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