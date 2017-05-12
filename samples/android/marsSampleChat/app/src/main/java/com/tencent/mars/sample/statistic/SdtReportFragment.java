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

import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.tencent.mars.sample.R;
import com.tencent.mars.sample.core.StatisticHandler;
import com.tencent.mars.sdt.SdtLogic;
import com.tencent.mars.sdt.SignalDetectResult;

import java.util.ArrayList;
import java.util.concurrent.LinkedBlockingDeque;

/**
 * Created by caoshaokun on 16/12/21.
 */
public class SdtReportFragment extends Fragment {

    public static String TAG = SdtReportFragment.class.getSimpleName();

    String[] detectTypes = new String[] {
        "PingCheck", "DnsCheck", "NewDnsCheck", "TcpCheck", "HttpCheck"
    };

    String[] tableTitles = new String[] {
        "detectType", "detail"
    };

    private ListView listView;
    private StatisticsAdapter adapter;

    ArrayList<StatisticsAdapter.TableRow> table = new ArrayList<>();

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_sdt_report, container, false);
        return view;
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        listView = (ListView) getActivity().findViewById(R.id.sdtprofile_listview);

        initData(StatisticHandler.sdtResults);
        adapter = new StatisticsAdapter(this.getActivity(), table);
        listView.setAdapter(adapter);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private void initData(LinkedBlockingDeque<SignalDetectResult> sdtHistory) {
        try {
            StatisticsAdapter.TableCell[] titles = new StatisticsAdapter.TableCell[2];
            int width = getActivity().getWindowManager().getDefaultDisplay().getWidth() - 50;
            titles[0] = new StatisticsAdapter.TableCell(tableTitles[0], 200, TaskReportFragment.LISTHEIGHT, StatisticsAdapter.TableCell.STRING);
            titles[1] = new StatisticsAdapter.TableCell(tableTitles[1], width, TaskReportFragment.LISTHEIGHT, StatisticsAdapter.TableCell.STRING);
            table.add(new StatisticsAdapter.TableRow(titles));

            for (SignalDetectResult profile : sdtHistory) {
                for (SignalDetectResult.ResultDetail detail : profile.details) {
                    StatisticsAdapter.TableCell[] cells = new StatisticsAdapter.TableCell[2];
                    cells[0] = new StatisticsAdapter.TableCell(detectTypes[detail.detectType], 200, TaskReportFragment.LISTHEIGHT, StatisticsAdapter.TableCell.STRING);
                    cells[1] = new StatisticsAdapter.TableCell(width, TaskReportFragment.LISTHEIGHT, StatisticsAdapter.TableCell.STRING);
                    switch (detail.detectType) {
                        case SdtLogic.NetCheckType.kPingCheck:
                            cells[1].value = String.format("ping ip:%s, rtt:%s, lossrate:%s", detail.detectIP, detail.rttStr, detail.pingLossRate);
                            break;
                        case SdtLogic.NetCheckType.kTcpCheck:
                            cells[1].value = String.format("tcp ip:%s, rtt:%d, errcode:%d", detail.detectIP, detail.rtt, detail.errorCode);
                            break;
                        case SdtLogic.NetCheckType.kHttpCheck:
                            cells[1].value = String.format("http ip:%s, status:%d, errcode:%d", detail.detectIP, detail.httpStatusCode, detail.errorCode);
                            break;
                        case SdtLogic.NetCheckType.kDnsCheck:
                            cells[1].value = String.format("dns host:%s, ip1:%s, ip2:%s", detail.dnsDomain, detail.dnsIP1, detail.dnsIP2);
                            break;
                        case SdtLogic.NetCheckType.kNewDnsCheck:
                        default:
                            cells[1].value = "";
                            break;
                    }
                    table.add(new StatisticsAdapter.TableRow(cells));
                }

            }

        } catch (Exception e) {
            //
        }


    }
}
