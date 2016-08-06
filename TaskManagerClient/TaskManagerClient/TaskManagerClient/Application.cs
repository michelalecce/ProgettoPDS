using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;


namespace TaskManagerClient
{
    class Application{

        //properties with relative public fields getter and setter
        private Stopwatch _focusTime;
        private UInt32 _appID;
        private UInt32 _nameSize=0;
        private UInt32 _iconSize=0;
        private string _appName = null;
        private byte[] _icon = null;

        public Stopwatch FocusTime{
            get{
                return _focusTime;
            }
            set{
                _focusTime = value; 
            }
        }
        public UInt32 AppID{
            get{
                return _appID;
            }
        }
        public UInt32 NameSize {
            get {
                return _nameSize;
            }
            set {
                _nameSize = value;
            }
        }
        public UInt32 IconSize {
            get {
                return _iconSize;
            }
            set {
                _iconSize = value;
            }
        }
        public string AppName {
            get {
                return _appName;
            }
            set {
                _appName = value;
            }
        }
        public byte[] Icon {
            get {
                return _icon;
            }
            set {
                if (_icon!=null && _iconSize != 0) {
                    _icon = new byte[_iconSize];
                }
                _icon = value;
            }
        }
        
        //constructor
        public Application(UInt32 id) {
            this._appID = id;
        } 
    }
}
