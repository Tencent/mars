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

package com.tencent.mars.sample.chat;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.Editable;
import android.text.InputFilter;
import android.text.TextWatcher;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import com.tencent.mars.sample.R;
import com.tencent.mars.sample.SampleApplicaton;
import com.tencent.mars.sample.core.ActivityEvent;
import com.tencent.mars.sample.core.ActivityEventConnection;
import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;

import java.util.Observable;
import java.util.Observer;

import utils.bindsimple.BindSimple;
import utils.bindsimple.BindView;

@ActivityEventConnection(connect = ActivityEvent.Connect.ChatActivity)
public class ChatActivity extends AppCompatActivity implements Observer {

    public static String TAG = ChatActivity.class.getSimpleName();

    @BindView(R.id.btn_send)
    private Button btnSend;// 发送btn

    @BindView(R.id.et_sendmessage)
    private EditText editTextContent;

    @BindView(R.id.main_toolbar)
    Toolbar mainToolbar;

    @BindView(R.id.title_text)
    TextView titleText;

    @BindView(R.id.chat_content)
    private ListView listView;

    private String topicName;

    private ChatMsgViewAdapter adapter;// 消息视图的Adapter

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_chat);
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);
        BindSimple.bind(this);

        setSupportActionBar(mainToolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(false);  //是否显示显示返回箭头
        getSupportActionBar().setDisplayShowTitleEnabled(false); //是否显示标题

        Intent intent = this.getIntent();
        if (intent != null) {
            topicName = intent.getStringExtra("conversation_id");
            titleText.setText(intent.getStringExtra("notice"));
        } else {
            titleText.setText("Mars Sample");
        }

        adapter = new ChatMsgViewAdapter(this, ChatDataCore.getInstance().getTopicDatas(topicName));
        listView.setAdapter(adapter);
        listView.setSelection(adapter.getCount() - 1);

        ChatDataCore.getInstance().addObserver(this);

        btnSend.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                onClickBtnSend();
            }
        });

        btnSend.setClickable(false);
        editTextContent.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {

            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                if (s.length() > 0) {
                    btnSend.setClickable(true);
                } else {
                    btnSend.setClickable(false);
                }
            }

            @Override
            public void afterTextChanged(Editable s) {

            }
        });

        if (!SampleApplicaton.hasSetUserName) {
            final EditText editText = new EditText(this);
            editText.setFilters(new InputFilter[]{new InputFilter.LengthFilter(12)});
            final AlertDialog d = new AlertDialog.Builder(this)
                    .setTitle("首次进入话题请输入昵称")
                    .setIcon(android.R.drawable.ic_dialog_info)
                    .setView(editText)
                    .setPositiveButton("确定", null).create();
            d.setOnShowListener(new DialogInterface.OnShowListener() {
                @Override
                public void onShow(DialogInterface dialog) {
                    Button b = d.getButton(AlertDialog.BUTTON_POSITIVE);
                    b.setOnClickListener(new OnClickListener() {
                        @Override
                        public void onClick(View view) {
                            String nick = editText.getText().toString();
                            if (nick.length() > 0 && !nick.trim().equals("")) {
                                SampleApplicaton.accountInfo.userName = nick.trim();
                                SampleApplicaton.hasSetUserName = true;
                            } else {
                                return;
                            }

                            d.dismiss();
                        }
                    });
                }
            });
            d.show();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        MarsServiceProxy.inst.setForeground(true);
    }

    public void onPause() {
        super.onPause();
        MarsServiceProxy.inst.setForeground(false);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        ChatDataCore.getInstance().deleteObserver(this);
        //leftTopic(topicName);
    }

    @Override
    public void update(Observable o, Object arg) {
        adapter.notifyDataSetChanged();
        listView.setSelection(listView.getCount() - 1);
    }

    public void onClickBtnSend() {

        final String message = editTextContent.getText().toString();
        if (message.length() <= 0 || message.trim().equals("")) {
            new AlertDialog.Builder(this)
                    .setTitle("不能输入空消息")
                    .setIcon(android.R.drawable.ic_dialog_info)
                    .setPositiveButton("确定", null).show();
            editTextContent.setText("");
            return;
        }

        // Sending using local service proxy

        ChatMsgEntity entity = new ChatMsgEntity();
        entity.setName(SampleApplicaton.accountInfo.userName);
        entity.setDate(ChatDataCore.getDate());
        entity.setMessage(message);
        entity.setMsgType(false);
        ChatDataCore.getInstance().addData(topicName, entity);

        editTextContent.setText("");

        MarsServiceProxy.send(new TextMessageTask(topicName, message)
                .onOK(new Runnable() {

                    @Override
                    public void run() {

                    }

                }).onError(new Runnable() {

                    @Override
                    public void run() {
                    }

                }));
    }

}