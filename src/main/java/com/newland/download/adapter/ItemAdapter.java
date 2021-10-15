package com.newland.download.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.cardview.widget.CardView;
import androidx.recyclerview.widget.RecyclerView;

import com.newland.download.R;
import com.newland.download.bean.OperationLog;
import com.newland.download.utils.LogUtil;

import java.util.ArrayList;
import java.util.List;


public class ItemAdapter extends RecyclerView.Adapter<ItemAdapter.DetectViewHolder>{

    private Context mContext;

    public void setDataList(List<OperationLog> dataList) {
        this.dataList = dataList;
    }

    private List<OperationLog> dataList = new ArrayList<>();

    public ItemAdapter(Context mContext, List<OperationLog> dataList) {
        this.mContext = mContext;
        this.dataList = dataList;
    }

    @NonNull
    @Override
    public DetectViewHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        //日志显示界面优化
//        View view = View.inflate(mContext, R.layout.item_card,null);
        View view = LayoutInflater.from(mContext).inflate(R.layout.item_card,viewGroup,false);
        return new DetectViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull DetectViewHolder holder, final int position) {
        if (dataList.size() > 0){
            holder.title.setText(dataList.get(position).toString());
//            LogUtil.e(" log : " + dataList.get(position).toString());
            holder.mLinearLayout.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    if (onClickListener!=null){
                        onClickListener.onClick(position);
                    }
                }
            });
        }
    }

    @Override
    public int getItemCount() {
        return dataList.size();
    }

    public class DetectViewHolder extends RecyclerView.ViewHolder{

        LinearLayout mLinearLayout;
        ImageView imageView;
        TextView title;
        ImageView ic_arrow;
        CardView cardView;

        public DetectViewHolder(View itemView) {
            super(itemView);
            mLinearLayout = itemView.findViewById(R.id.ll_auto_test);
            title = itemView.findViewById(R.id.tv_title);
        }
    }


    public void setOnClickListener(OnClickListenerRec onClickListener) {
        this.onClickListener = onClickListener;
    }
    public OnClickListenerRec onClickListener;

    public interface OnClickListenerRec{
        /**
         * click
         * @param position
         */
        void onClick(int position);
    }

}
