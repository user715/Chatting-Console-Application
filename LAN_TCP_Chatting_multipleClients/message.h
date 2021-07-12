#pragma once
#include "net_common.h"

namespace net
{
	class messageHeader
	{
	public:
		int sender_id=0;
		int message_size=0;
	};
	class message
	{
	public:
		messageHeader header;
		vector<char> msg;

		int size()
		{
			return msg.size() + sizeof(messageHeader);
		}

		friend ostream& operator << (ostream& os, message& m)
		{
			os << "sender [ " << m.header.sender_id << " ] : ";
			string s(m.msg.begin(), m.msg.end());
			os << s << endl;
			return os;
		}
	};
}