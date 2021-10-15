package com.newland.download.activity;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.SpinnerAdapter;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.newland.download.R;
import com.newland.download.bean.User;
import com.newland.download.dao.UserDao;
import com.newland.download.utils.AndroidUtils;
import com.newland.download.utils.Constants;
import com.newland.download.utils.LogUtil;

import java.util.ArrayList;
import java.util.List;

public class AddUserAcitvity extends BaseActivity {
    private Button bt_add;
    private EditText et_set_pwd;
    private EditText et_confirm_pwd;
    private EditText et_userno;
    private UserDao userDao;
    private User addUser;
    private String strUserType;
    private int ret;
    private int userType;
    private Spinner spinner_userType;
    private ArrayAdapter<String> spinnerAdapter;
    private List<String> spinnerList;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(3);
        setContentView(R.layout.activity_add_user);
        spinner_userType = findViewById(R.id.sp_usertype);
        et_userno = findViewById(R.id.et_userno);
        et_set_pwd = findViewById(R.id.et_set_pwd);
        et_confirm_pwd = findViewById(R.id.et_confirm_pwd);
        bt_add = findViewById(R.id.bt_add);
        //动态注册广播
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        //启动广播
        registerReceiver(homeKeyReceiver, intentFilter);
        userDao = new UserDao(this);
        addUser = new User();
        spinnerList = new ArrayList<>();
        spinnerList.add(getString(R.string.userType_Admin));
        spinnerList.add(getString(R.string.userType_Operator));

        spinnerAdapter = new ArrayAdapter<>(this,R.layout.support_simple_spinner_dropdown_item,spinnerList);
        spinnerAdapter.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item);
        spinner_userType.setAdapter(spinnerAdapter);
        spinner_userType.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                userType = position + 1;
                LogUtil.e("select position " + position + " | userType " + userType);
                addUser.setUserType(userType);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        bt_add.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String userno = et_userno.getText().toString().trim();
                String setPwd = et_set_pwd.getText().toString().trim();
                String confirmPwd = et_confirm_pwd.getText().toString().trim();

                if (AndroidUtils.isEmpty(userno)){
                    Toast.makeText(AddUserAcitvity.this, getString(R.string.userno_cannot_be_empty), Toast.LENGTH_SHORT).show();
                }

                if (AndroidUtils.isEmpty(setPwd)  || setPwd.length() != 8){
                    Toast.makeText(AddUserAcitvity.this, getString(R.string.set_pwd_pwd_length_error), Toast.LENGTH_SHORT).show();
                    return;
                }

                if (AndroidUtils.isEmpty(confirmPwd)  || confirmPwd.length() != 8){
                    Toast.makeText(AddUserAcitvity.this, getString(R.string.set_pwd_pwd_length_error), Toast.LENGTH_SHORT).show();
                    return;
                }

                if (!setPwd.equals(confirmPwd)){
                    Toast.makeText(AddUserAcitvity.this, getString(R.string.set_pwd_pwd_inconsistent), Toast.LENGTH_SHORT).show();
                    return;
                }
                addUser.setUserNo(userno);
                addUser.setPassword(setPwd);
                addUser.setIsFirstUse(0);
                if (userDao.findByUserNoAndUserType(userno,userType) != null){
                    Toast.makeText(AddUserAcitvity.this, getString(R.string.setting_add_user_user_exist), Toast.LENGTH_LONG).show();
                }else {
                    ret = userDao.insertUser(addUser);
                    LogUtil.e("insert ret " + ret);
                    if (ret > 0){
                        Toast.makeText(AddUserAcitvity.this, getString(R.string.setting_add_user_success), Toast.LENGTH_LONG).show();
                    }else {
                        Toast.makeText(AddUserAcitvity.this, getString(R.string.setting_add_user_fail), Toast.LENGTH_LONG).show();
                    }
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
            Intent intent = new Intent(AddUserAcitvity.this,SettingsActivity.class);
            startActivity(intent);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
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
                        finishAllActivity();
                    }
                }
            }
        }
    };
}