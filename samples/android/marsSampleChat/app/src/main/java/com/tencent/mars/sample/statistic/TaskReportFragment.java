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
import com.tencent.mars.stn.TaskProfile;
import com.tencent.mars.xlog.Log;

import java.util.ArrayList;
import java.util.concurrent.LinkedBlockingDeque;

/**
 * Created by shockcao on 16/12/21.
 */
public class TaskReportFragment extends Fragment{

    public static String TAG = TaskReportFragment.class.getSimpleName();

    String[] tableTitles = new String[] {
            "task", "success", "cost", "trycount", "channel"
    };

    private ListView mListView;
    private StatisticsAdapter adapter;

    public static final int LISTHEIGHT = 100;

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
        View view = inflater.inflate(R.layout.fragment_taskreport, container, false);
        return view;
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mListView = (ListView)getActivity().findViewById(R.id.taskprofile_listview);

        initData(StatisticHandler.taskHistory);
        adapter = new StatisticsAdapter(this.getActivity(), table);
        mListView.setAdapter(adapter);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private void initData(LinkedBlockingDeque<TaskProfile> taskHistory) {
        StatisticsAdapter.TableCell[] titles = new StatisticsAdapter.TableCell[5];
        int width = getActivity().getWindowManager().getDefaultDisplay().getWidth()/titles.length;
        for (int i = 0; i < titles.length; i++) {
            titles[i] = new StatisticsAdapter.TableCell(tableTitles[i], width, LISTHEIGHT, StatisticsAdapter.TableCell.STRING);
        }
        table.add(new StatisticsAdapter.TableRow(titles));

        for (TaskProfile profile : taskHistory) {
            StatisticsAdapter.TableCell[] cells = new StatisticsAdapter.TableCell[5];
            cells[0] = new StatisticsAdapter.TableCell(String.valueOf(profile.cgi.substring(profile.cgi.lastIndexOf("/")+1)), width, LISTHEIGHT, StatisticsAdapter.TableCell.STRING);
            cells[1] = new StatisticsAdapter.TableCell((profile.errCode == 0 && profile.errType == 0) ? "true " : "false", width, LISTHEIGHT, StatisticsAdapter.TableCell.STRING);
            cells[2] = new StatisticsAdapter.TableCell((profile.endTaskTime - profile.startTaskTime) + "ms", width, LISTHEIGHT, StatisticsAdapter.TableCell.STRING);
            cells[3] = new StatisticsAdapter.TableCell(String.valueOf(profile.historyNetLinkers.length), width, LISTHEIGHT, StatisticsAdapter.TableCell.STRING);
            cells[4] = new StatisticsAdapter.TableCell((profile.channelSelect == 1 ? "shortlink " : "longlink "), width, LISTHEIGHT, StatisticsAdapter.TableCell.STRING);

            table.add(new StatisticsAdapter.TableRow(cells));
        }

    }

}
