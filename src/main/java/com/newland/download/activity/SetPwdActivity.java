package com.newland.download.activity;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.newland.download.R;
import com.newland.download.bean.User;
import com.newland.download.dao.UserDao;
import com.newland.download.utils.AndroidUtils;
import com.newland.download.utils.Constants;
import com.newland.download.utils.LogUtil;

import java.util.ArrayList;
import java.util.List;

public class SetPwdActivity extends BaseActivity {
    private Button bt_set;
    private EditText et_set_pwd;
    private EditText et_confirm_pwd;
    private EditText et_userno;
    private UserDao userDao;
    private User updateUser;
    private String strUserType;
    private int ret;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(3);
        setContentView(R.layout.activity_set_pwd);
        et_userno = findViewById(R.id.et_userno);
        et_set_pwd = findViewById(R.id.et_set_pwd);
        et_confirm_pwd = findViewById(R.id.et_confirm_pwd);
        bt_set = findViewById(R.id.bt_set_pwd_set);

        userDao = new UserDao(this);
        updateUser = Constants.userToSetPwd;

        if (updateUser.getUserType().intValue() == 2 ){
            strUserType = getString(R.string.userType_Operator);
        }else {
            strUserType = getString(R.string.userType_Admin);
        }
        et_userno.setText(strUserType + " " + updateUser.getUserNo());

        bt_set.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String setPwd = et_set_pwd.getText().toString().trim();
                String confirmPwd = et_confirm_pwd.getText().toString().trim();

                if (AndroidUtils.isEmpty(setPwd)  || setPwd.length() != 8){
                    Toast.makeText(SetPwdActivity.this, getString(R.string.set_pwd_pwd_length_error), Toast.LENGTH_SHORT).show();
                    return;
                }

                if (AndroidUtils.isEmpty(confirmPwd)  || confirmPwd.length() != 8){
                    Toast.makeText(SetPwdActivity.this, getString(R.string.set_pwd_pwd_length_error), Toast.LENGTH_SHORT).show();
                    return;
                }

                if (!setPwd.equals(confirmPwd)){
                    Toast.makeText(SetPwdActivity.this, getString(R.string.set_pwd_pwd_inconsistent), Toast.LENGTH_SHORT).show();
                    return;
                }

                updateUser.setPassword(setPwd);
                updateUser.setIsFirstUse(0);
                ret = userDao.updatePwd(updateUser);
                LogUtil.e("update ret " + ret);
                if (ret == 1){
                    Toast.makeText(SetPwdActivity.this, getString(R.string.pwd_set_right), Toast.LENGTH_SHORT).show();
                    Intent intent = new Intent(SetPwdActivity.this, LoginActivity.class);
                    intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK |Intent.FLAG_ACTIVITY_NEW_TASK);
                    startActivity(intent);
                    finish();
                }else {
                    Toast.makeText(SetPwdActivity.this, getString(R.string.pwd_set_error), Toast.LENGTH_SHORT).show();
                }
            }
        });

    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK){
            Intent intent = new Intent(this,LoginActivity.class);
            startActivity(intent);
            finish();
        }
        if (keyCode == KeyEvent.KEYCODE_HOME){
            finishAllActivity();
        }
        return super.onKeyDown(keyCode, event);
    }
}