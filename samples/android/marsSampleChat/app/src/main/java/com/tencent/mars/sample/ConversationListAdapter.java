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

import android.databinding.DataBindingUtil;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.tencent.mars.sample.databinding.ItemConversationBinding;

import java.util.ArrayList;
import java.util.List;


/**
 * Created by zhaoyuan on 16/2/18.
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
