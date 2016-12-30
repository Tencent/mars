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

package com.tencent.mars.sample.core;

import com.google.gson.Gson;
import com.tencent.mars.sample.utils.print.BaseConstants;
import com.tencent.mars.sample.wrapper.remote.PushMessage;
import com.tencent.mars.sdt.SignalDetectResult;
import com.tencent.mars.stn.TaskProfile;
import com.tencent.mars.xlog.Log;

import java.nio.charset.Charset;
import java.util.concurrent.LinkedBlockingDeque;

/**
 * Created by caoshaokun on 16/12/21.
 */
public class StatisticHandler extends BusinessHandler{

    public static String TAG = StatisticHandler.class.getSimpleName();

    public static volatile LinkedBlockingDeque<TaskProfile> taskHistory = new LinkedBlockingDeque<>();

    public static volatile LinkedBlockingDeque<SignalDetectResult> sdtResults = new LinkedBlockingDeque<>();

    public static volatile LinkedBlockingDeque<String> flows = new LinkedBlockingDeque<>();

    public static volatile long wifiRecvFlow = 0;
    public static volatile long wifiSendFlow = 0;
    public static volatile long mobileRecvFlow = 0;
    public static volatile long mobileSendFlow = 0;

    @Override
    public boolean handleRecvMessage(PushMessage pushMessage) {
        try {
            switch (pushMessage.cmdId) {
                case BaseConstants.CGIHISTORY_CMDID:
                {
                    Gson gson = new Gson();
                    TaskProfile profile = gson.fromJson(new String(pushMessage.buffer, Charset.forName("UTF-8")), TaskProfile.class);
                    taskHistory.add(profile);
                }
                return true;
                case BaseConstants.CONNSTATUS_CMDID:
                {

                }
                return true;
                case BaseConstants.FLOW_CMDID:
                {
                    String flowStr = new String(pushMessage.buffer, Charset.forName("UTF-8"));
                    String[] flowsizes = flowStr.split(",");
                    wifiRecvFlow += Integer.valueOf(flowsizes[0]);
                    wifiSendFlow += Integer.valueOf(flowsizes[1]);
                    mobileRecvFlow += Integer.valueOf(flowsizes[2]);
                    mobileSendFlow += Integer.valueOf(flowsizes[3]);
                }
                return true;
                case BaseConstants.SDTRESULT_CMDID:
                {
                    Gson gson = new Gson();
                    SignalDetectResult profile = gson.fromJson(new String(pushMessage.buffer, Charset.forName("UTF-8")), SignalDetectResult.class);
                    sdtResults.add(profile);
                }
                return true;
                default:
                    break;
            }
        }
        catch (Exception e) {
            Log.e(TAG, "handle pushMessage failed, %s", e);
        }

        return false;
    }
}
