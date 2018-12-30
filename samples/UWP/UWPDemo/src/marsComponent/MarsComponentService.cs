using mars;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage;

namespace UWPDemo.marsComponent
{
    static public class MarsComponentService
    {
        static public void Init()
        {
            MarsRuntimeComponent.Init(new MarsCallback());
            MarsRuntimeComponent.OnCreate();
            MarsRuntimeComponent.OnForeground(true);

            StnComponent.SetClientVersion(200);
            StnComponent.SetLonglinkSvrAddr("www.marsopen.cn", new ushort[] { 8081 });
            StnComponent.SetShortlinkSvrAddr(8080);
            StnComponent.MakesureLonglinkConnected();

            LogComponent.AppenderOpen(TAppenderModeRuntime.kAppednerSync, ApplicationData.Current.LocalFolder.Path + "\\log", "sample");
            LogComponent.AppenderSetConsoleLog(true);
            LogComponent.SetLogLevel(TLogLevelRuntime.kLevelDebug);
            //LogComponent.LogWrite(TLogLevelRuntime.kLevelDebug, "uwp sample", "", "", 0, 0, 0, 0, "test log");
        }
    }
}
