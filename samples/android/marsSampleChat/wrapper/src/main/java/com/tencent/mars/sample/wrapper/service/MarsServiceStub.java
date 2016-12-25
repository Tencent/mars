package com.tencent.mars.sample.wrapper.service;

import android.content.Context;
import android.os.Bundle;
import android.os.RemoteException;

import com.google.gson.Gson;
import com.tencent.mars.app.AppLogic;
import com.tencent.mars.sample.utils.print.BaseConstants;
import com.tencent.mars.sample.wrapper.remote.MarsRecvCallBack;
import com.tencent.mars.sample.wrapper.remote.MarsService;
import com.tencent.mars.sample.wrapper.remote.MarsTaskProperty;
import com.tencent.mars.sample.wrapper.remote.MarsTaskWrapper;
import com.tencent.mars.sdt.SdtLogic;
import com.tencent.mars.sdt.SignalDetectResult;
import com.tencent.mars.stn.StnLogic;
import com.tencent.mars.stn.TaskProfile;
import com.tencent.mars.xlog.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Mars Task Wrapper implements
 */
public class MarsServiceStub extends MarsService.Stub implements StnLogic.ICallBack, SdtLogic.ICallBack, AppLogic.ICallBack{

    private static final String TAG = "Mars.Sample.MarsServiceStub";

    private final MarsServiceProfile profile;

    private AppLogic.AccountInfo accountInfo = new AppLogic.AccountInfo();

    public static final String DEVICE_NAME = android.os.Build.MANUFACTURER + "-" + android.os.Build.MODEL;
    public static String DEVICE_TYPE = "android-" + android.os.Build.VERSION.SDK_INT;
    private AppLogic.DeviceInfo info = new AppLogic.DeviceInfo(DEVICE_NAME, DEVICE_TYPE);

    private Context context;

    private MarsRecvCallBack recvCallBack;

    private int clientVersion = 200;

    public MarsServiceStub(Context context, MarsServiceProfile profile) {
        this.context = context;
        this.profile = profile;
    }

    private static final int FIXED_HEADER_SKIP = 4 + 2 + 2 + 4 + 4;

    private static Map<Integer, MarsTaskWrapper> TASK_ID_TO_WRAPPER = new ConcurrentHashMap<>();
    private static Map<MarsTaskWrapper, Integer> WRAPPER_TO_TASK_ID = new ConcurrentHashMap<>();

    @Override
    public void send(final MarsTaskWrapper taskWrapper, Bundle taskProperties) throws RemoteException {
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
        WRAPPER_TO_TASK_ID.put(taskWrapper, _task.taskID);

        // Send
        Log.i(TAG, "now start task with id %d", _task.taskID);
        StnLogic.startTask(_task);
        if (StnLogic.hasTask(_task.taskID)) {
            Log.i(TAG, "stn task started with id %d", _task.taskID);

        } else {
            Log.e(TAG, "stn task start failed with id %d", _task.taskID);
        }
    }

    @Override
    public void cancel(MarsTaskWrapper taskWrapper) throws RemoteException {
        if (taskWrapper == null) {
            Log.e(TAG, "cannot cancel null wrapper");
            return;
        }

        final Integer taskID = WRAPPER_TO_TASK_ID.remove(taskWrapper);
        if (taskID == null) {
            Log.w(TAG, "cancel null taskID wrapper");

        } else {
            Log.d(TAG, "cancel wrapper with taskID=%d using stn stop", taskID);
            StnLogic.stopTask(taskID);
            TASK_ID_TO_WRAPPER.remove(taskID); // TODO: check return
        }
    }

    @Override
    public void setRecvCallBack(MarsRecvCallBack callBack) {
        recvCallBack = callBack;
    }

    @Override
    public void setAccountInfo(long uin, String userName) {
        accountInfo.uin = uin;
        accountInfo.userName = userName;
    }

    @Override
    public boolean makesureAuthed() {
        //
        // Allow you to block all tasks which need to be sent before certain 'AUTHENTICATED' actions
        // Usually we use this to exchange encryption keys, sessions, etc.
        //
        return true;
    }

    @Override
    public String[] onNewDns(String host) {
        return new String[]{
                "118.89.24.72"
        };
    }

    @Override
    public void onPush(int cmdid, byte[] data) {
        if (recvCallBack != null) {
            try {
                recvCallBack.onRecv(cmdid, data);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void reportFlow(int wifiRecv, int wifiSend, int mobileRecv, int mobileSend) {

        try {
            if (recvCallBack != null) {
                recvCallBack.onRecv(BaseConstants.FLOW_CMDID, String.format("%d,%d,%d,%d", wifiRecv, wifiSend, mobileRecv, mobileSend).getBytes(Charset.forName("UTF-8")));
            }
        }
        catch (Exception e) {
            Log.e(TAG, "report flow error %s", e.toString());
        }

    }

    @Override
    public void reportConnectInfo(int status, int longlinkstatus) {

    }

    @Override
    public int getLongLinkIdentifyCheckBuffer(ByteArrayOutputStream identifyReqBuf, ByteArrayOutputStream hashCodeBuffer, int[] reqRespCmdID) {
        // Send identify request buf to server
        // identifyReqBuf.write();

        return StnLogic.ECHECK_NEVER;
    }

    @Override
    public boolean onLongLinkIdentifyResp(byte[] buffer, byte[] hashCodeBuffer) {
        return false;
    }

    @Override
    public void requestDoSync() {

    }

    @Override
    public String[] requestNetCheckShortLinkHosts() {
        return new String[0];
    }

    @Override
    public boolean isLogoned() {
        return false;
    }

    @Override
    public int onTaskEnd(int taskID, Object userContext, int errType, int errCode) {
        final MarsTaskWrapper wrapper = TASK_ID_TO_WRAPPER.remove(taskID);
        if (wrapper == null) {
            Log.w(TAG, "stn task onTaskEnd callback may fail, null wrapper, taskID=%d", taskID);
            return 0; // TODO: ???
        }

        try {
            wrapper.onTaskEnd();

        } catch (RemoteException e) {
            e.printStackTrace();

        } finally {
            WRAPPER_TO_TASK_ID.remove(wrapper); // onTaskEnd will be called only once for each task
        }

        return 0;
    }

    @Override
    public boolean req2Buf(int taskID, Object userContext, ByteArrayOutputStream reqBuffer, int[] errCode, int channelSelect) {
        final MarsTaskWrapper wrapper = TASK_ID_TO_WRAPPER.get(taskID);
        if (wrapper == null) {
            Log.e(TAG, "invalid req2Buf for task, taskID=%d", taskID);
            return false;
        }

        // Short link encode
        Log.d(TAG, "encode for short link, body only");
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
    public int buf2Resp(int taskID, Object userContext, byte[] respBuffer, int[] errCode, int channelSelect) {
        final MarsTaskWrapper wrapper = TASK_ID_TO_WRAPPER.get(taskID);
        if (wrapper == null) {
            Log.e(TAG, "buf2Resp: wrapper not found for stn task, taskID=%", taskID);
            return StnLogic.RESP_FAIL_HANDLE_TASK_END;
        }

        try {
            return wrapper.buf2resp(respBuffer);

        } catch (RemoteException e) {
            Log.e(TAG, "remote wrapper disconnected, clean this context, taskID=%d", taskID);
            TASK_ID_TO_WRAPPER.remove(taskID);
        }
        return StnLogic.RESP_FAIL_HANDLE_TASK_END;
    }

    @Override
    public void reportTaskProfile(String reportString) {
        try {
            if (recvCallBack != null) {
                recvCallBack.onRecv(BaseConstants.CGIHISTORY_CMDID, reportString.getBytes(Charset.forName("UTF-8")));
            }
        }
        catch (Exception e) {
            Log.e(TAG, "task end report error %s", e.toString());
        }
    }

    @Override
    public void reportSignalDetectResults(String reportString) {
        try {
            if (recvCallBack != null) {
                recvCallBack.onRecv(BaseConstants.SDTRESULT_CMDID, reportString.getBytes(Charset.forName("UTF-8")));
            }
        }
        catch (Exception e) {
            Log.e(TAG, "sdt result error %s", e.toString());
        }
    }

    @Override
    public String getAppFilePath() {
        if (null == context){
            return null;
        }

        try {
            File file = context.getFilesDir();
            if(!file.exists()) {
                file.createNewFile();
            }
            return file.toString();
        }
        catch (Exception e) {
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
}