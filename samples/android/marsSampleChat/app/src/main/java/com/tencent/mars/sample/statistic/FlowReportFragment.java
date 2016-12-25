package com.tencent.mars.sample.statistic;

import android.content.Context;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.tencent.mars.sample.R;
import com.tencent.mars.sample.core.StatisticHandler;
import com.tencent.mars.xlog.Log;

/**
 * Created by shockcao on 16/12/21.
 */
public class FlowReportFragment extends Fragment{

    public static String TAG = FlowReportFragment.class.getSimpleName();

    private TextView mTextView;

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
        View view = inflater.inflate(R.layout.fragment_flowreport, container, false);
        return view;
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mTextView = (TextView)getActivity().findViewById(R.id.flow_textview);
        mTextView.setText(String.format("wifiRecv: %d, wifiSend: %d, mobileRecv: %d, mobileSend: %d",
                StatisticHandler.wifiRecvFlow, StatisticHandler.wifiSendFlow,
                StatisticHandler.mobileRecvFlow, StatisticHandler.mobileSendFlow));

    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}
