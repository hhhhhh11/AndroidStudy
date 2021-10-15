package com.newland.download.dao;

import android.content.Context;
import android.database.Cursor;

import com.newland.download.R;
import com.newland.download.bean.OperationLog;
import com.newland.download.bean.User;
import com.newland.download.utils.AndroidUtils;
import com.newland.download.utils.Constants;

import java.util.List;

/**
 * @author lin
 * @version 2021/1/29
 */
public class UserDao extends BaseDao<User>{

    OperationDao dao;
    public UserDao(Context context) {
        super(context);
        dao = new OperationDao(context);
    }


    public int deleteByUserNo(String userNo) {
        return super.delete("user_no", userNo);
    }


    public User findByUserNoAndUserType(String userNo, int userType) {
        List<User> l = super.query("user_type=? and user_no = ?", new String[] {String.valueOf(userType), userNo}, null);
        return l.size() > 0 ? l.get(0) : null;
    }
    public User findByUserNo(String userNo) {
        List<User> l = super.query("user_no = ?", new String[] {userNo}, null);
        return l.size() > 0 ? l.get(0) : null;
    }

    public List<User> findByUserType(int userType) {
        return super.query("user_type=?", new String[] {String.valueOf(userType)}, null);
    }

    public List<User> findAllUserNo(){
        return super.findAll();
    }

    public int updatePwd(User user){
        return super.update(user);
    }

    public int updateUser(User user){
        return super.update(user);
    }

    public int insertUser(User user){
        return (int) super.insert(user);
    }

    public int deleteAll() {
        return super.delete("", new String[]{});
    }

    public int checkLogin(String userNo,int userType, String password) {
        if (AndroidUtils.isEmpty(userNo) || AndroidUtils.isEmpty(password)) {
            return -3;
        }
        User user = null;
        user = this.findByUserNoAndUserType(userNo,userType);
        if (user == null) {//用户名不存在
            return -1;
        } else if (!user.getPassword().equals(password)) {
            //记录管理员或操作密码登录成功或错误日志
            boolean isAdmin = userType == 2 ? false : true;
            dao.updateLog(isAdmin,context,1,context.getString(R.string.login_in_fail));
            return -2;
        }

        return user.getUserType();
    }
}
