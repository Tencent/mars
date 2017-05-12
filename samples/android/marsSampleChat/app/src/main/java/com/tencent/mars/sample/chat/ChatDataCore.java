/*
* Tencent is pleased to support the open source community by making Mars available.
* Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the MIT License (the "License"); you may not use this file except in 
* compliance with the License. You may obtain a copy of the License at
* http://opensource.org/licenses/MIT
*
* Unless required by applicable law or agreed to in writing, software distributed under the License is
* distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
* either express or implied. See the License for the specific language governing permissions and
* limitations under the License.
*/

/**
 * Created by zhouzhijie on 16/12/22.
 */

package com.tencent.mars.sample.chat;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import com.tencent.mars.sample.SampleApplicaton;
import com.tencent.mars.sample.utils.Constants;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Observable;
import java.util.concurrent.ConcurrentHashMap;

public class ChatDataCore extends Observable {

    public static String TAG = ChatActivity.class.getSimpleName();

    private static ChatDataCore inst = new ChatDataCore();

    private BroadcastReceiver receiver = new RecvMessageReceiver();
    private ConcurrentHashMap<String, List<ChatMsgEntity>> dataArrays = new ConcurrentHashMap<>();

    public static ChatDataCore getInstance() {
        return inst;
    }

    public ChatDataCore() {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Constants.PUSHACTION);
        SampleApplicaton.getContext().registerReceiver(receiver, intentFilter);
    }

    @Override
    public void finalize() {
        SampleApplicaton.getContext().unregisterReceiver(receiver);
    }

    public List<ChatMsgEntity> getTopicDatas(String topic) {
        synchronized (this) {
            if (!dataArrays.containsKey(topic)) {
                dataArrays.put(topic, new ArrayList<ChatMsgEntity>());
            }
        }

        return dataArrays.get(topic);
    }

    public void addData(String topic, ChatMsgEntity entity) {
        getTopicDatas(topic).add(entity);
        setChanged();
        notifyObservers();
    }

    /**
     * 发送消息时，获取当前事件.
     *
     * @return 当前时间
     */
    public static String getDate() {
        SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
        return format.format(new Date());
    }

    private class RecvMessageReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {

            if (intent != null && intent.getAction().equals(Constants.PUSHACTION)) {

                String topic = intent.getStringExtra("msgtopic");
                String from = intent.getStringExtra("msgfrom");
                String text = intent.getStringExtra("msgcontent");

                synchronized (this) {
                    if (!dataArrays.containsKey(topic)) {
                        dataArrays.put(topic, new ArrayList<ChatMsgEntity>());
                    }

                    ChatMsgEntity entity = new ChatMsgEntity();
                    entity.setName(from);
                    entity.setDate(getDate());
                    entity.setMessage(text);
                    entity.setMsgType(true);
                    dataArrays.get(topic).add(entity);
                }

                setChanged();
                notifyObservers();
            }

        }
    }
}
