package com.newland.download.activity;

import android.Manifest;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.app.DatePickerDialog;
import android.app.Instrumentation;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Process;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.newland.download.R;
import com.newland.download.adapter.ItemAdapter;
import com.newland.download.bean.OperationLog;
import com.newland.download.dao.OperationDao;
import com.newland.download.ndk.NdkApi;
import com.newland.download.utils.AndroidUtils;
import com.newland.download.utils.Constants;
import com.newland.download.utils.FileUtil;
import com.newland.download.utils.LogUtil;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;

public class OperationLogActivity extends BaseActivity {

    RecyclerView recyclerView;
    TextView tv_none;
    private List<OperationLog> itemList;
    private List<OperationLog> searchList;
    private ItemAdapter adapter;
    private Button bt_out;
    private Button bt_search;
    private EditText et_search;
    private Button bt_date_start;
    private Button bt_date_end;
    private Button bt_clear_date;
    private OperationDao operationDao;
    private Spinner sp_search;
    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE};
    private File logFile;
    private String path;
    private int mYearStart;
    private int mMonthStart;
    private int mDayStart;
    private int mYearEnd;
    private int mMonthEnd;
    private int mDayEnd;
    private int type;
    private String strStartTime;
    private String strEndTime;
    private NdkApi ndkApi;
    private AlertDialog.Builder dialogBuider;

    @RequiresApi(api = Build.VERSION_CODES.M)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(3);
        setContentView(R.layout.activity_operation_log);

//        mSettingsManager = (SettingsManager) getSystemService(NlContext.SETTINGS_MANAGER_SERVICE);
//        mSettingsManager.setHomeKeyEnabled(false);
        recyclerView = findViewById(R.id.recyclerView);
        tv_none = findViewById(R.id.tv_none);
        operationDao = new OperationDao(this);
        itemList = new ArrayList<>();
        searchList = new ArrayList<>();
        sp_search = (Spinner) findViewById(R.id.spinner1);
        bt_out = (Button) findViewById(R.id.bt_out);
        bt_search = findViewById(R.id.bt_search);
        et_search = findViewById(R.id.et_search);
        bt_date_start = findViewById(R.id.bt_date_start);
        bt_date_end = findViewById(R.id.bt_date_end);
        bt_clear_date = findViewById(R.id.bt_clear_date);

        initData();


        //动态注册广播
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_CLOSE_SYSTEM_DIALOGS);
        //启动广播
        registerReceiver(homeKeyReceiver, intentFilter);


        sp_search.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                LogUtil.e("onItemSelected " + i);
                itemList.clear();
                type = i;
                LogUtil.e( "type "+ type + " | startTime [" + strStartTime + "] | endTime [" + strEndTime + "]");
                itemList = findByTypeAndTime(type,strStartTime,strEndTime);
                LogUtil.e("itemList size " + itemList.size());
                adapter.setDataList(itemList);
                adapter.notifyDataSetChanged();
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });

        sp_search.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                switch (keyCode){
                    case KeyEvent.KEYCODE_HOME:
                        finishAllActivity();
                        return true;
                    case KeyEvent.KEYCODE_BACK:
                        return true;
                }
                return false;
            }
        });

        bt_out.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                int[] len = new int[]{128};
                byte[] buf = new byte[len[0]];
                String strSN = "";
                String strTime;
                int ret = -1;
                long time = System.currentTimeMillis();
                SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyyMMddHHmmss", Locale.CHINA);
                strTime = simpleDateFormat.format(time);

                ret = ndkApi.JNI_NDK_GetSN(len,buf);
                if (ret == 0){
                    LogUtil.e(" bufString " + new String(buf,0,len[0]));
                    if (buf[len[0] - 1] == 0){
                        strSN = new String(buf,0,len[0] - 1);
                    }else {
                        strSN = new String(buf,0,len[0]);
                    }
                }

                LogUtil.e("sn [" + strSN + "] | time [" + strTime + "]");

                String content = "";
                if (itemList.size() > 0){
                    for (int i = 0; i < itemList.size(); i++) {
                        content = content + itemList.get(i).toString() +  "\n";
                    }
//                    LogUtil.d("onClick: " + content );
                    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
                        if (new File(Constants.rootUsbPath).canRead()){
                            path = Constants.rootUsbPath;
                        }else if (new File(Constants.rootSdPath).canRead()){
                            path = Constants.rootSdPath;
                        }else {
                            path = Constants.rootInPath;
                        }
                    }else {
                        File[] file = AndroidUtils.getExternalStorage(OperationLogActivity.this, AndroidUtils.StorageType.USB);
                        if (file.length < 1){
                            file = AndroidUtils.getExternalStorage(OperationLogActivity.this, AndroidUtils.StorageType.SD);
                            if (file.length <1){
                                path = Environment.getExternalStorageDirectory().getPath();

                            }else {
                                path = file[0].getAbsolutePath();
                            }
                        }else {
                            path = file[0].getAbsolutePath();
                        }
                    }
                    LogUtil.e(" path " + path);
                    logFile = new File(path + "/" + strSN + "_" + strTime + "_" + Constants.logName);

                    dialogBuider = new AlertDialog.Builder(OperationLogActivity.this);
                    dialogBuider.setTitle(getString(R.string.log_out));
                    dialogBuider.setMessage(getString(R.string.export_log_to_path) + " " + logFile.getAbsolutePath());
                    String finalContent = content;
                    dialogBuider.setPositiveButton(getString(R.string.ok), new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {

                            LogUtil.e("logFile.exists " + logFile.exists());
                            if (Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP) {
                                if (ContextCompat.checkSelfPermission(OperationLogActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                                    ActivityCompat.requestPermissions(OperationLogActivity.this, PERMISSIONS_STORAGE, REQUEST_EXTERNAL_STORAGE);
                                }else {
                                    if (!logFile.exists()) {
                                        try {
                                            if (logFile.createNewFile()) {
                                                LogUtil.d("create log.txt success");
                                            } else {
                                                LogUtil.d("create log.txt fail");
                                            }
                                        } catch (IOException e) {
                                            e.printStackTrace();
                                        }
                                    }
                                }
                            }

                            FileUtil.writeFileByString(logFile, finalContent,false);
                            LogUtil.e("Export log Path : " + logFile.getAbsolutePath());
                            LogUtil.e("logFile.exists " + logFile.exists());
                            if(logFile.exists()){
                                Toast.makeText(OperationLogActivity.this,"Export log successfully! Path : " + logFile.getAbsolutePath(),Toast.LENGTH_LONG).show();
                            }else {
                                Toast.makeText(OperationLogActivity.this,"Export log fail!",Toast.LENGTH_LONG).show();
                            }

                        }
                    });
                    AlertDialog exportDialog = dialogBuider.create();
                    exportDialog.getWindow().addFlags(3);
                    exportDialog.show();
                    exportDialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
                        @Override
                        public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                            switch (keyCode){
                                case KeyEvent.KEYCODE_HOME:
                                    exportDialog.dismiss();
                                    finishAllActivity();
                                    return true;
                                case KeyEvent.KEYCODE_BACK:
                                    exportDialog.dismiss();
                                    return true;
                            }
                            return false;
                        }
                    });

                }
            }
        });

        bt_search.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                LogUtil.e(" search ");
                searchList.clear();
                String strSearch = et_search.getText().toString().trim();
                for (OperationLog operationLog : itemList){
                    if (operationLog.toString().toLowerCase().contains(strSearch.toLowerCase())){
                        searchList.add(operationLog);
                    }
                }
                adapter.setDataList(searchList);
                adapter.notifyDataSetChanged();
            }
        });

        bt_date_start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                itemList.clear();
                Calendar calendar=Calendar.getInstance();
                mYearStart = calendar.get(Calendar.YEAR);
                mMonthStart = calendar.get(Calendar.MONTH);
                mDayStart = calendar.get(Calendar.DAY_OF_MONTH);
                DatePickerDialog startDateDialog = new DatePickerDialog(OperationLogActivity.this, new DatePickerDialog.OnDateSetListener() {
                    @Override
                    public void onDateSet(DatePicker view, int year, int month, int dayOfMonth) {
                        mYearStart = year;
                        mMonthStart = month + 1;
                        mDayStart = dayOfMonth;
                        String startDate;
                        startDate = dateIntToString(mYearStart,mMonthStart,mDayStart);
                        bt_date_start.setText(startDate);
                        strStartTime = startDate + " 00:00:00";
                        LogUtil.e( "type "+ type + " | startTime [" + strStartTime + "] | endTime [" + strEndTime + "]");

                        itemList = findByTypeAndTime(type,strStartTime,strEndTime);
                        LogUtil.e("itemList size " + itemList.size());
                        adapter.setDataList(itemList);
                        adapter.notifyDataSetChanged();
                    }
                },mYearStart,mMonthStart,mDayStart);
                startDateDialog.getWindow().addFlags(3);
                startDateDialog.show();
                startDateDialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        switch (keyCode){
                            case KeyEvent.KEYCODE_HOME:
                                startDateDialog.dismiss();
                                finishAllActivity();
                                return true;
                            case KeyEvent.KEYCODE_BACK:
                                startDateDialog.dismiss();
                                return true;
                        }
                        return false;
                    }
                });
            }
        });

        bt_date_end.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                itemList.clear();
                Calendar calendar=Calendar.getInstance();
                mYearEnd = calendar.get(Calendar.YEAR);
                mMonthEnd = calendar.get(Calendar.MONTH);
                mDayEnd = calendar.get(Calendar.DAY_OF_MONTH);
                DatePickerDialog endDateDialog = new DatePickerDialog(OperationLogActivity.this, new DatePickerDialog.OnDateSetListener() {
                    @Override
                    public void onDateSet(DatePicker view, int year, int month, int dayOfMonth) {
                        mYearEnd = year;
                        mMonthEnd = month + 1;
                        mDayEnd = dayOfMonth;
                        String endDate;
                        endDate = dateIntToString(mYearEnd,mMonthEnd,mDayEnd);
                        bt_date_end.setText(endDate);
                        strEndTime = endDate + " 23:59:59";
                        LogUtil.e( "type "+ type + " | startTime [" + strStartTime + "] | endTime [" + strEndTime + "]");

                        itemList = findByTypeAndTime(type,strStartTime,strEndTime);
                        LogUtil.e("itemList size " + itemList.size());
                        adapter.setDataList(itemList);
                        adapter.notifyDataSetChanged();
                    }
                },mYearEnd,mMonthEnd,mDayEnd);
                endDateDialog.getWindow().addFlags(3);
                endDateDialog.show();
                endDateDialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
                    @Override
                    public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                        switch (keyCode){
                            case KeyEvent.KEYCODE_HOME:
                                endDateDialog.dismiss();
                                finishAllActivity();
                                return true;
                            case KeyEvent.KEYCODE_BACK:
                                endDateDialog.dismiss();
                                return true;
                        }
                        return false;
                    }
                });
            }
        });

        bt_clear_date.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                bt_date_start.setText(getString(R.string.log_start_date));
                bt_date_end.setText(getString(R.string.log_end_date));
                strStartTime = null;
                strEndTime = null;
                LogUtil.e( "type "+ type + " | startTime [" + strStartTime + "] | endTime [" + strEndTime + "]");
                itemList = findByTypeAndTime(type,strStartTime,strEndTime);
                LogUtil.e("itemList size " + itemList.size());
                adapter.setDataList(itemList);
                adapter.notifyDataSetChanged();
            }
        });

    }

    private String dateIntToString(int year,int month,int day){
        String date;
        if (month < 10 && day < 10){
            date = year + "-0" + month + "-0" + day;
        }else if (month < 10 && day >= 10){
            date = year + "-0" + month + "-" + day;
        }else if (month > 10 && day < 10){
            date = year + "-" + month + "-0" + day;
        } else {
            date = year + "-" + month + "-" + day;
        }
        return date;
    }

    private List<OperationLog> findByTypeAndTime(int type,String startTime,String endTime){
        List<OperationLog> tempList = new ArrayList<>();
        List<OperationLog> resultList = new ArrayList<>();
        if (type == 0){
            tempList = operationDao.findAllLogOrder();
        }else {
            tempList = operationDao.findByType(type);
        }
        LogUtil.e("tempList size " + tempList.size());
        if (startTime == null && endTime == null){
            return tempList;
        }else {
            if (startTime == null){
                for (OperationLog operationLog : tempList){
                    String time = operationLog.getTime();
                    if (time.compareTo(endTime) <= 0){
                        resultList.add(operationLog);
                    }
                }
            }else if (endTime == null){
                for (OperationLog operationLog : tempList){
                    String time = operationLog.getTime();
                    if (time.compareTo(startTime) >= 0){
                        resultList.add(operationLog);
                    }
                }
            }else {
                for (OperationLog operationLog : tempList){
                    String time = operationLog.getTime();
                    if (time.compareTo(startTime) >= 0 && time.compareTo(endTime) <= 0){
                        resultList.add(operationLog);
                    }
                }
            }
            LogUtil.e("resultList size " + resultList.size());
            return resultList;
        }
    }

    private void initData() {
        ndkApi = new NdkApi();
        itemList = operationDao.findAllLog();
        if (itemList.size() > 0){
//            for (int i = 0; i < itemList.size(); i++) {
//                LogUtil.e("onCreate: " + itemList.get(i).toString() );
//            }
            recyclerView.setVisibility(View.VISIBLE);
            tv_none.setVisibility(View.GONE);
        }else {
            recyclerView.setVisibility(View.GONE);
            tv_none.setVisibility(View.VISIBLE);
        }
        initRecyclerView();
    }

    @Override
    protected void onStart() {
        super.onStart();

    }

    @Override
    protected void onDestroy() {
        unregisterReceiver(homeKeyReceiver);
        super.onDestroy();
    }

    private BroadcastReceiver homeKeyReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String strAction = intent.getAction();
            if (Intent.ACTION_CLOSE_SYSTEM_DIALOGS.equals(strAction)){
                String strReason = intent.getStringExtra(Constants.SYSTEM_DIALOG_REASON_KEY);
                if (strReason != null){
                    if (strReason.equals(Constants.SYSTEM_DIALOG_REASON_HOME_KEY)){
                        hideSpinnerDropDown(sp_search);
                        finishAllActivity();
//                        ComponentName componetName = new ComponentName("com.android.launcher3",
//                                "com.android.launcher3.Launcher");
//                        try {
//                            Intent intent1 = new Intent();
//                            intent1.setComponent(componetName);
//                            intent1.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
//                            startActivity(intent1);
//                        } catch (Exception e) {
//                            e.printStackTrace();
//                        }

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

    private void initRecyclerView() {
        recyclerView.setHasFixedSize(false);
        recyclerView.setNestedScrollingEnabled(false);
        LinearLayoutManager manager = new LinearLayoutManager(this,LinearLayoutManager.VERTICAL,false);
        recyclerView.setLayoutManager(manager);

        adapter = new ItemAdapter(this,itemList);
        recyclerView.setAdapter(adapter);

        adapter.setOnClickListener(new ItemAdapter.OnClickListenerRec() {
            @Override
            public void onClick(int position) {

            }
        });
    }

    @Override
    public void onBackPressed() {
        finish();
        super.onBackPressed();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_HOME){
            finishAllActivity();
            LogUtil.e("exit");
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_EXTERNAL_STORAGE) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                for (int i = 0; i < permissions.length; i++) {
                    LogUtil.d("申请的权限为：" + permissions[i] + ",申请结果：" + grantResults[i]);
                }
                try {
                    if (!logFile.exists()) {
                        if (logFile.createNewFile()) {
                            LogUtil.d("request create log.txt success");
                        } else {
                            LogUtil.d("request create log.txt fail");
                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}