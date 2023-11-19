from http.server import HTTPServer, BaseHTTPRequestHandler
import threading
import json
from msg import *
#import RestClient

users = []
clientId = 0
clientmsg = ''

class requestHandler(BaseHTTPRequestHandler):
    global clientId

    def MakeResponse(self, to, From, type, data):
        return '{"to":"' + str(to) + '","type":"' + str(type) + '","data":"' + data + '","from":"' + str(From) + '"}'

    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        #self.send_header('Content-type', 'text/html')
        self.end_headers()

    def do_GET(self):
        self._set_headers()
        #htmlo = "<html><body><h1>HELLO WORLD</h1></body></html>"
        #self.wfile.write(htmlo.encode())
        content_length = int(self.headers['Content-Length'])  # <--- Gets the size of data
        post_data = self.rfile.read(content_length)
        data = post_data.decode('utf-8')
        data = json.loads(data)
        if int(data['type']) == MT_INIT:
            m = Message.SendMessage(int(data['to']), int(data['type']), data['data'])
            self.wfile.write(self.MakeResponse(m.Header.To, m.Header.From, m.Header.Type, m.Data).encode())
            print("Rest client " + str(m.Header.To) + " entered")
        else:
            m = Message.SendAsClient(int(data['to']), int(data['from']), int(data['type']), data['data'])
            self.wfile.write(self.MakeResponse(m.Header.To, m.Header.From, m.Header.Type, m.Data).encode())


def ProcessServer():
    server_address = ("", 8989)
    print("Rest support server has started")
    HTTPServer(server_address, requestHandler).serve_forever()

def ProcessMessages():
    global clientId
    while True:
        m = Message.SendMessage(MR_BROKER, MT_GETDATA)
        clientId = m.Header.To
        if m.Header.Type == MT_DATA:
            print("New message: " + m.Data + "\nFrom: " + str(m.Header.From))
        else:
            time.sleep(1)

def RestServe():
    global clientId
    w = threading.Thread(target=ProcessServer)
    w.start()
    Message.SendMessage(MR_BROKER, MT_REST_SERVER)
    t = threading.Thread(target=ProcessMessages)
    t.start()

RestServe()