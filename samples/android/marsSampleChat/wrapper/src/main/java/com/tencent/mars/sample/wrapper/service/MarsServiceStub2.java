package com.tencent.mars.sample.wrapper.service;

import android.content.Context;
import android.os.Bundle;
import android.os.RemoteException;

import com.tencent.mars.BaseEvent;
import com.tencent.mars.app.AppLogic;
import com.tencent.mars.app.AppManager;
import com.tencent.mars.sample.utils.print.BaseConstants;
import com.tencent.mars.sample.wrapper.remote.MarsPushMessageFilter;
import com.tencent.mars.sample.wrapper.remote.MarsService;
import com.tencent.mars.sample.wrapper.remote.MarsTaskProperty;
import com.tencent.mars.sample.wrapper.remote.MarsTaskWrapper;
import com.tencent.mars.sdt.SdtManager;
import com.tencent.mars.stn.ErrCmdType;
import com.tencent.mars.stn.StnLogic;
import com.tencent.mars.stn.StnManager;
import com.tencent.mars.xlog.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

public class MarsServiceStub2 extends MarsService.Stub implements StnManager.CallBack , SdtManager.CallBack, AppManager.CallBack {

    private static final String TAG = "Mars.Sample.MarsServiceStub2";

    private final MarsServiceProfile profile;

    private AppLogic.AccountInfo accountInfo = new AppLogic.AccountInfo();

    public static final String DEVICE_NAME = android.os.Build.MANUFACTURER + "-" + android.os.Build.MODEL;
    public static String DEVICE_TYPE = "android-" + android.os.Build.VERSION.SDK_INT;
    private AppLogic.DeviceInfo info = new AppLogic.DeviceInfo(DEVICE_NAME, DEVICE_TYPE);

    private Context context;

    private ConcurrentLinkedQueue<MarsPushMessageFilter> filters = new ConcurrentLinkedQueue<>();

    private int clientVersion = 200;

    public MarsServiceStub2(Context context, MarsServiceProfile profile) {
        this.context = context;
        this.profile = profile;
    }

    private static final int FIXED_HEADER_SKIP = 4 + 2 + 2 + 4 + 4;

    private static Map<Integer, MarsTaskWrapper> TASK_ID_TO_WRAPPER = new ConcurrentHashMap<>();


    //start MarsService.Stub
    @Override
    public int send(MarsTaskWrapper taskWrapper, Bundle taskProperties) throws RemoteException {
        final StnLogic.Task _task = new StnLogic.Task(StnLogic.Task.EShort, 0, "", null);

        // Set host & cgi path
        final String host = taskProperties.getString(MarsTaskProperty.OPTIONS_HOST);
        final String cgiPath = taskProperties.getString(MarsTaskProperty.OPTIONS_CGI_PATH);
        _task.shortLinkHostList = new ArrayList<>();
        _task.shortLinkHostList.add(host);
        _task.cgi = cgiPath;

        final boolean shortSupport = taskProperties.getBoolean(MarsTaskProperty.OPTIONS_CHANNEL_SHORT_SUPPORT, true);
        final boolean longSupport = taskProperties.getBoolean(MarsTaskProperty.OPTIONS_CHANNEL_LONG_SUPPORT, false);
        if (shortSupport && longSupport) {
            _task.channelSelect = StnLogic.Task.EBoth;

        } else if (shortSupport) {
            _task.channelSelect = StnLogic.Task.EShort;

        } else if (longSupport) {
            _task.channelSelect = StnLogic.Task.ELong;

        } else {
            Log.e(TAG, "invalid channel strategy");
            throw new RemoteException("Invalid Channel Strategy");
        }

        // Set cmdID if necessary
        int cmdID = taskProperties.getInt(MarsTaskProperty.OPTIONS_CMD_ID, -1);
        if (cmdID != -1) {
            _task.cmdID = cmdID;
        }

        TASK_ID_TO_WRAPPER.put(_task.taskID, taskWrapper);

        // Send
        Log.i(TAG, "now start task with id %d", _task.taskID);
        MarsContext.getInstance().getManager(StnManager.class).startTask(_task);
        if (StnLogic.hasTask(_task.taskID)) {
            Log.i(TAG, "stn task started with id %d", _task.taskID);

        } else {
            Log.e(TAG, "stn task start failed with id %d", _task.taskID);
        }

        return _task.taskID;
    }

    @Override
    public void cancel(int taskID) throws RemoteException {
        Log.d(TAG, "cancel wrapper with taskID=%d using stn stop", taskID);
        MarsContext.getInstance().getManager(StnManager.class).stopTask(taskID);
        TASK_ID_TO_WRAPPER.remove(taskID); // TODO: check return
    }

    @Override
    public void registerPushMessageFilter(MarsPushMessageFilter filter) throws RemoteException {
        filters.remove(filter);
        filters.add(filter);
    }

    @Override
    public void unregisterPushMessageFilter(MarsPushMessageFilter filter) throws RemoteException {
        filters.remove(filter);
    }

    @Override
    public void setAccountInfo(long uin, String userName) {
        accountInfo.uin = uin;
        accountInfo.userName = userName;
    }

    @Override
    public void setForeground(int isForeground) {
        BaseEvent.onForeground(isForeground == 1);
    }//end MarsService.Stub

    //start StnManager.Callback


    @Override
    public boolean makesureAuthed(String host, String user_id) {
        // Allow you to block all tasks which need to be sent before certain 'AUTHENTICATED' actions
        // Usually we use this to exchange encryption keys, sessions, etc.
        //
        return true;
    }

    @Override
    public void trafficData(int send, int recv) {
        onPush("", BaseConstants.FLOW_CMDID, 0, String.format("%d,%d", send, recv).getBytes(Charset.forName("UTF-8")), new byte[0]);

    }

    @Override
    public String[] onNewDns(String host, boolean longlinkHost) {
        // No default new dns support
        return new String[0];
    }

    @Override
    public void onPush(String channelId, int cmdId, int taskId, byte[] body, byte[] extend) {
        for (MarsPushMessageFilter filter : filters) {
            try {
                if (filter.onRecv(cmdId, body)) {
                    break;
                }

            } catch (RemoteException e) {
                //
            }
        }
    }

    @Override
    public boolean req2Buf(int taskId, Object userContext, String userId, ByteArrayOutputStream reqBuffer, int[] errCode, int channelSelect, String host) {
        final MarsTaskWrapper wrapper = TASK_ID_TO_WRAPPER.get(taskId);
        if (wrapper == null) {
            Log.e(TAG, "invalid req2Buf for task, taskID=%d", taskId);
            return false;
        }

        try {
            reqBuffer.write(wrapper.req2buf());
            return true;

        } catch (IOException | RemoteException e) {
            e.printStackTrace();
            Log.e(TAG, "task wrapper req2buf failed for short, check your encode process");
        }

        return false;
    }

    @Override
    public int buf2Resp(int taskId, Object userContext, String userId, byte[] respBuffer, int[] errCode, int channelSelect) {
        final MarsTaskWrapper wrapper = TASK_ID_TO_WRAPPER.get(taskId);
        if (wrapper == null) {
            Log.e(TAG, "buf2Resp: wrapper not found for stn task, taskID=%", taskId);
            return StnLogic.RESP_FAIL_HANDLE_TASK_END;
        }

        try {
            return wrapper.buf2resp(respBuffer);

        } catch (RemoteException e) {
            Log.e(TAG, "remote wrapper disconnected, clean this context, taskID=%d", taskId);
            TASK_ID_TO_WRAPPER.remove(taskId);
        }
        return StnLogic.RESP_FAIL_HANDLE_TASK_END;
    }

    @Override
    public int onTaskEnd(int taskId, Object userContext, int errType, int errCode, StnLogic.CgiProfile profile) {
        final MarsTaskWrapper wrapper = TASK_ID_TO_WRAPPER.remove(taskId);
        if (wrapper == null) {
            Log.w(TAG, "stn task onTaskEnd callback may fail, null wrapper, taskID=%d", taskId);
            return 0; // TODO: ???
        }

        try {
            wrapper.onTaskEnd(errType, errCode);

        } catch (RemoteException e) {
            e.printStackTrace();

        }

        return 0;
    }

    @Override
    public void reportConnectStatus(int status, int longLinkstatus) {

    }

    @Override
    public void onLongLinkNetworkError(ErrCmdType errType, int errCode, String ip, int port) {

    }

    @Override
    public void onShortLinkNetworkError(ErrCmdType errType, int errCode, String ip, String _host, int port) {

    }

    @Override
    public int getLongLinkIdentifyCheckBuffer(String channelId, ByteArrayOutputStream identifyReqBuf, ByteArrayOutputStream hashCodeBuffer, int[] reqRespCmdID) {
        // Send identify request buf to server
        // identifyReqBuf.write();

        return StnLogic.ECHECK_NEVER;
    }

    @Override
    public boolean onLongLinkIdentifyResponse(String channelId, byte[] buffer, byte[] hashCodeBuffer) {
        return false;
    }

    @Override
    public void requestSync() {

    }

    @Override
    public void onLongLinkStatusChange(int status) {

    }

    @Override
    public void networkAnalysisCallBack(int status, int stage, boolean isDetectEnd, String kvInfo) {

    }

    @Override
    public String[] requestNetCheckShortLinkHosts() {
        return new String[0];
    }

    @Override
    public void reportTaskProfile(String taskString) {
        onPush("", BaseConstants.CGIHISTORY_CMDID, 0, taskString.getBytes(Charset.forName("UTF-8")),new byte[0]);

    }

    //end StnManager.Callback

    //start SdtManager.CallBack
    @Override
    public void reportSignalDetectResults(String reportString) {
        onPush("", BaseConstants.SDTRESULT_CMDID, 0, reportString.getBytes(Charset.forName("UTF-8")), new byte[0]);
    }//end SdtManager.CallBack

    //start AppManager.CallBack
    @Override
    public String getAppFilePath() {
        if (null == context) {
            return null;
        }

        try {
            File file = context.getFilesDir();
            if (!file.exists()) {
                file.createNewFile();
            }
            return file.toString();
        } catch (Exception e) {
            Log.e(TAG, "", e);
        }

        return null;
    }

    @Override
    public AppLogic.AccountInfo getAccountInfo() {
        return accountInfo;
    }

    @Override
    public int getClientVersion() {
        return clientVersion;
    }

    @Override
    public AppLogic.DeviceInfo getDeviceType() {
        return info;
    }
    //end AppManager.CallBack
}
