package com.tencent.mars.sample;

import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.DefaultItemAnimator;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.tencent.mars.sample.chat.ChatActivity;
import com.tencent.mars.sample.core.ActivityEvent;
import com.tencent.mars.sample.proto.Main;
import com.tencent.mars.sample.statistic.ReportDisplayActivity;
import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;
import com.tencent.mars.sample.wrapper.remote.NanoMarsTaskWrapper;
import com.tencent.mars.xlog.Log;

import java.util.LinkedList;
import java.util.List;

import utils.bindsimple.BindSimple;
import utils.bindsimple.BindView;

public class ConversationActivity extends AppCompatActivity {

    private static final String TAG = "Mars.Sample.ConversationActivity";

    private static final String CONVERSATION_HOST = "www.marsopen.cn"; // using preset ports

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

        initTopics();

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
                // update data list only
                if (response.list == null) {
                    Log.i(TAG, "getconvlist: empty response list");
                    progressBar.setVisibility(View.VISIBLE);
                    return;
                }
                else if (response.list.length == 0) {
                    Log.i(TAG, "getconvlist: empty response list");
                    progressBar.setVisibility(View.VISIBLE);
                    return;
                }

                for (Main.Conversation conv : response.list) {
                    dataList.add(new Conversation(conv.name, conv.topic, conv.notice));
                }
            }

            @Override
            public void onTaskEnd() {
                runOnUiThread(new Runnable() {

                    @Override
                    public void run() {
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
}
