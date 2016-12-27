package com.tencent.mars.comm;

import android.content.Context;
import android.os.Handler;
import android.os.PowerManager;

public class WakerLock {
	private static final String TAG = "MicroMsg.WakerLock";

	private PowerManager.WakeLock wakeLock = null;
	private Handler mHandler = null;
	private Runnable mReleaser = new Runnable() {
		@Override
		public void run() {
			unLock();
		}
	};

	public WakerLock(final Context context) {
		final PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
		wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG);
		wakeLock.setReferenceCounted(false);
		mHandler = new Handler(context.getMainLooper());
	}

	@Override
	protected void finalize() throws Throwable {
		unLock();
	}

	public void lock(final long timeInMills) {
		lock();
		mHandler.postDelayed(mReleaser, timeInMills);
	}

	public void lock() {
		mHandler.removeCallbacks(mReleaser);
		wakeLock.acquire();
	}

	public void unLock() {
		mHandler.removeCallbacks(mReleaser);
		if (wakeLock.isHeld()) {
			wakeLock.release();
		}
	}

	public boolean isLocking() {
		return wakeLock.isHeld();
	}
	
}
