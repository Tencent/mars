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

package com.tencent.mars.sample.statistic;

import android.os.Bundle;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.widget.RadioButton;
import android.widget.RadioGroup;

import com.tencent.mars.sample.R;
import com.tencent.mars.sample.wrapper.remote.MarsServiceProxy;

import utils.bindsimple.BindSimple;
import utils.bindsimple.BindView;

public class ReportDisplayActivity extends AppCompatActivity implements RadioGroup.OnCheckedChangeListener {

    public static String TAG = ReportDisplayActivity.class.getSimpleName();

    @BindView(R.id.main_sheet)
    RadioGroup mainSheet;

    @BindView(R.id.display_toolbar)
    Toolbar toolbar;

    FragmentManager fragmentManager;

    FlowReportFragment flowReportFragment;
    SdtReportFragment sdtReportFragment;
    TaskReportFragment taskReportFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_report_display);
        BindSimple.bind(this);

        setSupportActionBar(toolbar);
        getSupportActionBar().setDisplayHomeAsUpEnabled(false);  //是否显示显示返回箭头
        getSupportActionBar().setDisplayShowTitleEnabled(false); //是否显示标题

        fragmentManager = getSupportFragmentManager();

        ((RadioButton) mainSheet.getChildAt(0)).setChecked(true);
        onCheckedChanged(mainSheet, 1);

        mainSheet.setOnCheckedChangeListener(this);
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
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        FragmentTransaction fragmentTrans = fragmentManager.beginTransaction();
        hideFragments(fragmentTrans);
        int id = (checkedId % 3 == 0 ? 3 : (checkedId % 3));
        switch (id) {
            case 1:
                if (taskReportFragment == null) {
                    taskReportFragment = new TaskReportFragment();
                    fragmentTrans.add(R.id.dis_ll_fragment, taskReportFragment);
                } else {
                    fragmentTrans.show(taskReportFragment);
                }
                break;
            case 2:
                if (flowReportFragment == null) {
                    flowReportFragment = new FlowReportFragment();
                    fragmentTrans.add(R.id.dis_ll_fragment, flowReportFragment);
                } else {
                    fragmentTrans.show(flowReportFragment);
                }
                break;
            case 3:
                if (sdtReportFragment == null) {
                    sdtReportFragment = new SdtReportFragment();
                    fragmentTrans.add(R.id.dis_ll_fragment, sdtReportFragment);
                } else {
                    fragmentTrans.show(sdtReportFragment);
                }
                break;
            default:
                break;
        }
        fragmentTrans.commit();

    }

    private void hideFragments(FragmentTransaction transaction) {
        if (flowReportFragment != null) {
            transaction.hide(flowReportFragment);
        }
        if (sdtReportFragment != null) {
            transaction.hide(sdtReportFragment);
        }
        if (taskReportFragment != null) {
            transaction.hide(taskReportFragment);
        }
    }
}
