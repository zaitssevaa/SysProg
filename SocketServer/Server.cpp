#include "pch.h"
#include "Server.h"

int Server::maxID = MR_USER;
int Server::restServerId = MR_RESTSERVER;

void Server::ProcessClient(SOCKET hSock)
{
	CSocket s;
	s.Attach(hSock);
	Message m;
	//int messageType = m.receive(s);

	switch (m.receive(s))
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

	// хр сообщен
	case MT_INITSTORAGE:
	{
		auto session = make_shared<Session>(MR_STORAGE);
		sessions[session->id] = session;
		Message::send(s, session->id, MR_BROKER, MT_INITSTORAGE);
		session->updateLastInteraction();
		cout << "Storage connected" << endl;
		break;
	}

	case MT_REST_SERVER:
	{
		//auto session = make_shared<Session>(restServerId, m.data, std::chrono::high_resolution_clock::now());
		auto session = make_shared<Session>(restServerId);
		sessions[session->id] = session;
		restServerId = session->id;
		Message::send(s, session->id, MR_BROKER, MT_REST_SERVER);
		cout << "REST server entered" << endl;
		break;
	}

	case MT_GETLAST_PUBLIC:
	{
		if (m.header.from == MR_STORAGE)
		{
			auto iSessionTo = sessions.find(m.header.to);
			if (iSessionTo != sessions.end())
			{
				Message ms = Message(m.header.to, MR_BROKER, MT_GETLAST_PUBLIC, m.data);
				iSessionTo->second->add(ms);
			}
		}
		else
		{
			auto iSessionFrom = sessions.find(m.header.from);
			auto StorageSession = sessions.find(MR_STORAGE);
			if (StorageSession != sessions.end() && iSessionFrom != sessions.end())
			{
				iSessionFrom->second->updateLastInteraction();
				Message ms = Message(MR_STORAGE, m.header.from, MT_GETLAST_PUBLIC);
				StorageSession->second->add(ms);
			}
		}
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
	//
	case MT_GETUSERS:
	{
		string str = "";
		auto iSession = sessions.find(m.header.from);
		if (iSession != sessions.end())
		{
			for (auto& [id, session] : sessions)
			{
				// ≈сли сесси€ не €вл€етс€ сессией отправител€ (id != m.header.from) и не €вл€етс€ сессией "Storage" (id != MR_STORAGE),
				if (id != m.header.from && id != MR_STORAGE)
				{
					str.append("Client ");
					str.append(to_string(session->id));
					str.append(", last action ");
					str.append(to_string(session->inActivity()));
					str.append(" ms ago");
					str.append("\n");
				}
			}
			Message ms = Message::send(m.header.from, MT_GETUSERS, str);
			iSession->second->add(ms);
			iSession->second->updateLastInteraction();
		}
		break;
	}
	//
	case MT_GETLAST:
	{
		if (m.header.from == MR_STORAGE)
		{
			auto iSessionTo = sessions.find(m.header.to);
			if (iSessionTo != sessions.end())
			{
				Message ms = Message(m.header.to, MR_BROKER, MT_GETLAST, m.data);
				iSessionTo->second->add(ms);
			}
		}
		else
		{
			auto iSessionFrom = sessions.find(m.header.from);
			auto StorageSession = sessions.find(MR_STORAGE);
			if (StorageSession != sessions.end() && iSessionFrom != sessions.end())
			{
				iSessionFrom->second->updateLastInteraction();
				Message ms = Message(MR_STORAGE, m.header.from, MT_GETLAST);
				StorageSession->second->add(ms);
			}
		}
		break;
	}

	default:
	{
		//  
		auto iSessionFrom = sessions.find(m.header.from);
		auto StorageSession = sessions.find(MR_STORAGE);
		if (iSessionFrom != sessions.end() && m.header.from != MR_STORAGE)
		{
			auto iSessionTo = sessions.find(m.header.to);
			if (iSessionTo != sessions.end())
			{
				iSessionTo->second->add(m);
				if (StorageSession != sessions.end())
				{
					m.data = "{'" + to_string(m.header.from) + "':'" + m.data + "'}";
					Message ms = Message(MR_BROKER, m.header.to, MT_DATA, m.data);
					StorageSession->second->add(ms);
				}
			}
			else if (m.header.to == MR_ALL)
			// ≈сли сесси€ получател€ не была найдена и идентификатор получател€ равен MR_ALL, то это означает, что сообщение должно быть отправлено всем активным сесси€м, кроме сессии "Storage".
			{
				for (auto& [id, session] : sessions)
				{
					if (id != m.header.from && id != MR_STORAGE)
					{
						session->add(m);
						if (StorageSession != sessions.end())
						{
							string mes = "{'" + to_string(m.header.from) + "':'" + m.data + "'}";
							Message ms = Message(MR_BROKER, id, MT_DATA, mes);
							StorageSession->second->add(ms);
						}
					}
				}
			}
			iSessionFrom->second->updateLastInteraction();
		}
		break;
	}
	}
}
//
void Server::IsActive()
{
	while (true)
	{
		for (auto& [id, session] : sessions)
		{
			if (!(session->stillActive()))
			{
				cout << "Time out. Client " << id << "disconnected" << endl;
				sessions.erase(id);
				break;
			}
		}
		Sleep(1000);
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
