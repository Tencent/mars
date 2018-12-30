using mars;
using System.Collections.Generic;

namespace UWPDemo.scene
{
    static class MarsTaskMgr
    {
        static object sObjLocker = new object();
        static Dictionary<int, MarsTaskWrapperBase> sMapSeqToTask = new Dictionary<int, MarsTaskWrapperBase>();
        static public bool doTask(MarsTaskWrapperBase netScene)
        {
            lock(sObjLocker)
            {
                if (null == netScene || sMapSeqToTask.ContainsKey(netScene.mTaskId))
                {
                    return false;
                }

                TaskRuntime task = netScene.getTask();
                if (null == task)
                {
                    return false;
                }

                StnComponent.StartTask(task);

                sMapSeqToTask[netScene.mTaskId] = netScene;

                return true;
            }
        }

        static public MarsTaskWrapperBase getTask(int nTaskId)
        {
            lock (sObjLocker)
            {
                if (!sMapSeqToTask.ContainsKey(nTaskId))
                {
                    return null;
                }

                MarsTaskWrapperBase netScene = sMapSeqToTask[nTaskId];

                return netScene;
            }

        }

        static public void onTaskEnd(int nTaskId, int error_type, int error_code)
        {
            MarsTaskWrapperBase netScene = getTask(nTaskId);
            if (null == netScene)
            {
                return;
            }


            lock (sObjLocker)
            {
                sMapSeqToTask.Remove(nTaskId);
            }

            Windows.ApplicationModel.Core.CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                netScene.onCompleted(error_type, error_code);
            });
        }
    }
}
