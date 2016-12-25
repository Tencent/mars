package com.tencent.mars.sample;

import android.databinding.BaseObservable;
import android.databinding.Bindable;

/**
 * Created by kirozhao on 16/2/18.
 */
public class Conversation extends BaseObservable {

    private String name;

    private String topic;
    private String notice;


    @Bindable
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }


    public Conversation(final String name, final String topic, final String notice) {
        this.name = name;
        this.topic = topic;
        this.notice = notice;
    }

    @Bindable
    public String getNotice() {
        return notice;
    }

    public void setNotice(String notice) {
        this.notice = notice;
        notifyPropertyChanged(com.tencent.mars.sample.BR.notice);
    }


    @Bindable
    public String getTopic() {
        return topic;
    }

    public void setTopic(String topic) {
        this.topic = topic;
        notifyPropertyChanged(com.tencent.mars.sample.BR.topic);
    }
}
