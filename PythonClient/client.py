import threading
from dataclasses import dataclass
import socket, struct, time, sys
from msg import *


def ProcessMessages(status):
    while True:
        m = Message.SendMessage(MR_BROKER, MT_GETDATA)
        if m.Header.Type == MT_DATA:
            print(f"Client {m.Header.From} : {m.Data}")
        elif m.Header.Type == MT_GETUSERS:
             print("Clients: ")
             print(m.Data)
        else:
            time.sleep(1)
        if status():
            break


def Menu():
    print("1. Send msg to user")
    print("2. Send msg to all users")
    print("3. Get user list")
    print("4. Disconnect")


def Client():
    Message.SendMessage(MR_BROKER, MT_INIT)
    ProcMsgStatus = False
    t = threading.Thread(target=ProcessMessages, args=(lambda: ProcMsgStatus,))
    t.start()

    print(f"Welcome, Client {Message.ClientID}")

    while True:
        Menu()
        i = input()
        if (i.isdigit()):
            match i:
                case '1':
                    print('To: Client ')
                    id = input()
                    if (id.isdigit()):
                        print('Message: ')
                        Message.SendMessage(int(id), MT_DATA, input())
                    else:
                        pass
                case '2':
                    print('To all Clients')
                    print('Message: ')
                    Message.SendMessage(MR_ALL, MT_DATA, input())
                case '3':
                    Message.SendMessage(MR_ALL, MT_GETUSERS)
                case '4':
                    Message.SendMessage(MR_ALL, MT_EXIT)
                    ProcMsgStatus = True
                    t.join()
                    break

Client()