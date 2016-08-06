using System;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientProva {
    class TaskManager {

        Dictionary<UInt32, Application> applicationList;
        Application focus = null;
        UInt32 appListLen = 0;
        //All socket variables 
        IPHostEntry ipHostInfo = null;
        IPAddress ipAddress = null;
        IPEndPoint remoteEP = null;
        SocketType sockType = SocketType.Stream;
        ProtocolType sockProtocol = ProtocolType.Tcp;
        Socket clientSocket = null;
        bool processData = true;
        bool firstCom = true;
        string cmd = null;

        public UInt32 readNum() {
            UInt32 num = 0;
            byte[] array = new byte[4];
            int bytesRec = clientSocket.Receive(array);
            if (bytesRec == 4) {
                num = BitConverter.ToUInt32(array, 0);
            }
            return num;
        }

        public void sendErr() {
            //Send the command err
            byte[] sendBuffer = new byte[3];
            sendBuffer = System.Text.Encoding.ASCII.GetBytes("err");
            int rc = clientSocket.Send(sendBuffer);
            Console.WriteLine("Client: Sent command of app over {0} bytes", rc);
        }

        public Application readApp() {

            //Read the application id and create the application instance
            UInt32 id = readNum();
            Application a = new Application(id);

            //Read application name lenght 
            UInt32 nameLen = readNum();
            a.NameSize = nameLen;

            //Read application name
            byte[] array = new byte[nameLen];
            int bytesRec = clientSocket.Receive(array);
            a.AppName = System.Text.Encoding.UTF8.GetString(array, 0, array.Length);

            //Read icon size
            UInt32 icoSize = readNum();
            a.IconSize = icoSize;

            //Read icon data
            array = new byte[icoSize];
            bytesRec = clientSocket.Receive(array);
            a.Icon = array;

            return a;
        }

        public void receiveAppList() {

            //Reading number of total app i.e. lenght of applicationList
            byte[] array = new byte[4];
            int bytesRec = clientSocket.Receive(array);
            if (bytesRec == 4) {
                appListLen = BitConverter.ToUInt32(array, 0);
            }
            applicationList = new Dictionary<UInt32, Application>();

            //Adding application to the list 
            for (int i = 0; i < appListLen; i++) {
                Application a = readApp();
                applicationList.Add(a.AppID, a);
                //Console.WriteLine("App "+ a.ToString()+" added");
            }
        }

        public void addApp() {

            //Read the new application 
            Application a = readApp();
            applicationList.Add(a.AppID, a);
            Console.WriteLine("App " + a.ToString() + " added");
        }

        public void removeApp() {

            //Delete the app with corresponding AppID
            UInt32 id = 0;
            byte[] array = new byte[4];
            int bytesRec = clientSocket.Receive(array);
            if (bytesRec == 4) {
                id = BitConverter.ToUInt32(array, 0);
            }
            applicationList.Remove(id);
        }

        public void changeFocus() {

            UInt32 id = readNum();

            //Check if focus object is already created, if not send err message
            bool ok = applicationList.TryGetValue(id, out focus);
            if (!ok) {
                //Err message to the server! Focus is not valid
                sendErr();
            }
            
        }

        public void sendSockCommand() {


        }

        public void readSockCommand() {
            byte[] command = new byte[3];
            int bytesRec = clientSocket.Receive(command);
            cmd = System.Text.Encoding.UTF8.GetString(command, 0, command.Length);
            //Console.WriteLine("Comando: "+cmd+ " Originale: "+command);
        }

        public void printList() {
            try {
                foreach (KeyValuePair<UInt32, Application> kvp in applicationList) {
                    Console.WriteLine("ID = " + kvp.Key + " Value = " + kvp.Value.toString());
                }
            }catch (NullReferenceException) {
                Console.WriteLine("Empty applicationList");
            }
        }

        public void sockConnect() {

            try {
                // Establish the remote endpoint for the socket.
                string hostname = "localhost";
                int portNum = 1500;
                ipHostInfo = Dns.GetHostEntry(hostname);
                ipAddress = ipHostInfo.AddressList[1];
                remoteEP = new IPEndPoint(ipAddress, portNum);

                // Create a TCP/IP  client socket
                clientSocket = new Socket(AddressFamily.InterNetwork, sockType, sockProtocol);
                Console.WriteLine("Socket created");

                // Connect the socket to the remote endpoint (server). Catch any errors.
                try {
                    clientSocket.Connect(remoteEP);
                    Console.WriteLine("Socket connected to {0}", clientSocket.RemoteEndPoint.ToString());

                    while (processData) {
                        readSockCommand();

                        switch (cmd) {
                            case "lis":
                                Console.WriteLine("Command lis");
                                receiveAppList();
                                //First time we want to be sure that the second command received is foc
                                //If not send err to server
                                
                                if (firstCom == true) {
                                    Console.WriteLine("Command foc");
                                    readSockCommand();
                                    if (String.Compare(cmd, 0, "foc", 0, 3) == 0)
                                        changeFocus();
                                    else
                                        sendErr();
                                }
                                
                                break;
                            case "add":
                                Console.WriteLine("Command add");
                                addApp();
                                break;
                            case "rem":
                                Console.WriteLine("Command rem");
                                removeApp();
                                break;
                            case "foc":
                                Console.WriteLine("Command foc");
                                changeFocus();
                                break;
                            case "com":
                                sendSockCommand();
                                break;
                            case "err":
                                Console.WriteLine("Error received from the server");
                                break;
                            default:
                                //Temporary in order to get out of the while loop
                                processData = false;
                                Console.WriteLine("Unknown command");
                                break;
                        }
                    }

                    // Release the socket.
                    clientSocket.Shutdown(SocketShutdown.Both);
                    clientSocket.Close();

                } catch (ArgumentNullException ane) {
                    Console.WriteLine("ArgumentNullException : {0}", ane.ToString());
                } catch (SocketException se) {
                    Console.WriteLine("SocketException : {0}", se.ToString());
                } catch (Exception e) {
                    Console.WriteLine("Unexpected exception : {0}", e.ToString());
                }
            } catch (Exception e) {
                //Temporary!!! In the GUI there will be a connection failed message
                Console.WriteLine("Client: Socket error occurred: {0}", e.ToString());
            } finally {
                // Close the socket if necessary
                if (clientSocket != null) {
                    clientSocket.Close();
                }
            }
        }
    }
}
