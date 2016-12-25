package com.tencent.mars.sample;

import android.databinding.DataBindingUtil;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.tencent.mars.sample.databinding.ItemConversationBinding;

import java.util.ArrayList;
import java.util.List;


/**
 * Created by kirozhao on 16/2/18.
 */
public class ConversationListAdapter extends RecyclerView.Adapter<ConversationListAdapter.BindingHolder> implements View.OnClickListener {

    public List<Conversation> list = new ArrayList<>(20);

    private OnItemClickListener onItemClickListener = null;

    @Override
    public BindingHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        ItemConversationBinding binding = DataBindingUtil.inflate(
                LayoutInflater.from(parent.getContext()),
                R.layout.item_conversation,
                parent,
                false);
        BindingHolder holder = new BindingHolder(binding.getRoot());
        holder.setBinding(binding);

        holder.binding.cvItem.setOnClickListener(this);

        return holder;
    }

    @Override
    public void onBindViewHolder(BindingHolder holder, int position) {
        Conversation conversation = list.get(position);
        holder.binding.setVariable(com.tencent.mars.sample.BR.conversation, conversation);
        holder.binding.executePendingBindings();

        holder.binding.cvItem.setTag(R.id.tag_first, position);
        holder.binding.cvItem.setTag(R.id.tag_second, conversation);
    }

    @Override
    public int getItemCount() {
        return list.size();
    }

    @Override
    public void onClick(View view) {
        if (onItemClickListener == null) {
            return;
        }

        view.getTag();
        onItemClickListener.onItemClick((Conversation) view.getTag(R.id.tag_second), (int) view.getTag(R.id.tag_first));
    }

    public void setOnItemClickListener(OnItemClickListener onItemClickListener) {
        this.onItemClickListener = onItemClickListener;
    }

    public interface OnItemClickListener {
        void onItemClick(Conversation conversation, int pos);
    }

    public class BindingHolder extends RecyclerView.ViewHolder {
        private ItemConversationBinding binding;

        public BindingHolder(View v) {
            super(v);
        }

        public ItemConversationBinding getBinding() {
            return binding;
        }

        public void setBinding(ItemConversationBinding binding) {
            this.binding = binding;
        }
    }

    public void addConversation(Conversation conversation) {
        list.add(conversation);
        notifyItemInserted(list.size() - 1);
    }

    public void clearAllConversations() {
        list.clear();
        notifyDataSetChanged();
    }

}
