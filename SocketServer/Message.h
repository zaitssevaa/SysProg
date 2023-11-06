#pragma once

enum MessageTypes
{
	MT_INIT,
	MT_EXIT,
	MT_GETDATA,
	MT_DATA,
	MT_NODATA,
	MT_CONFIRM,
	MT_GETUSERS,
	MT_GETLAST,
	MT_INITSTORAGE
};

enum MessageRecipients
{
	MR_BROKER = 10,
	MR_STORAGE = 20,
	MR_ALL = 50,
	MR_USER = 100   // users' ids from 100 (101,102,...)
};

struct MessageHeader
{
	int to;
	int from;
	int type;
	int size;
};

class Message
{
public:
	MessageHeader header = { 0 };
	string data;
	static int clientID;

	Message() {}
	Message(int to, int from, int type = MT_DATA, const string& data = "")
	{
		this->data = data;
		this->header = { to, from, type, int(data.length()) };
	}

	void send(CSocket& s)
	{
		s.Send(&header, sizeof(MessageHeader));
		if (header.size)
		{
			s.Send(data.c_str(), (int)header.size);
		}
	}

	int receive(CSocket& s)
	{
		if (!s.Receive(&header, sizeof(MessageHeader)))
		{
			return MT_NODATA;
		}
		if (header.size)
		{
			vector <char> v(header.size);
			s.Receive(&v[0], (int)header.size);
			this->data = string(&v[0], header.size);
		}
		return header.type;
	}

	static void send(CSocket& s, int to, int from, int type = MT_DATA, const string& data = "");
	static Message send(int to, int type = MT_DATA, const string& data = "");
};

