package com.newland.download;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Process;
import android.os.SystemClock;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.ForegroundColorSpan;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TextView;

import com.newland.download.activity.BaseActivity;
import com.newland.download.activity.FileMangerActivity;
import com.newland.download.activity.LoginActivity;
import com.newland.download.activity.SetPwdActivity;
import com.newland.download.activity.SettingsActivity;
import com.newland.download.bean.DownType;
import com.newland.download.bean.DownloadFile;
import com.newland.download.bean.OperationLog;
import com.newland.download.dao.OperationDao;
import com.newland.download.ndk.NdkApi;
import com.newland.download.ui.NumberProgressBar;
import com.newland.download.utils.AndroidUtils;
import com.newland.download.utils.Constants;
import com.newland.download.utils.FileAnalysis;
import com.newland.download.utils.LinuxCmd;
import com.newland.download.utils.LogUtil;

import org.apache.commons.io.HexDump;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.TimeUnit;

public class MainActivity extends com.newland.download.activity.BaseActivity implements View.OnClickListener{

    private static final String TAG = "MainActivity";
    private Button bt_download;
    private Button bt_file;
    private Button bt_app;
    private Button bt_cancel;
    private Button bt_set;

    private TextView tv_result;
    private TextView tv_file;

    private CheckBox cb_clear;
    private NumberProgressBar number_pb;
    private FileAnalysis fileAnalysis;
    private DownloadFile downloadFile;
    private NdkApi ndkApi;
    private boolean bClear;
    private byte[] szResult;
    private byte[] iszResult;

    //so能识别的路径
    private String logPath;
    private App app;
    private int nRet = -1;
    private boolean isRunning = false;
    private boolean isCancel = false;
    private int isConnect = -1;
    private int isContinuousDownload = -1;
    private ExecutorService executorService;
    private DownloadRunnable downloadRunnable;
    private UsbManager usbManager;
    OperationDao dao;
    private ProgressDialog waitingDialog;
    private AlertDialog.Builder waitingDownloadEndDialog;
    private AlertDialog  alertDialog;

    private Handler mHandler = new Handler(){
        String sn = null;
        @SuppressLint("HandlerLeak")
        @Override
        public void handleMessage(@NonNull Message msg) {

            super.handleMessage(msg);
            if (msg.what == 1){//进度条
                if (msg.arg1 == 0){
                    String filePath = ((String) msg.obj).toLowerCase();
                    if (filePath.endsWith(".ard") || filePath.endsWith(".nlc") || filePath.endsWith(".zip")){
                        tv_result.setText(getString(R.string.main_result_unzip) + msg.obj);
                    }

                }else if (msg.arg1 == -2) {
                    String[] strings = msg.obj.toString().split("_");
                    sn = new String(strings[3]) ;
                    LogUtil.e("SN [" + sn + "]");
                }else {
                    number_pb.setProgress(msg.arg1);
                    tv_result.setText(getString(R.string.main_result_file) + (CharSequence) msg.obj);
                }
            }else if (msg.what == 0){//下载结果
                Log.e(TAG, "handleMessage result : " + nRet );
                int len = 0;
                for (byte b : szResult) {
                    if (b != 0x00){
                        len++;
                    }
                    else {
                        break;
                    }
                }
                String result = new String(szResult,0,len);
                int sum = 0;
                for (byte elem : szResult){
                    sum += elem;
                }
                if (result.contains("/Share/NlDownload/")){
                    result.replace("/Share/NlDownload/","");
                }
                if (nRet != 0){
                    LogUtil.e(" download fail ");
                    tv_result.setText(getString(R.string.main_download_fail) +result );
                    if (isCancel){
                        enableButtons();
                    }
                }else {
                    LogUtil.e("sum " + sum);
                    if (sum == 0){
                        tv_result.setText("\n" + "null");
                    }else {
                        tv_result.setText("\n" + result );
                    }
                }
                String[] strings = result.split(",");
                if (strings.length > 1){
                    sn = strings[1];
                }
                LogUtil.e("downloadToSN [" + sn + "]");
                if (isContinuousDownload == 0){
                    bt_cancel.setEnabled(true);
                    enableButtons();
                }
                String strRetDownloadOrGetApp = "";
                if (isContinuousDownload == 0){
                    strRetDownloadOrGetApp = (nRet == 0 ?getString(R.string.get_app_succ) : getString(R.string.get_app_fail));
                }
                if (isContinuousDownload == 1){
                    strRetDownloadOrGetApp = (nRet == 0 ?getString(R.string.download_succ) : getString(R.string.download_fail));
                }
                String content = strRetDownloadOrGetApp
                        + "\nSN:" + sn
                        +getString(R.string.download_file) + downloadFile.getDownloadFiles()
                        +getString(R.string.download_result) + result
                        + getString(R.string.download_clear) + downloadFile.isbClear();
                dao.updateLog(MainActivity.this, OperationLog.TYPE_4,content);
            }
        }
    };
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(3);
        setContentView(R.layout.activity_main);
        app = new App();
        dao = new OperationDao(this);
        initView();
        downloadFile = new DownloadFile();
        ndkApi = new NdkApi();
        ndkApi.setHandler(mHandler);

        IntentFilter usbDeviceStateFilter = new IntentFilter();
        usbDeviceStateFilter.addAction(UsbManager.ACTION_USB_DEVICE_ATTACHED);
        usbDeviceStateFilter.addAction(UsbManager.ACTION_USB_DEVICE_DETACHED);
        registerReceiver(mUsbReceiver, usbDeviceStateFilter);
        usbManager = (UsbManager)getSystemService(Context.USB_SERVICE);
        downloadRunnable = new DownloadRunnable();
        intiData();
        waitingDialog = new ProgressDialog(MainActivity.this);
        waitingDialog.setTitle(getString(R.string.mian_tip_unonline));
        waitingDialog.setMessage(getString(R.string.wait_for_device));
        waitingDialog.setCanceledOnTouchOutside(true);
        waitingDialog.setCancelable(true);
        waitingDownloadEndDialog = new AlertDialog.Builder(this);
        waitingDownloadEndDialog.setMessage(getString(R.string.wait_for_download_end));
        waitingDownloadEndDialog.setPositiveButton(getString(R.string.ok), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });

    }

    BroadcastReceiver mUsbReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action))
            {
                UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                if (device != null) {
                    LogUtil.e("有设备拔出");
                    isConnect = 0;
                    ndkApi.Pos_SetUsbStatus((isConnect == 1));
                    LogUtil.e(" isRunning " + isRunning);
                    if (isRunning){
                        alertDialog = waitingDownloadEndDialog.show();
                        tv_result.setText(getString(R.string.wait_for_download_end));
                    }else {
                        SpannableString spannableString = new SpannableString(getString(R.string.main_connect_device));
                        spannableString.setSpan(new ForegroundColorSpan(Color.RED), 0, spannableString.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                        tv_result.setText(spannableString);
                    }
                }
            }
            else if (UsbManager.ACTION_USB_DEVICE_ATTACHED.equals(action)) {
                tv_result.setText("");
                LogUtil.e("有设备插入");
                LogUtil.e(" isRunning " + isRunning);
                if (isRunning){
                    tv_result.setText(getString(R.string.wait_for_download_end));
                }else {
                    if (alertDialog != null){
                        alertDialog.dismiss();
                    }
                }
                isConnect = 1;
                ndkApi.Pos_SetUsbStatus((isConnect == 1));

            }
        }
    };

    private void intiData() {
        fileAnalysis = new FileAnalysis(this);

        bClear = false;
        szResult = new byte[1024];
        iszResult = new byte[1024];
        logPath = getFilesDir().getAbsolutePath();
        System.arraycopy(logPath.getBytes(),0,iszResult,0,logPath.getBytes().length);
        File iniFile = new File(logPath+"/Language","Multi_eng.ini");
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


    }

    private void initView() {
        bt_download = findViewById(R.id.bt_download);
        bt_file = findViewById(R.id.bt_file);
        tv_result = findViewById(R.id.tv_result);
        number_pb = findViewById(R.id.number_pb);
        cb_clear = findViewById(R.id.cb_clear);
        bt_app = findViewById(R.id.bt_app);
        tv_file = findViewById(R.id.tv_file);
        bt_cancel = findViewById(R.id.bt_cancel);
        bt_set = findViewById(R.id.bt_set);

        if (Constants.isAdmin){
            bt_set.setVisibility(View.VISIBLE);
        }
        bt_download.setOnClickListener(this);
        bt_file.setOnClickListener(this);
        bt_app.setOnClickListener(this);
        bt_cancel.setOnClickListener(this);
        bt_set.setOnClickListener(this);
        number_pb.setProgress(0);
    }

    private File acmFile = new File("/dev/ttyACM0");
    private File usbFile = new File("/dev/ttyUSB0");
    private String[] devsAcm = new String[]{"ttyACM0","ttyACM1","ttyACM2","ttyACM3","ttyACM4","ttyACM5"};
    private String[] devsUsb = new String[]{"ttyUSB0","ttyUSB1","ttyUSB2","ttyUSB3","ttyUSB4","ttyUSB5"};
    private File[] acmFiles = new File[]{new File("/dev/ttyACM0"),
            new File("/dev/ttyACM1"), new File("/dev/ttyACM2"),
            new File("/dev/ttyACM3"), new File("/dev/ttyACM4"),
            new File("/dev/ttyACM5")};
    private File[] usbFiles = new File[]{new File("/dev/ttyUSB0"),
            new File("/dev/ttyUSB1"), new File("/dev/ttyUSB2"),
            new File("/dev/ttyUSB3"), new File("/dev/ttyUSB4"),
            new File("/dev/ttyUSB5")};
    /**
     * 初始化驱动
     */
    private void initDev() {
//        if (acmFile.exists()){
//            downloadFile.setSzDev("ttyACM0");
//        }else if (usbFile.exists()) {
//            downloadFile.setSzDev("ttyUSB0");
//        }
        if (acmFiles[0].exists()){
            downloadFile.setSzDev(devsAcm[0]);
        }else if (acmFiles[1].exists()){
            downloadFile.setSzDev(devsAcm[1]);
        }else if (acmFiles[2].exists()){
            downloadFile.setSzDev(devsAcm[2]);
        }else if (acmFiles[3].exists()) {
            downloadFile.setSzDev(devsAcm[3]);
        }else if (acmFiles[4].exists()) {
            downloadFile.setSzDev(devsAcm[4]);
        }else if (acmFiles[5].exists()) {
            downloadFile.setSzDev(devsAcm[5]);
        }
        else if (usbFiles[0].exists()){
            downloadFile.setSzDev(devsUsb[0]);
        }else if (usbFiles[1].exists()){
            downloadFile.setSzDev(devsUsb[1]);
        }else if (usbFiles[2].exists()){
            downloadFile.setSzDev(devsUsb[2]);
        }else if (usbFiles[3].exists()) {
            downloadFile.setSzDev(devsUsb[3]);
        }else if (usbFiles[4].exists()) {
        downloadFile.setSzDev(devsUsb[4]);
        }else if (usbFiles[5].exists()) {
            downloadFile.setSzDev(devsUsb[5]);
        }else {
            downloadFile.setSzDev(null);

        }
        if (downloadFile.getSzDev() == null){
            isConnect = 0;
        }else {
            isConnect = 1;
        }
        LogUtil.e("downloadFile.getSzDev() " + downloadFile.getSzDev() + " | isConnect " + isConnect);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()){
            case R.id.bt_download:
                isContinuousDownload = 1;
                bt_download.setEnabled(false);
                bt_file.setEnabled(false);
                bt_app.setEnabled(false);
//                bt_cancel.setEnabled(true);
                if ("12".equals(downloadFile.getSzDownFile()) ){
                    downloadFile.setCanDownload(false);
                }
                if (downloadFile.getDownloadFiles() == null){
                    tv_file.setText(getString(R.string.main_tip_file_list) );
                    SpannableString spannableString = new SpannableString(getString(R.string.main_tip_choose));
                    spannableString.setSpan(new ForegroundColorSpan(Color.RED), 0, spannableString.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                    tv_result.setText(spannableString);
                    enableButtons();
                    return;
                }
                if (!downloadFile.isCanDownload()){
                    showResult(getString(R.string.main_tip_choose));
                    enableButtons();
                    return;
                }
                if (isRunning){
                    alertDialog = waitingDownloadEndDialog.show();
                    return;
                }
                initDev();
                if (isConnect != 1){
                    LogUtil.e("isConnect != 1, isConnect " + isConnect);
                    SpannableString spannableString1 = new SpannableString(getString(R.string.main_connect_device));
                    spannableString1.setSpan(new ForegroundColorSpan(Color.RED), 0, spannableString1.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                    tv_result.setText(spannableString1);
                }else {
                    tv_result.setText("");
                }


                downloadFile.setbClear(cb_clear.isChecked());
                LogUtil.e("接口信息：" + downloadFile.toString());

                number_pb.setProgress(0);
                isCancel = false;
                executorService = app.getGlobalThreadPool();
                executorService.execute(downloadRunnable);
                break;
            case R.id.bt_file:
                Intent intent = new Intent(this, FileMangerActivity.class);
                intent.putExtra(Constants.PATH_KEY,Constants.rootPath);
                intent.putExtra(Constants.PATH_NAME_KEY,getString(R.string.file_mk_down));
                startActivityForResult(intent,1);

                break;
            case R.id.bt_app:
                isContinuousDownload = 0;
                tv_file.setText(getString(R.string.main_tip_file_list) );
                showDialog();
                break;

            case R.id.bt_cancel:
                isCancel = true;
                isContinuousDownload = -1;
//                bt_cancel.setEnabled(false);
                enableButtons();
                break;
            case R.id.bt_set:
                Intent intent1 = new Intent(this, SettingsActivity.class);
                startActivity(intent1);
                break;

            default:
                break;
        }
    }

    public class DownloadRunnable implements Runnable{
        @Override
        public void run() {
           LogUtil.e("isCancel " + isCancel + " isConnect " + isConnect + " isRunning" + isRunning);
           while (!isCancel && isContinuousDownload == 1){
               if (isContinuousDownload != 1){
                   break;
               }

               if (isConnect == 1 && !isRunning){
                   LogUtil.e(" download ");

                   initDev();
                   if (downloadFile.getSzDev() == null) {
                       LogUtil.e("no devices");
                       SpannableString spannableString = new SpannableString(getString(R.string.main_connect_device));
                       spannableString.setSpan(new ForegroundColorSpan(Color.RED), 0, spannableString.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                       runOnUiThread(()->{
                           tv_result.setText(spannableString);
                       });
                       enableButtons();
                       return;
                   }
                   initDownload();
                   System.arraycopy(logPath.getBytes(), 0, iszResult, 0, logPath.getBytes().length);
                   LogUtil.e(" parameters " + downloadFile.toString() );
                   ndkApi.Pos_SetUsbStatus((isConnect == 1));
                   nRet = ndkApi.Pos_DownLoaderInterface(downloadFile.getnType(),
                           downloadFile.getnPlat(), downloadFile.getSzDev(), downloadFile.getSzDownFile(),
                           downloadFile.getSzAppList(), downloadFile.isbClear(), iszResult, szResult);
                   LogUtil.e("download ret " + nRet);
                   showDownloadResult(nRet);
                   isConnect = -1;
               }

           }

        }
    }

    private void enableButtons(){
        runOnUiThread(()->{
            bt_download.setEnabled(true);
            bt_file.setEnabled(true);
            bt_app.setEnabled(true);
        });
    }

    @Override
    protected void onDestroy() {
        LogUtil.e("ondestroy ");
        super.onDestroy();
//        if (waitingDialog != null){
//            waitingDialog.dismiss();
//        }
        unregisterReceiver(mUsbReceiver);
    };

    private void initDownload() {

        isRunning = true;
        iszResult = new byte[1024*5];
        szResult = new byte[1024*5];
    }

    Message msg;
    private void showDownloadResult(int nRet) {
        isRunning = false;
        if (alertDialog != null){
            alertDialog.dismiss();
        }
        msg = mHandler.obtainMessage();
        msg.what = 0;
        msg.sendToTarget();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 1 && resultCode == 1){
            String szDownFile = data.getStringExtra(Constants.KEY_FILE_PATH);

            downloadFile = fileAnalysis.analysis(szDownFile,downloadFile);
            if (downloadFile.isCanDownload()){
                tv_file.setText(getString(R.string.main_tip_file_list)+ downloadFile.getDownloadFiles());
                showResult("");
            }else {
                tv_file.setText(getString(R.string.main_tip_parse_error) + downloadFile.getError());
            }

        }
    }

    private void showResult(String content){
        tv_result.setText(content);
    }

    private int nlpType = 0;
    private void showDialog() {
        String[] item2 = new String[]{ getString(R.string.main_plat_android),getString(R.string.main_plat_m)};
        AlertDialog.Builder builder2 = new AlertDialog.Builder(this);
        builder2.setTitle(getString(R.string.dialog_warning));
        builder2.setCancelable(false);
        builder2.setSingleChoiceItems(item2, nlpType, new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub
                LogUtil.e("which " + which);
                nlpType = which;
                downloadFile.setnPlat(nlpType);
            }
        });

        builder2.setPositiveButton(getString(R.string.dialog_confirm), new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                downloadFile.setnType(DownType.DOWN_GETAPP.getValue());
//                downloadFile.setnPlat(nlpType);
                downloadFile.setSzDownFile("12");
                downloadFile.setSzAppList("");
                downloadFile.setCanDownload(true);
                bt_download.setEnabled(false);
                bt_cancel.setEnabled(false);
                bt_file.setEnabled(false);
                bt_app.setEnabled(false);
                initDev();
                if (downloadFile.getSzDev() == null){
                    SpannableString spannableString = new SpannableString(getString(R.string.main_connect_device));
                    spannableString.setSpan(new ForegroundColorSpan(Color.RED), 0, spannableString.length(), Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
                    tv_result.setText(spannableString);
                    bt_cancel.setEnabled(true);
                    enableButtons();
                    return;
                }
                if (isRunning){
                    return;
                }
                tv_result.setText("");
                tv_file.setText(getString(R.string.main_xml_get_app) );
                number_pb.setProgress(0);
                app.getGlobalThreadPool().execute(new Runnable() {
                    @Override
                    public void run() {
                        LogUtil.e(" getApp ");
                        LogUtil.e(" downloadFile.getnPlat " + downloadFile.getnPlat());
                        initDownload();
                        downloadFile.setDownloadFiles(null);
                        System.arraycopy(logPath.getBytes(),0,iszResult,0,logPath.getBytes().length);
                        LogUtil.e(" parameters " + downloadFile.toString() );
                        ndkApi.Pos_SetUsbStatus((isConnect == 1));
                        nRet = ndkApi.Pos_DownLoaderInterface(downloadFile.getnType(),
                                downloadFile.getnPlat() ,downloadFile.getSzDev() ,downloadFile.getSzDownFile() ,
                                downloadFile.getSzAppList(),true ,iszResult, szResult);
                        showDownloadResult(nRet);
                    }
                });
            }
        });

        builder2.setNegativeButton(getString(R.string.dialog_cancel), new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO Auto-generated method stub
            }
        });

        AlertDialog dialog2 = builder2.create();
        dialog2.getWindow().addFlags(3);
        dialog2.show();
        dialog2.setOnKeyListener(new DialogInterface.OnKeyListener() {
            @Override
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                switch (keyCode){
                    case KeyEvent.KEYCODE_HOME:
                        dialog2.dismiss();
                        finishAllActivity();
                        return true;
                    case KeyEvent.KEYCODE_BACK:
                        dialog2.dismiss();
                        return true;
                }
                return false;
            }
        });

    }

    private void showNoDeviceDialog(){
        final AlertDialog.Builder noDeviceDialog = new AlertDialog.Builder(MainActivity.this);
        noDeviceDialog.setTitle(getString(R.string.mian_tip_unonline));
        noDeviceDialog.setMessage(getString(R.string.main_connect_device));
        noDeviceDialog.setPositiveButton(getString(R.string.dialog_confirm), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        noDeviceDialog.show();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK){
            showExitLoginConfirmDialog();
        }
        if (keyCode == KeyEvent.KEYCODE_HOME) {
            finishAllActivity();
        }
        return super.onKeyDown(keyCode, event);
    }

    private void showExitLoginConfirmDialog(){
        final AlertDialog.Builder exitDialogBuilder = new AlertDialog.Builder(MainActivity.this);
        exitDialogBuilder.setMessage(getString(R.string.exit_login));
        exitDialogBuilder.setPositiveButton(getString(R.string.dialog_confirm), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
                Intent intent = new Intent(MainActivity.this,LoginActivity.class);
                startActivity(intent);
            }
        });
        exitDialogBuilder.setNegativeButton(getString(R.string.dialog_cancel), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {

            }
        });
        AlertDialog exitDialog = exitDialogBuilder.create();
        exitDialog.getWindow().addFlags(3);
        exitDialog.show();
        exitDialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
            @Override
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                switch (keyCode){
                    case KeyEvent.KEYCODE_HOME:
                        finishAllActivity();
                        return true;
                }
                return false;
            }
        });
    }
}