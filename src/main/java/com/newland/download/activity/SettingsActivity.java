package com.newland.download.activity;

import androidx.appcompat.app.AppCompatActivity;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;

import com.newland.download.MainActivity;
import com.newland.download.R;
import com.newland.download.utils.AndroidUtils;
import com.newland.download.utils.Constants;
import com.newland.download.utils.LinuxCmd;
import com.newland.download.utils.LogUtil;

import java.io.File;
import java.io.IOException;

public class SettingsActivity extends BaseActivity implements View.OnClickListener {

    private Button bt_import;
    private Button bt_delete;
    private Button bt_addUser;
    private Button bt_pwd;
    private Button bt_log;
    private Button bt_other;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(3);
        setContentView(R.layout.activity_settings);

        bt_import = findViewById(R.id.bt_import);
        bt_delete = findViewById(R.id.bt_delete_imported_file);
        bt_addUser = findViewById(R.id.bt_add_user);
        bt_pwd = findViewById(R.id.bt_pwd);
        bt_log = findViewById(R.id.bt_log);
        bt_other = findViewById(R.id.bt_other);

        bt_import.setOnClickListener(this);
        bt_delete.setOnClickListener(this);
        bt_addUser.setOnClickListener(this);
        bt_pwd.setOnClickListener(this);
        bt_log.setOnClickListener(this);
        bt_other.setOnClickListener(this);


    }

    @Override
    public void onClick(View view) {
        switch (view.getId()){
            case R.id.bt_import:
                Intent intent = new Intent(this, FileMangerActivity.class);
//                Constants.rootUsbPath = AndroidUtils.getExternalStorage(SettingsActivity.this,AndroidUtils.StorageType.USB)[0].getAbsolutePath();

                String path;
                if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
                    if (new File(Constants.rootUsbPath).canRead()){
                        path = Constants.rootUsbPath;
                    }else if (new File(Constants.rootSdPath).canRead()){
                        path = Constants.rootSdPath;
                    }else {
                        path = Constants.rootInPath;
                    }
                }else {
                    File[] file = AndroidUtils.getExternalStorage(this, AndroidUtils.StorageType.USB);
                    if (file.length < 1){
                        file = AndroidUtils.getExternalStorage(this, AndroidUtils.StorageType.SD);
                        if (file.length <1){
                            path = Environment.getExternalStorageDirectory().getPath();

                        }else {
                            path = file[0].getAbsolutePath();
                        }
                    }else {
                        path = file[0].getAbsolutePath();
                    }
                }
                intent.putExtra(Constants.PATH_NAME_KEY,getString(R.string.file_mk_usb));
                intent.putExtra("import",true);
                intent.putExtra(Constants.PATH_KEY, path);
                startActivity(intent);
                break;
            case R.id.bt_delete_imported_file:
                Intent intentDelete = new Intent(this, FileMangerActivity.class);
                intentDelete.putExtra("delete",true);
                startActivity(intentDelete);
                break;

            case R.id.bt_add_user:
                Intent intentAdd = new Intent(this, AddUserAcitvity.class);
                startActivity(intentAdd);
                break;

            case R.id.bt_pwd:
                Intent intent1 = new Intent(this, PwdActivity.class);
                startActivity(intent1);
                break;

            case R.id.bt_log:
                Intent intent2 = new Intent(this, OperationLogActivity.class);
                startActivity(intent2);
                break;

            case R.id.bt_other:
                break;

            default:
                break;
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK){
            showExitLoginConfirmDialog();
        }
        if (keyCode == KeyEvent.KEYCODE_HOME){
            finishAllActivity();
        }
        return super.onKeyDown(keyCode, event);
    }

    private void showExitLoginConfirmDialog(){
        final AlertDialog.Builder exitLoginDialog = new AlertDialog.Builder(SettingsActivity.this);
        exitLoginDialog.setMessage(getString(R.string.exit_login));
        exitLoginDialog.setPositiveButton(getString(R.string.dialog_confirm), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Intent intent = new Intent(SettingsActivity.this,LoginActivity.class);
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK | Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
                finish();
            }
        });
        exitLoginDialog.setNegativeButton(getString(R.string.dialog_cancel), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        exitLoginDialog.show();
    }

}