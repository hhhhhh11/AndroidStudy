package com.example.downloadtest;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.Button;

import com.lidroid.xutils.util.LogUtils;
import com.updatedemo.util.SerialManager;

import java.io.File;
import java.io.IOException;

public class MainActivity extends AppCompatActivity {
    private DownFile mDownFile;
    /** 下载目录 */
    private static final File PCDownDir = new File(Environment.getExternalStorageDirectory().getAbsolutePath() + "/newland_pcdownload");
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Button btn_download=findViewById(R.id.button);
        Button btnRead=findViewById(R.id.btn_read);

        byte[] posInfo=new byte[]{0x00, (byte) 0x80,0x04,0x4e,0x39,0x30,0x30,
                (byte) 0x81,0x03,0x53,0x41,0x32, (byte) 0x82,0x07,0x56,0x32,0x2e,0x31,0x2e,0x37,0x30, (byte) 0x83,0x0c,
        0x4e,0x37,0x4e,0x4c,0x30,0x30,0x36,0x32,0x33,0x30,0x31,0x37, (byte) 0x84,0x0f,0x4e,0x37,
        0x4e,0x4c,0x30,0x30,0x36,0x32,0x33,0x30,0x31,0x37,0x31,0x32,0x33,0x00};

        String androidModel="N900";
        String androidHardwareVersion="SA2";
        String androidFirmwareVersion="V2.1.70";
        String sn="N7NL00623017";
        String pn="N7NL00623017123";
        byte[] appInfo=new byte[]{0x01};
        byte[] readData=new byte[1024*16];
        mDownFile=new DownFile();
        SerialManager serialManager=new SerialManager(MainActivity.this);
        serialManager.clearSerial();
        btn_download.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                try {
                    /*
                     * 下载应用和固件
                     * @param androidModel 型号
                     * @param androidHardwareVersion 硬件识别码
                     * @param androidFirmwareVersion 软件版本
                     * @param sn SN号
                     * @param pn PN号
                     * @param dir 下载路径(不为null，其他参数可为null)
                     * @return 0-成功  -3 ota包不匹配  -4/-1失败 -33串口打开失败 1-OTA测试结束
                     */
                    mDownFile.downloadAppAndFirm(androidModel,
                            androidHardwareVersion, androidFirmwareVersion,
                            sn, pn,(PCDownDir.getAbsolutePath()+"/").getBytes(),MainActivity.this);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
        btnRead.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                int ret=serialManager.serialRead(readData,1024*16,1);
                LogUtils.e("ret "+ret);
                LogUtils.d("read: "+byteArrayToHexString(readData));

            }
        });
    }




    /**
     *byte[]转成16进制String 每个数组成员之间以空格隔开
     * @param byteArray 输入需要转换的byte数组
     * @return 返回16进制形式 String
     */
    public String byteArrayToHexString(byte[] byteArray) {
        String datalogString="";
        for (int byteIndex = 0; byteIndex <byteArray.length; byteIndex++) {
            String hex= Integer.toHexString(byteArray[byteIndex]);
            if(hex.length()>1){
                datalogString+="0x"+hex.substring(hex.length()-2)+" ";
            }
            else {
                datalogString+="0x0"+hex+" ";
            }
        }
        return datalogString;
    }
}