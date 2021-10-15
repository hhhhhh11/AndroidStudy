package com.newland.download.ndk;

import android.os.Handler;
import android.os.Message;

import com.newland.download.utils.LogUtil;

import java.io.File;

/**
 * @author lin
 * @version 2021/5/19
 */
public class NdkApi {
    static {
        System.loadLibrary("NlDownload");
    }

    public native int Pos_DownLoaderInterface(int nType,int nPlat,String szDev,String szDownFile,String szAppList,boolean bClear,byte[] iszResult,byte[] szResult);

//    //调到C层的方法
//    public native void nativeDownload();

    File file;



    /**
     * c层回调上来的方法 下载进度处理
     * @param filePath 下载文甲路径
     * @param progress 进度
     * @return
     */
    private int onProgressCallBack(String filePath, int progress) {
        //自行执行回调后的操作
        LogUtil.e("onProgressCallBack:" + filePath);
        File file = new File(filePath);
        msg = mHandler.obtainMessage();
        msg.what = 1;
        msg.arg1 = progress;
        msg.obj = file.getName();
        msg.sendToTarget();
        return 1;
    }


//    //回调到各个线程
//    public interface OnSubProgressListener {
//        int onProgressChange(int total, int already);
//    }
//
//    //调到C层的方法
//    public static native int nativeDownload(String downloadPath,OnSubProgressListener l);
    Message msg = null;
    public void setHandler(Handler mHandler) {
        this.mHandler = mHandler;
    }

    private Handler mHandler;


    public native int JNI_NDK_GetSN(int[] len,byte[] buf);
    public native void Pos_SetUsbStatus(boolean isConnect);
}
