using mars;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UWPDemo.marsComponent
{
    static public class MarsComponentService
    {
        static public void Init()
        {
            MarsRuntimeComponent.Init(new MarsCallback());
            MarsRuntimeComponent.onCreate();
            MarsRuntimeComponent.OnForeground(true);

            StnComponent.SetClientVersion(200);
            StnComponent.SetLonglinkSvrAddr("www.marsopen.cn", new ushort[] { 8081 });
            StnComponent.SetShortlinkSvrAddr(8080);
            StnComponent.MakesureLonglinkConnected();

            LogComponent.appender_open_(TAppenderModeRuntime.kAppednerSync, "/marslog.txt", "sample");
            LogComponent.appender_set_console_log_(true);
            LogComponent.setLogLevel(0);

        }
    }
}
