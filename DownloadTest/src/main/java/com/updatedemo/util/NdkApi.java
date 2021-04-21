package com.updatedemo.util;

import android.util.Log;

import com.lidroid.xutils.util.LogUtils;

public class NdkApi {
    static {
        System.loadLibrary("NDKTestApi");
    }

    public native int JNI_NDK_SysGetConfigInfo(int type,int[] value);
    public native int JNI_NDK_SysGetPosInfo(int type,int[] len,byte[] buf);
    public native int JNI_NDK_AscToHex(byte[] asciiBuf,int len,byte type,byte[] bcdBuf);
    public native int JNI_NDK_HexToAsc(byte[] bcdBuf,int len,byte type,byte[] asciiBuf);


    public int NdkApi_SysGetConfigInfo(int type,int[] value){
        int ret;
        ret = JNI_NDK_SysGetConfigInfo(type,value);
        return ret;
    }
    public int NdkApi_SysGetPosInfo(int type,int[] len,byte[] buf){
        int ret;
        ret = JNI_NDK_SysGetPosInfo(type,len,buf);
        return ret;
    }

    /**
     * 将AscII码的字符串转换成压缩的HEX格式               a,b,c  ->  0xab,0xc0
     * @param asciiBuf          被转换的ASCII字符串
     * @param len               输入数据长度(ASCII字符串的长度)
     * @param type              对齐方式  0－左对齐  1－右对齐
     * @param bcdBuf            转换输出的HEX数据
     * @return
     */
    public int NdkApi_AscToHex(byte[] asciiBuf,int len,byte type,byte[] bcdBuf){
        int ret;
        ret = JNI_NDK_AscToHex(asciiBuf, len, type, bcdBuf);
        return ret;
    }

    /**
     * 将HEX码数据转换成AscII码字符串      0xaa,0x0b,0x0c --> a,a,0,b,0,c
     * @param bcdBuf            被转换的Hex数据
     * @param len               转换数据长度(ASCII字符串的长度)
     * @param type              对齐方式  1－左对齐  0－右对齐
     * @param asciiBuf          转换输出的AscII码数据
     * @return
     */
    public int NdkApi_HexToAsc(byte[] bcdBuf,int len,byte type,byte[] asciiBuf){
        int ret;
        ret = JNI_NDK_HexToAsc(bcdBuf, len, type, asciiBuf);
        return ret;
    }


    public String getSn(){
        int[] len = new int[]{128};
        byte[] buf = new byte[128];
        int type = 3;// sn
        int ret;
        ret = NdkApi_SysGetPosInfo(type,len,buf);
        if (ret == 0) {
            LogUtils.e("{sn len}" + len[0]);
            if (buf[len[0] - 1] == 0) {
                Log.e("", new String(buf, 0, len[0] - 1));
                return new String(buf, 0, len[0] - 1);

            } else {
                Log.e("", new String(buf, 0, len[0]));
                return new String(buf, 0, len[0]);
            }
        }
        return "";
    }

    public String getPn(){
        int[] len = new int[]{128};
        byte[] buf = new byte[128];
        int type = 4;// pn
        int ret;
        ret = NdkApi_SysGetPosInfo(type,len,buf);
        if (ret == 0) {
            LogUtils.e( "{pn len}" + len[0]);
            if (buf[len[0] - 1] == 0) {
                LogUtils.e(new String(buf, 0, len[0] - 1));
                return new String(buf, 0, len[0] - 1);

            } else {
                LogUtils.e(new String(buf, 0, len[0]));
                return new String(buf, 0, len[0]);
            }
        }
        return "";
    }

}
