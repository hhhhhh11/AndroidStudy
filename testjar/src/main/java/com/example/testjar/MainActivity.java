package com.example.testjar;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.Button;


import com.example.downfile.DownFile;

import java.io.File;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {
    private DownFile mDownFile;
    /** 下载目录 */
    private static final File PCDownDir = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/newland_pcdownload");

    String androidModel="N900";
    String androidHardwareVersion="SA2";
    String androidFirmwareVersion="V2.1.70";
    String sn="N7NL00623017";
    String pn="N7NL00623017123";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mDownFile=new DownFile();
        Button btn_download=(Button)findViewById(R.id.button_download);
        btn_download.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    mDownFile.downloadAppAndFirm(androidModel,
                            androidHardwareVersion, androidFirmwareVersion,
                            sn, pn,(PCDownDir.getAbsolutePath()+"/").getBytes(),MainActivity.this);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
    }
}