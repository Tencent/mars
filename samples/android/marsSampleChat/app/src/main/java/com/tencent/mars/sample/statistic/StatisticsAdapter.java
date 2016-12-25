package com.tencent.mars.sample.statistic;

import java.util.List;
import android.content.Context;
import android.graphics.Color;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

/**
 * Created by shockcao on 16/12/21.
 */
public class StatisticsAdapter extends BaseAdapter {

    private Context context;
    private List<TableRow> table;

    public StatisticsAdapter(Context context, List<TableRow> table) {
        this.context = context;
        this.table = table;
    }

    @Override
    public int getCount() {
        return table.size();
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    public TableRow getItem(int position) {
        return table.get(position);
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        TableRow tableRow = table.get(position);
        return new TableRowView(this.context, tableRow);
    }

    class TableRowView extends LinearLayout {
        public TableRowView(Context context, TableRow tableRow) {
            super(context);

            this.setOrientation(LinearLayout.HORIZONTAL);
            for (int i = 0; i < tableRow.getSize(); i++) {
                TableCell tableCell = tableRow.getCellValue(i);
                LinearLayout.LayoutParams layoutParams = new LinearLayout.LayoutParams(
                        tableCell.width, tableCell.height);
                layoutParams.setMargins(0, 0, 1, 1);
                if (tableCell.type == TableCell.STRING) {
                    TextView textCell = new TextView(context);
                    textCell.setLines(1);
                    textCell.setGravity(Gravity.CENTER);
                    textCell.setText(String.valueOf(tableCell.value));
                    addView(textCell, layoutParams);
                } else if (tableCell.type == TableCell.IMAGE) {
                    ImageView imgCell = new ImageView(context);
                    imgCell.setImageResource((Integer) tableCell.value);
                    addView(imgCell, layoutParams);
                }
            }
            this.setBackgroundColor(Color.WHITE);
        }
    }

    static public class TableRow {
        private TableCell[] cell;
        public TableRow(TableCell[] cell) {
            this.cell = cell;
        }
        public int getSize() {
            return cell.length;
        }
        public TableCell getCellValue(int index) {
            if (index >= cell.length)
                return null;
            return cell[index];
        }
    }


    static public class TableCell {
        static public final int STRING = 0;
        static public final int IMAGE = 1;
        public Object value;
        public int width;
        public int height;
        private int type;
        public TableCell(int width, int height, int type) {
            this.width = width;
            this.height = height;
            this.type = type;
        }
        public TableCell(Object value, int width, int height, int type) {
            this.value = value;
            this.width = width;
            this.height = height;
            this.type = type;
        }
    }
}