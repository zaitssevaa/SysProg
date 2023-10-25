#include "pch.h"
#include "Server.h"

int Server::maxID = MR_USER;

void Server::ProcessClient(SOCKET hSock)
{
	CSocket s;
	s.Attach(hSock);
	Message m;
	int messageType = m.receive(s);

	switch (messageType)
	{
	case MT_INIT:
	{
		auto session = make_shared<Session>(++maxID);
		sessions[session->id] = session;
		Message::send(s, session->id, MR_BROKER, MT_INIT);
		session->updateLastInteraction();
		cout << "Client " << session->id << "connected" << endl;
		break;
	}
	case MT_EXIT:
	{
		sessions.erase(m.header.from);
		Message::send(s, m.header.from, MR_BROKER, MT_CONFIRM);
		cout << "Client " << m.header.from << "disconnected" << endl;
		break;
	}
	case MT_GETDATA:
	{
		auto iSession = sessions.find(m.header.from);
		if (iSession != sessions.end())
		{
			iSession->second->send(s);
			iSession->second->updateLastInteraction();
		}
		else {
			cout << "hi!";
		}

		break;
	}
	case MT_GETUSERS:
	{
		string str = "";
		auto iSession = sessions.find(m.header.from);
		if (iSession != sessions.end())
		{
			for (auto& [id, session] : sessions)
			{
				if (id != m.header.from)
				{
					str.append("Client ");
					str.append(to_string(session->id));
					str.append(", last action ");
					str.append(to_string(session->inActivity()));
					str.append(" ms ago");
					str.append("\n");
				}
			}

			Message ms = Message(m.header.from, MR_BROKER, MT_GETUSERS, str);
			iSession->second->add(ms);
			iSession->second->updateLastInteraction();
		}
		break;
	}
	default:
	{
		auto iSessionFrom = sessions.find(m.header.from);
		if (iSessionFrom != sessions.end())
		{

			auto iSessionTo = sessions.find(m.header.to);
			if (iSessionTo != sessions.end())
			{
				iSessionTo->second->add(m);
			}
			else if (m.header.to == MR_ALL)
			{
				for (auto& [id, session] : sessions)
				{
					if (id != m.header.from)
						session->add(m);
				}
			}
			iSessionFrom->second->updateLastInteraction();
		}
		break;
	}
	}
}

void Server::IsActive()
{
	while (true)
	{
		std::vector<int> allIds(sessions.size());
		for (auto& [id, session] : sessions)
		{
			allIds.push_back(id);
		}

		for (int id : allIds)
		{
			auto sessionIt = sessions.find(id);

			if (sessionIt != sessions.end() && !(sessionIt->second->stillActive()))
			{
				cout << "Time out. Client " << id <<" disconnected" << endl;
				sessions.erase(sessionIt);
			}
		}
		Sleep(100);
	}
}

Server::Server()
{

	thread clientConnection(&Server::IsActive, this);
	clientConnection.detach();

	AfxSocketInit();
	CSocket server;
	server.Create(12345);
	while (true)
	{
		if (!server.Listen())
			break;
		CSocket s;
		server.Accept(s);
		thread t(&Server::ProcessClient, this, s.Detach());
		t.detach();
	}


}

int Server::getMaxID()
{
	return maxID;
}
