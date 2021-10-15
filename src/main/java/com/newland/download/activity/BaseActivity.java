package com.newland.download.activity;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

import com.newland.download.ActivityManager;

public class BaseActivity extends AppCompatActivity {

    private ActivityManager manager = ActivityManager.getActivityManager();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        manager.putActivity(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        manager.removeActivity(this);
    }

    public void finishAllActivity() {
        manager.finishAllActivity();
    }

}
