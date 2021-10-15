package com.newland.download.adapter.base;

import android.view.View;

import androidx.recyclerview.widget.RecyclerView;


public abstract class RecyclerViewHolder<T> extends RecyclerView.ViewHolder {

   public RecyclerViewHolder(View itemView) {
        super(itemView);
    }

    public abstract void onBindViewHolder ( T t , RecyclerViewAdapter adapter , int position) ;
}
