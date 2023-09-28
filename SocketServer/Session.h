#pragma once
#include "Message.h"

class Session
{
public:
	int id; // cl id 
	system_clock::time_point lastInteraction;
	queue<Message> messages;

	CCriticalSection cs;
	Session(int _id)
		:id(_id)
	{
	}

	void add(Message& m)
	{
		CSingleLock lock(&cs, TRUE);
		messages.push(m);
	}

	void send(CSocket& s)
	{
		CSingleLock lock(&cs, TRUE);
		if (messages.empty())
		{
			Message::send(s, id, MR_BROKER, MT_NODATA);
		}
		else
		{
			messages.front().send(s);
			messages.pop();
		}
	}

	void updateLastInteraction();

	bool stillActive();

	int inActivity();
};

