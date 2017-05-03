using System.Collections.Generic;
using UWPDemo.model;

namespace UWPDemo.scene
{
    public interface IMarsPushObserver
    {
        void onPush(int cmdID, MarsEventArgs args);
    }
    
    public class MarsPushMgr
    {
        private static object sLocker = new object();
        private static Dictionary<int, List<IMarsPushObserver>> sObserverList = new Dictionary<int, List<IMarsPushObserver>>();
        
        public static void addObserver(int cmdID, IMarsPushObserver obc)
        {
            lock (sLocker)
            {
                if (sObserverList.ContainsKey(cmdID))
                {
                    List<IMarsPushObserver> list = sObserverList[cmdID];
                    list.Add(obc);
                }
                else
                {
                    List<IMarsPushObserver> list = new List<IMarsPushObserver>();
                    list.Add(obc);
                    sObserverList[cmdID] = list;
                }
            }
        }

        public static void delObserver(int cmdID, IMarsPushObserver obc)
        {
            lock (sLocker)
            {
                if (sObserverList.ContainsKey(cmdID))
                {
                    List<IMarsPushObserver> list = sObserverList[cmdID];
                    list.Remove(obc);
                }
            }
        }

        public static void onPush(int cmdID, MarsEventArgs args)
        {
            lock (sLocker)
            {
                if (sObserverList.ContainsKey(cmdID))
                {
                    List<IMarsPushObserver> list = sObserverList[cmdID];
                    foreach (IMarsPushObserver obc in list)
                    {
                        Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
                        {
                            if (obc != null)
                            {
                                obc.onPush(cmdID, args);
                            }
                        });
                    }
                }
            }
        }
    }

}
