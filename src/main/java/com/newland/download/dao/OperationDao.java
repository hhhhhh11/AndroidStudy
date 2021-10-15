package com.newland.download.dao;

import android.content.Context;

import com.newland.download.R;
import com.newland.download.bean.OperationLog;
import com.newland.download.bean.User;
import com.newland.download.utils.AndroidUtils;
import com.newland.download.utils.Constants;
import com.newland.download.utils.LogUtil;

import java.util.List;

/**
 * @author lin
 * @version 2021/1/29
 */
public class OperationDao extends BaseDao<OperationLog>{


    public OperationDao(Context context) {
        super(context);
    }


    public int deleteById(String userNo) {
        return super.delete("ID", userNo);
    }


//    public User findByUserNoAndUserType(String userNo, int userType) {
//        List<User> l = super.query("user_type=? and user_no = ?", new String[] {String.valueOf(userType), userNo}, null);
//        return l.size() > 0 ? l.get(0) : null;
//    }

    public List<OperationLog> findByType(int type) {
        return super.query("TYPE=?", new String[] {String.valueOf(type)}, "TIME DESC");
    }


    public long insertLog(OperationLog log){
        return super.insert(log);
    }

    public List<OperationLog> findAllLog(){
        return super.findAll();
    }

    public List<OperationLog> findAllLogOrder(){
        return super.findAll("TIME DESC");
    }
    public int deleteAll() {
        return super.delete("", new String[]{});
    }

    public void updateLog(Context context, int type, String content){
        OperationLog log = new OperationLog();
        log.setType(type);
        log.setContent(content);
        log.setTime(AndroidUtils.getDate());
        log.setUserNo(Constants.isAdmin ? context.getString(R.string.log_admin): context.getString(R.string.log_operator));
        log.setTime(AndroidUtils.getDate());
        insertLog(log);
    }

    public void updateLog(boolean isAdmin,Context context, int type, String content){
        OperationLog log = new OperationLog();
        log.setType(type);
        log.setContent(content);
        log.setTime(AndroidUtils.getDate());
        log.setUserNo(isAdmin ? context.getString(R.string.log_admin): context.getString(R.string.log_operator));
        log.setTime(AndroidUtils.getDate());
        insertLog(log);
    }
}
