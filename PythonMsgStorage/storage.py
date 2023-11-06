import threading
from dataclasses import dataclass
import time, json,os
from msg import * 

def ProcessMessages():
    while True:
        m = Message.SendMessage(MR_BROKER, MT_GETDATA)
        if (m.Header.Type == MT_DATA):
            print("MT_DATA")
            print(m.Header.To)
            print(m.Header.From)
            print(m.Data)
            data = []
            try:
                with open('msgs.json', 'r') as f:
                    data = json.load(f)
            except:
                with open('msgs.json', 'w') as f:
                    pass

            with open('msgs.json', 'w') as f:
                temp = {m.Header.From: m.Data}
                data.append(temp)
                json.dump(data, f)
                print(f"New msg added to {m.Header.From}. \n")

        if (m.Header.Type == MT_GETLAST):
            print("MT_GETLAST")
            print(m.Header.To)
            print(m.Header.From)
            print(m.Data)
            taker = str(m.Header.From)
            with open('msgs.json', 'r') as f:
                data = json.load(f)
            text = ""
            for item in data:
                for key, value in item.items():
                    if (key == taker):
                        text += value
                        text += ","
            text = text[:-1]
            Message.SendMessage(m.Header.From, MT_GETLAST, text)
            print(f"Last msgs sent to {taker}: {text}. \n")
        else:
            time.sleep(1)

        
def Storage():
    Message.SendMessage(MR_BROKER, MT_INITSTORAGE)
    t = threading.Thread(target=ProcessMessages, daemon = True)
    t.start()
    while(1):
        time.sleep(1)             
        
Storage()

