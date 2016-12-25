package com.tencent.mars.sample.wrapper.remote;

import android.os.Bundle;

import com.tencent.mars.sample.utils.print.BundleFormat;
import com.tencent.mars.sample.wrapper.TaskProperty;



/**
 * MarsTaskWrapper using nano protocol buffer encoding
 * <p></p>
 * Created by kirozhao on 16/2/29.
 */
public abstract class AbstractTaskWrapper extends MarsTaskWrapper.Stub {

    private Bundle properties = new Bundle();

    public AbstractTaskWrapper() {

        // Reflects task properties
        final TaskProperty taskProperty = this.getClass().getAnnotation(TaskProperty.class);
        if (taskProperty != null) {
            setHttpRequest(taskProperty.host(), taskProperty.path());
            setShortChannelSupport(taskProperty.shortChannelSupport());
            setLongChannelSupport(taskProperty.longChannelSupport());
            setCmdID(taskProperty.cmdID());
        }
    }

    @Override
    public Bundle getProperties() {
        return properties;
    }

    @Override
    public void onTaskEnd() {
        // TODO: Running in client, need to complete onTaskEnd if service connection break
        // Nothing to do by defaults
    }

    public AbstractTaskWrapper setHttpRequest(String host, String path) {
        properties.putString(MarsTaskProperty.OPTIONS_HOST, ("".equals(host) ? null : host));
        properties.putString(MarsTaskProperty.OPTIONS_CGI_PATH, path);

        return this;
    }

    public AbstractTaskWrapper setShortChannelSupport(boolean support) {
        properties.putBoolean(MarsTaskProperty.OPTIONS_CHANNEL_SHORT_SUPPORT, support);
        return this;
    }

    public AbstractTaskWrapper setLongChannelSupport(boolean support) {
        properties.putBoolean(MarsTaskProperty.OPTIONS_CHANNEL_LONG_SUPPORT, support);
        return this;
    }

    public AbstractTaskWrapper setCmdID(int cmdID) {
        properties.putInt(MarsTaskProperty.OPTIONS_CMD_ID, cmdID);
        return this;
    }

    @Override
    public String toString() {
        return "AbsMarsTask: " + BundleFormat.toString(properties);
    }
}
