import json
from Session import *
import threading
import socket, struct, time, sys
from venv import create
from message import *

def ProcessMessages():
    while True:
        m = Session.getData()
        if m.Type == MT_DATA:
            print(f"Client {m.From} : {m.Data}" )
        elif m.Type == MT_GETUSERS:
            if(len(m.Data) > 0 ):
                print("Clients: ")
                print(m.Data)
            else:
                print("No clients!")
        elif m.Type == MT_GETLAST:
            if(len(m.Data) > 0 ):
                print(f"{m.Data}")
        else:
            time.sleep(2)

def Menu():
        print(" 1. Send msg to user")
        print(" 2. Send msg to all users")
        print(" 3. Get user's list")
        print(" 4. Get last msgs")
        print(" 5. Exit")
        
def Client():
    s = Session()
    t = threading.Thread(target=ProcessMessages, daemon = True)
    t.start()             
    while True:
        Menu()
        sw = input()
        if(sw.isdigit()):
            if sw == "1":
                print('To Client: ')
                to = input()
                if(sw.isdigit()):
                    print('Message: ')
                    s.send(int(to), MT_DATA, input())
                else:
                    print("Wrong action!")

            elif sw == "2":
                print('Message: ')
                s.send(MR_ALL, MT_DATA, input())

            elif sw == "3":
                s.getUsers()

            elif sw == "4":
                s.getLast()

            elif sw == "5":
                break

Client()