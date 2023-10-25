#include "pch.h"
#include "Client.h"
#include "../SocketServer/Message.h"
#include "../SocketServer/Message.cpp"

void Client::ProcessMessages()
{
	while (true)
	{
		Message m = Message::send(MR_BROKER, MT_GETDATA);
		switch (m.header.type)
		{
		case MT_DATA:
			cout << endl;
			cout << "Client " << m.header.from << ": " << m.data << endl;
			cout << "Choose action: ";
			cout << endl;
			break;

		case MT_GETUSERS:
			cout << endl;
			cout << "Clients:\n" << m.data << endl;
			cout << "Choose action: ";
			cout << endl;
			break;

		default:
			Sleep(1000);
			break;
		}
	}
}

void Client::Menu()
{
	cout << "----Actions----\n" <<
		"1. Send msg to user\n" <<
		"2. Send msg to all users\n" <<
		"3. Get user list\n" <<
		"4. Disconnect\n";
}

Client::Client()
{
	AfxSocketInit();
	thread t(&Client::ProcessMessages, this);
	t.detach();

	Message m = Message::send(MR_BROKER, MT_INIT);
	this->id = m.clientID;
	cout << "Welcome, Client " << this->id << endl; // запрос регистрации клиента(получение уникального идентификатора от сервера)

	while (true)
	{
		Menu();
		cout << "Choose action: ";
		int i;

		while ((cin >> i).fail())
		{
			cin.clear();
			cin.ignore(10000, '\n');
			cout << "Enter i\n";
		}

		cout << endl;
		switch (i)
		{
		case(1):
		{
			cout << "To: Client ";
			int id;

			while ((cin >> id).fail())
			{
				cin.clear();
				cin.ignore(10000, '\n');
				cout << "Enter id\n";
			}

			cout << "Message: ";
			string strmsg;
			cin.ignore();
			getline(cin, strmsg);
			Message::send(id, MT_DATA, strmsg);
			break;
		}
		case(2):
		{
			cout << "To all Clients" << endl;
			cout << "Message: ";
			string msg;
			cin.ignore();
			getline(cin, msg);
			Message::send(MR_ALL, MT_DATA, msg);
			break;
		}
		case(3):
		{
			Message::send(MR_BROKER, MT_GETUSERS);
			break;
		}
		case(4):
		{
			Message::send(MR_BROKER, MT_EXIT);
			return;
		}
		default:
		{
			cout << "Wrong action!\n";
		}
		}

	}

}
