package com.newland.download.activity;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Process;
import android.os.SystemClock;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.newland.download.MainActivity;
import com.newland.download.R;
import com.newland.download.bean.OperationLog;
import com.newland.download.bean.User;
import com.newland.download.dao.OperationDao;
import com.newland.download.dao.UserDao;
import com.newland.download.utils.AndroidUtils;
import com.newland.download.utils.Constants;
import com.newland.download.utils.LinuxCmd;
import com.newland.download.utils.LogUtil;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


/**
 * @author LYQ
 */
public class LoginActivity extends BaseActivity {

    ImageView mDeleteUserName;
    EditText et_username;
    EditText et_password;
    Button mLogin;
    Spinner spinner_userno;
    TextView tv_version;
    UserDao userDao;
    OperationDao dao;
    List<User> userList;
    List<String> spinnerContentList;
    ArrayAdapter<String> spinnerAdapter;
    String name;
    int userType;
    String strUserType;
    private User user;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        initViews();
        setListener();
        userDao = new UserDao(this);
        dao = new OperationDao(this);

        userList = userDao.findAllUserNo();
        spinnerContentList = new ArrayList<>();
        try {
            showApkVersion();
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            tv_version.setText("NULL");
        }
        if (!Constants.downloadFile.exists()){
            Constants.downloadFile.mkdirs();
            try {
                Runtime.getRuntime().exec("chmod 777 " + Constants.downloadFile.getAbsolutePath());
            } catch (IOException e) {
                LogUtil.e(" load_addinfo: ");
                e.printStackTrace();
            }
        }
        File iniFile = new File(getFilesDir().getAbsolutePath() + "/Language","Multi_eng.ini");
        if (!iniFile.getParentFile().exists()){
            iniFile.getParentFile().mkdirs();
        }
        try {
            AndroidUtils.initFile(this,iniFile);
            Runtime.getRuntime().exec("chmod 777 " + iniFile.getAbsolutePath());
            Runtime.getRuntime().exec("chmod 777 " + iniFile.getParentFile().getAbsolutePath());
        } catch (IOException e) {
            e.printStackTrace();
        }

        for (int i = 0; i < userList.size(); i++) {
            LogUtil.e("list users " + userList.get(i).toString());
            if (userList.get(i).getUserType().intValue() == 2 ){
                strUserType = getString(R.string.userType_Operator);
            }else {
                strUserType = getString(R.string.userType_Admin);
            }
            spinnerContentList.add(i,strUserType + " " + userList.get(i).getUserNo());
        }
        for (int i = 0; i < spinnerContentList.size() ; i++) {
            LogUtil.e(" spinnerContentList[" + i + "] " + spinnerContentList.get(i));
        }
        spinnerAdapter = new ArrayAdapter<>(this,R.layout.support_simple_spinner_dropdown_item,spinnerContentList);
        spinnerAdapter.setDropDownViewResource(R.layout.support_simple_spinner_dropdown_item);
        spinner_userno.setAdapter(spinnerAdapter);
        spinner_userno.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String[] tempStr = spinnerContentList.get(position).split(" ");
//                LogUtil.e("select position " + position + " | tempStr[0] " + tempStr[0] + " | tempStr[1] " + tempStr[1]);
                name = tempStr[1];
                if (getString(R.string.userType_Admin).equals(tempStr[0])){
                    userType = 1;
                }
                if (getString(R.string.userType_Operator).equals(tempStr[0])){
                    userType = 2;
                }

                LogUtil.e("select position " + position + " | userno(name) " + name + " | userType " + userType);
                user = userDao.findByUserNoAndUserType(name,userType);
                if (user.getIsFirstUse().intValue() == 1){
                    showSetPwdDialog();
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

    }

    private void initViews() {
        mDeleteUserName = findViewById(R.id.delete_username);
        et_username = findViewById(R.id.et_username);
        et_password = findViewById(R.id.et_password);
        mLogin = findViewById(R.id.btn_login);
        spinner_userno = findViewById(R.id.spinner_userno);
        tv_version = findViewById(R.id.tv_version);
    }

    private void setListener() {

        et_password.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
            }
        });

        mLogin.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                login();
            }
        });
    }

    private void login() {
        String password = et_password.getText().toString();

        int type = userDao.checkLogin(name,userType,password);
        LogUtil.e(" type " + type);

        if (type == -1){
            Toast.makeText(this, getString(R.string.login_user_unexit),Toast.LENGTH_SHORT).show();
            return;
        }else if(type == -2){
            Toast.makeText(this, getString(R.string.login_pwd_error),Toast.LENGTH_SHORT).show();
            return;
        }else if(type == -3){
            Toast.makeText(this, getString(R.string.login_user_null),Toast.LENGTH_SHORT).show();
            return;
        }
        Constants.isAdmin = type != 2;

        dao.updateLog(this, OperationLog.TYPE_1,getString(R.string.login_in_succ));

        user.setIsFirstUse(0);
        int result = userDao.updatePwd(user);
        LogUtil.e("updatePwd result " + result);
        //记住密码
//        PreferenceUtil.putBoolean(ConstantUtil.KEY, true);
        Intent intent;
        if (Constants.isAdmin){
            intent = new Intent(LoginActivity.this, SettingsActivity.class);
            startActivity(intent);
        }else {
            intent = new Intent(LoginActivity.this, MainActivity.class);
            startActivity(intent);
        }
        finish();
    }

    private void delete() {
        // 清空用户名以及密码
        et_username.setText("");
        et_password.setText("");
        mDeleteUserName.setVisibility(View.GONE);
        et_username.setFocusable(true);
        et_username.setFocusableInTouchMode(true);
        et_username.requestFocus();
    }

    private void showApkVersion() throws PackageManager.NameNotFoundException {
        PackageManager pm = getPackageManager();
        PackageInfo pi = pm.getPackageInfo(getPackageName(), 0);
        tv_version.setText("V" + pi.versionName);
    }

    private void showSetPwdDialog(){
        final AlertDialog.Builder exitLoginDialog = new AlertDialog.Builder(LoginActivity.this);
        exitLoginDialog.setMessage(getString(R.string.set_pwd_prompt));
        exitLoginDialog.setPositiveButton(getString(R.string.ok), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
                Constants.userToSetPwd = user;
                Intent intent = new Intent(LoginActivity.this,SetPwdActivity.class);
                startActivity(intent);
            }
        });
        exitLoginDialog.setNegativeButton(getString(R.string.dialog_cancel), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                user.setIsFirstUse(0);
                int result = userDao.updateUser(user);
                LogUtil.e("updateIsFirstUse result " + result);
            }
        });
        exitLoginDialog.show();
    }
}
