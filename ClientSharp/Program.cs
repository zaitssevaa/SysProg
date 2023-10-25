using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace ClientSharp
{ 
    class Program
    {
        static void ProcessMessages()
        {
            while (true)
            {
                var m = Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_GETDATA);
                switch (m.header.type)
                {
                    case MessageTypes.MT_DATA:
                        Console.WriteLine($"Client {m.header.from}: {m.data}");
                        break;
                    case MessageTypes.MT_GETUSERS:
                        Console.WriteLine("Clients:\n");
                        Console.WriteLine(m.data);
                        break;
                    default:
                        Thread.Sleep(1000);
                        break;
                }
            }
        }
        static void Menu()
        {
            Console.WriteLine("1. Send msg to user");
            Console.WriteLine("2. Send msg to all users");
            Console.WriteLine("3. Get user list");
            Console.WriteLine("4. Disconnect");
        }

        static void Main(string[] args)
        {
            Thread t = new Thread(ProcessMessages);
            t.Start();

            var m = Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_INIT);
            Console.WriteLine($"Welcome, Client: {m.getClientID()}");

            while (true)
            {
                Menu();
                var s = Console.ReadLine();
                //var isNumber = ; //https://www.arungudelli.com/ru/tutorial/c-sharp/check-if-string-is-number/
                if (int.TryParse(s, out int i))
                {
                    switch (i)
                    {
                        case 1:
                            {
                                Console.WriteLine("To: Client ");
                                var client = Console.ReadLine();
                                var isNumber1 = int.TryParse(client, out int id);
                                Console.WriteLine("Message: ");
                                var text = Console.ReadLine();
                                if (isNumber1 && (text is not null))
                                {
                                    Message.send((MessageRecipients)id, MessageTypes.MT_DATA, text);
                                }
                                break;
                            }
                        case 2:
                            {
                                Console.WriteLine("To all Clients");
                                Console.WriteLine("Message: ");
                                var text = Console.ReadLine();
                                if (text is not null)
                                {
                                    Message.send(MessageRecipients.MR_ALL, MessageTypes.MT_DATA, text);
                                }
                                break;
                            }
                        case 3:
                            {
                                Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_GETUSERS);
                                break;
                            }
                        case 4:
                            {
                                Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_EXIT);
                                Environment.Exit(0);
                                break;
                            }
                        default:
                            {
                                Console.WriteLine("Wrong action!");
                                break;
                            }
                    }
                }
            }
        }
    }
}
