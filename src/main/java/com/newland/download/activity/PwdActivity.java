package com.newland.download.activity;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Message;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.newland.download.R;
import com.newland.download.bean.User;
import com.newland.download.dao.UserDao;
import com.newland.download.utils.AndroidUtils;
import com.newland.download.utils.Constants;
import com.newland.download.utils.LogUtil;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

public class PwdActivity extends BaseActivity{

    private Button bt_set;
    private EditText ed_user;
    private EditText ed_old;
    private EditText ed_new;
    private Spinner spinner_modifyPwd_userno;
    private UserDao dao;
    private String user;
    String strUserType;

    private List<User> userList;
    private List<String> spinnerList;
    private ArrayAdapter<String> spinnerAdapter;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(3);
        setContentView(R.layout.activity_password);
        dao = new UserDao(this);
        bt_set = findViewById(R.id.bt_set);

        ed_user = findViewById(R.id.ed_user);
        ed_old = findViewById(R.id.ed_old);
        ed_new = findViewById(R.id.ed_new);
        spinner_modifyPwd_userno = findViewById(R.id.spinner_modifypwd_userno);
        userList = dao.findAllUserNo();
        spinnerList = new ArrayList<>();
        //动态注册广播
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        //启动广播
        registerReceiver(homeKeyReceiver, intentFilter);

        for (int i = 0; i < userList.size(); i++) {
            LogUtil.e("list users " + userList.get(i).toString());
            if (userList.get(i).getUserType().intValue() == 2) {
                strUserType = getString(R.string.userType_Operator);
            } else {
                strUserType = getString(R.string.userType_Admin);
            }
            spinnerList.add(i, strUserType + " " + userList.get(i).getUserNo());
        }
        spinnerAdapter = new ArrayAdapter<>(this, R.layout.support_simple_spinner_dropdown_item, spinnerList);
        spinnerAdapter.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item);
        spinner_modifyPwd_userno.setAdapter(spinnerAdapter);
        spinner_modifyPwd_userno.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String[] tempStr = spinnerList.get(position).split(" ");
                LogUtil.e("select position " + position + " | tempStr[0] " + tempStr[0] + " | tempStr[1] " + tempStr[1]);
                user = tempStr[1];
                LogUtil.e("select position " + position + " | userno(name) " + user);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });
        spinner_modifyPwd_userno.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                return false;
            }
        });

        bt_set.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
//                String user = ed_user.getText().toString().trim();
                String oldPwd = ed_old.getText().toString().trim();
                String newPwd = ed_new.getText().toString().trim();
                if (AndroidUtils.isEmpty(user)) {
                    Toast.makeText(PwdActivity.this, getString(R.string.pwd_user_error), Toast.LENGTH_SHORT).show();
                    return;
                }
                if (!user.equals("00") && !user.equals("01") && !user.equals("02") && !user.equals("99")) {
                    Toast.makeText(PwdActivity.this, getString(R.string.pwd_user_error), Toast.LENGTH_SHORT).show();
                    return;
                }
                if (AndroidUtils.isEmpty(oldPwd) || oldPwd.length() != 8) {
                    Toast.makeText(PwdActivity.this, getString(R.string.pwd_old_error), Toast.LENGTH_SHORT).show();
                    return;
                }
                //密码判断
                if (!dao.findByUserNo(user).getPassword().equals(oldPwd)) {
                    Toast.makeText(PwdActivity.this, getString(R.string.pwd_old_error1), Toast.LENGTH_SHORT).show();
                    return;
                }

                if (AndroidUtils.isEmpty(newPwd) || newPwd.length() != 8) {
                    Toast.makeText(PwdActivity.this, getString(R.string.pwd_new_error), Toast.LENGTH_SHORT).show();
                    return;
                }

                User updateUser = dao.findByUserNo(user);
                updateUser.setPassword(newPwd);
                updateUser.setIsFirstUse(0);
                int result = dao.updatePwd(updateUser);
                LogUtil.e("更新数据库：" + result);
                if (result == 1) {
                    Toast.makeText(PwdActivity.this, getString(R.string.pwd_set_right), Toast.LENGTH_SHORT).show();
                    Intent intent = new Intent(PwdActivity.this, LoginActivity.class);
                    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
                    startActivity(intent);
                } else {
                    Toast.makeText(PwdActivity.this, getString(R.string.pwd_set_error), Toast.LENGTH_SHORT).show();
                }

            }
        });
    }

    @Override
    protected void onDestroy() {

        unregisterReceiver(homeKeyReceiver);
        super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK){
            Intent intent = new Intent(this,SettingsActivity.class);
            startActivity(intent);
            finish();
        }
        if (keyCode == KeyEvent.KEYCODE_HOME){
            finishAllActivity();
        }
        return super.onKeyDown(keyCode, event);
    }

    private BroadcastReceiver homeKeyReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String strAction = intent.getAction();
            if (Intent.ACTION_CLOSE_SYSTEM_DIALOGS.equals(strAction)){
                String strReason = intent.getStringExtra(Constants.SYSTEM_DIALOG_REASON_KEY);
                if (strReason != null){
                    if (strReason.equals(Constants.SYSTEM_DIALOG_REASON_HOME_KEY)){
                        hideSpinnerDropDown(spinner_modifyPwd_userno);
                        finishAllActivity();
                    }
                }
            }
        }
    };

    public static void hideSpinnerDropDown(Spinner spinner) {
        try {
            Method method = Spinner.class.getDeclaredMethod("onDetachedFromWindow");
            method.setAccessible(true);
            method.invoke(spinner);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}