using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ClientProva {
    class Program {
        static void Main(string[] args) {

            TaskManager tm = new TaskManager();
            //Connect to localhost and port 1500
            tm.sockConnect();

            tm.printList();
        }
    }
}
