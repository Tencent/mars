package com.tencent.mars.comm;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Process;
import android.os.SystemClock;

import com.tencent.mars.xlog.Log;


import java.util.Comparator;
import java.util.Iterator;
import java.util.TreeSet;

/**
 * 定时器工具类，mars会在网络组件stn中使用定时器管理任务队列、连接间隔等
 * Created by caoshaokun on 16/3/7.
 */
public class Alarm extends BroadcastReceiver {

    private final static String TAG = "MicroMsg.Alarm";
    private final static String KEXTRA_ID = "ID";
    private final static String KEXTRA_PID = "PID";

    private enum TSetData {
        ID,
        WAITTIME,
        PENDINGINTENT,
    }

    private static WakerLock wakerlock = null;
    private static Alarm bc_alarm = null;

    private native void onAlarm(final long id);

    private static class ComparatorAlarm implements Comparator<Object[]> {
        @Override
        public int compare(Object[] lhs, Object[] rhs) {
            return (int) ((Long) (lhs[TSetData.ID.ordinal()]) - (Long) (rhs[TSetData.ID.ordinal()]));
        }
    }

    private static TreeSet<Object[]> alarm_waiting_set = new TreeSet<Object[]>(new ComparatorAlarm());

    public static void resetAlarm(final Context context) {
        synchronized (alarm_waiting_set) {
            Iterator<Object[]> it = alarm_waiting_set.iterator();
            while (it.hasNext()) {
                cancelAlarmMgr(context, (PendingIntent) (it.next()[TSetData.PENDINGINTENT.ordinal()]));
            }
            alarm_waiting_set.clear();
            if (null != bc_alarm) {
                context.unregisterReceiver(bc_alarm);
                bc_alarm = null;
            }
        }
    }

    public static boolean start(final long id, final int after, final Context context) {
        long curtime = SystemClock.elapsedRealtime();

        if (0 > after) {
            Log.e(TAG, "id:%d, after:%d", id, after);
            return false;
        }

        if (null == context) {
            Log.e(TAG, "null==context, id:%d, after:%d", id, after);
            return false;
        }

        synchronized (alarm_waiting_set) {
            if (null == wakerlock) {
                wakerlock = new WakerLock(context);
                Log.i(TAG, "start new wakerlock");
            }

            if (null == bc_alarm) {
                bc_alarm = new Alarm();
                context.registerReceiver(bc_alarm, new IntentFilter("ALARM_ACTION(" + String.valueOf(Process.myPid()) + ")"));
            }

            Iterator<Object[]> it = alarm_waiting_set.iterator();
            while (it.hasNext()) {
                if ((Long) (it.next()[TSetData.ID.ordinal()]) == id) {
                    Log.e(TAG, "id exist=%d", id);
                    return false;
                }
            }

            long waittime = after >= 0 ? curtime + after : curtime;

            PendingIntent pendingIntent = setAlarmMgr(id, waittime, context);
            if (null == pendingIntent) return false;

            alarm_waiting_set.add(new Object[]{id, waittime, pendingIntent});
        }
        return true;
    }

    public static boolean stop(final long id, final Context context) {

        if (null == context) {
            Log.e(TAG, "context==null");
            return false;
        }

        synchronized (alarm_waiting_set) {
            if (null == wakerlock) {
                wakerlock = new WakerLock(context);
                Log.i(TAG, "stop new wakerlock");
            }

            if (null == bc_alarm) {
                bc_alarm = new Alarm();
                IntentFilter filter = new IntentFilter();
                context.registerReceiver(bc_alarm, filter);
                Log.i(TAG, "stop new Alarm");
            }

            Iterator<Object[]> it = alarm_waiting_set.iterator();

            while (it.hasNext()) {
                Object[] next = it.next();
                if ((Long) (next[TSetData.ID.ordinal()]) == id) {
                    cancelAlarmMgr(context, (PendingIntent) (next[TSetData.PENDINGINTENT.ordinal()]));
                    it.remove();
                    return true;
                }
            }

        }

        return false;
    }

    private static PendingIntent setAlarmMgr(final long id, final long time, final Context context) {
        final AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        if (am == null) {
            Log.e(TAG, "am == null");
            return null;
        }

        Intent intent = new Intent();
        intent.setAction("ALARM_ACTION(" + String.valueOf(Process.myPid()) + ")");
        intent.putExtra(KEXTRA_ID, id);
        intent.putExtra(KEXTRA_PID, Process.myPid());
        PendingIntent pendingIntent = PendingIntent.getBroadcast(context, (int) id, intent, PendingIntent.FLAG_CANCEL_CURRENT);


        if (Build.VERSION.SDK_INT < 19) { //KITKAT
            am.set(AlarmManager.ELAPSED_REALTIME_WAKEUP, time, pendingIntent);
        } else {
            am.setExact(AlarmManager.ELAPSED_REALTIME_WAKEUP, time, pendingIntent);
        }

        return pendingIntent;
    }

    private static boolean cancelAlarmMgr(final Context context, PendingIntent pendingIntent) {
        final AlarmManager am = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        if (am == null) {
            Log.e(TAG, "am == null");
            return false;
        }
        if (pendingIntent == null) {
            Log.e(TAG, "pendingIntent == null");
            return false;
        }

        am.cancel(pendingIntent);
        pendingIntent.cancel();
        return true;
    }

    @Override
    public void onReceive(final Context context, Intent intent) {

        if (null == context || null == intent) return;

        final Long id = intent.getLongExtra(KEXTRA_ID, 0);
        final Integer pid = intent.getIntExtra(KEXTRA_PID, 0);

        if (0 == id || 0 == pid) return;

        if (pid != Process.myPid()) {
            Log.w(TAG, "onReceive id:%d, pid:%d, mypid:%d", id, pid, Process.myPid());
            return;
        }

        boolean found = false;
        synchronized (alarm_waiting_set) {
            Iterator<Object[]> it = alarm_waiting_set.iterator();
            while (it.hasNext()) {
                Object[] next = it.next();
                Long curId = (Long) next[TSetData.ID.ordinal()];
                Log.i(TAG, "onReceive id=%d, curId=%d", id, curId);
                if (curId.equals(id)) {

                    Log.i(TAG, "onReceive find alarm id:%d, pid:%d, delta miss time:%d", id, pid, SystemClock.elapsedRealtime() - (Long) next[TSetData.WAITTIME.ordinal()]);
                    it.remove();
                    found = true;
                    break;
                }
            }
            if (!found)
                Log.e(TAG, "onReceive not found id:%d, pid:%d, alarm_waiting_set.size:%d", id, pid, alarm_waiting_set.size());
        }

        if (null != wakerlock) wakerlock.lock(200);
        if (found) onAlarm(id);
    }
}
