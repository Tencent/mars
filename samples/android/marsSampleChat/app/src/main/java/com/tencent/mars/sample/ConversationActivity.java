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

package com.tencent.mars.sample;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.DefaultItemAnimator;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.tencent.mars.sample.chat.ChatActivity;
import com.tencent.mars.sample.core.ActivityEvent;
import com.tencent.mars.sample.core.MainService;
import com.tencent.mars.sample.proto.Main;
import com.tencent.mars.sample.statistic.ReportDisplayActivity;
import com.tencent.mars.sample.utils.Constants;
import com.tencent.mars.sample.utils.print.BaseConstants;
import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;
import com.tencent.mars.sample.wrapper.remote.NanoMarsTaskWrapper;
import com.tencent.mars.xlog.Log;

import java.util.LinkedList;
import java.util.List;

import utils.bindsimple.BindSimple;
import utils.bindsimple.BindView;

public class ConversationActivity extends AppCompatActivity {

    private static final String TAG = "Mars.Sample.ConversationActivity";

    private static final String CONVERSATION_HOST = "marsopen.cn"; // using preset ports

    private int conversationFilterType = Main.ConversationListRequest.DEFAULT;

    @BindView(R.id.conversation_list)
    RecyclerView conversationListView;

    @BindView(R.id.swipe_refresh)
    SwipeRefreshLayout swipeRefreshLayout;

    @BindView(R.id.main_page_load)
    ProgressBar progressBar;

    @BindView(R.id.toolbar)
    Toolbar mMainToolbar;

    @BindView(R.id.main_page_refreshtext)
    TextView mTextView;

    private ConversationListAdapter conversationListAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_conversation);

        BindSimple.bind(this);

        setSupportActionBar(mMainToolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(false);  //是否显示显示返回箭头
        getSupportActionBar().setDisplayShowTitleEnabled(false); //是否显示标题



        conversationListAdapter = new ConversationListAdapter();
        conversationListAdapter.setOnItemClickListener(new ConversationListAdapter.OnItemClickListener() {

            @Override
            public void onItemClick(Conversation conversation, int pos) {
//                ActivityEvent.jumpTo(
//                        ConversationActivity.this, ActivityEvent.Connect.ChatActivity,
//                        new Intent().putExtra("conversation_id", conversation.getTopic())
                Intent intent = new Intent(ConversationActivity.this, ChatActivity.class);
                intent.putExtra("conversation_id", conversation.getTopic());
                intent.putExtra("notice", conversation.getNotice());
                startActivity(intent);
//                );
            }
        });

        conversationListView.setLayoutManager(new LinearLayoutManager(this));
        conversationListView.setAdapter(conversationListAdapter);
        conversationListView.setItemAnimator(new DefaultItemAnimator());

        swipeRefreshLayout.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {

            @Override
            public void onRefresh() {
                updateConversationTopics();
            }
        });

        mTextView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                updateConversationTopics();
            }
        });

        final MainService mainService = new MainService();
        MarsServiceProxy.setOnPushMessageListener(BaseConstants.CGIHISTORY_CMDID, mainService);
        MarsServiceProxy.setOnPushMessageListener(BaseConstants.CONNSTATUS_CMDID, mainService);
        MarsServiceProxy.setOnPushMessageListener(BaseConstants.FLOW_CMDID, mainService);
        MarsServiceProxy.setOnPushMessageListener(BaseConstants.PUSHMSG_CMDID, mainService);
        MarsServiceProxy.setOnPushMessageListener(BaseConstants.SDTRESULT_CMDID, mainService);

        initTopics();

        TextView tv = new TextView(this);
        tv.setText(
                "\n   请注意：\n\n" +
                "   1. 消息在服务器和客户端均不做存储\n\n" +
                "   2. 服务器仅将消息推送给与其保持长连接的客户端\n\n" +
                "      * 当前长连接不可用时您将无法收到消息推送\n\n" +
                "      * Mars Sample未运行时也将无法收到消息推送\n\n" +
                "   3. 消息使用服务器处理时间排序，并非消息本身的时序");
        new AlertDialog.Builder(this)
                .setTitle("Mars Sample是一个快速聊天工具")
                .setIcon(android.R.drawable.ic_dialog_info).setView(tv).setPositiveButton("确定", null).create().show();
    }

    @Override
    protected void onPause() {
        super.onPause();
        MarsServiceProxy.inst.setForeground(false);
    }

    @Override
    protected void onResume() {
        super.onResume();
        MarsServiceProxy.inst.setForeground(true);
    }

    private void initTopics() {
        updateConversationTopics();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.conversation, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            startActivity(new Intent(this, ReportDisplayActivity.class));
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private NanoMarsTaskWrapper<Main.ConversationListRequest, Main.ConversationListResponse> taskGetConvList = null;

    /**
     * pull conversation list from server
     */
    private void updateConversationTopics() {
        if (taskGetConvList != null) {
            MarsServiceProxy.cancel(taskGetConvList);
        }

        mTextView.setVisibility(View.INVISIBLE);
        progressBar.setVisibility(View.VISIBLE);

        swipeRefreshLayout.setRefreshing(true);

        taskGetConvList = new NanoMarsTaskWrapper<Main.ConversationListRequest, Main.ConversationListResponse>(
                new Main.ConversationListRequest(),
                new Main.ConversationListResponse()
        ) {

            private List<Conversation> dataList = new LinkedList<>();

            @Override
            public void onPreEncode(Main.ConversationListRequest req) {
                req.type = conversationFilterType;
                req.accessToken = ""; // TODO:
            }

            @Override
            public void onPostDecode(Main.ConversationListResponse response) {

            }

            @Override
            public void onTaskEnd(int errType, int errCode) {
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
                        if (response != null) {
                            for (Main.Conversation conv : response.list) {
                                dataList.add(new Conversation(conv.name, conv.topic, conv.notice));
                            }
                        }

                        if (!dataList.isEmpty()) {
                            progressBar.setVisibility(View.INVISIBLE);
                            conversationListAdapter.list.clear();
                            conversationListAdapter.list.addAll(dataList);
                            conversationListAdapter.notifyDataSetChanged();

                            swipeRefreshLayout.setRefreshing(false);

                        }
                        else {
                            Log.i(TAG, "getconvlist: empty response list");
                            progressBar.setVisibility(View.INVISIBLE);
                            mTextView.setVisibility(View.VISIBLE);
                        }
                    }
                });
            }

        };

        MarsServiceProxy.send(taskGetConvList.setHttpRequest(CONVERSATION_HOST, "/mars/getconvlist"));
    }

    /**
     *
     */
    private void checkPermission() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
            }
            else {
                ActivityCompat.requestPermissions(this,
                        new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        Constants.STORAGE_REQUESTCODE);
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        switch (requestCode) {
            case Constants.STORAGE_REQUESTCODE:
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    SampleApplicaton.openXlog();
                } else {

                }
                break;
            default:
                break;
        }
    }
}
