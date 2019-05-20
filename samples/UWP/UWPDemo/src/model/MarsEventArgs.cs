using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UWPDemo.model
{
    public enum EventConst
    {
        SUCCESS = 1,
        FAIL,
    }

    public class MarsEventArgs : EventArgs
    {
        public EventConst Code { get; set; }

        public Object Data { get; set; }
    }
}
