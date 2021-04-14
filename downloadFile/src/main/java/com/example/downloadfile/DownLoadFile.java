package com.example.downloadfile;

import android.content.Context;

import com.lidroid.xutils.util.LogUtils;
import com.updatedemo.util.ApplicationUtil;
import com.updatedemo.util.BytesUtils;
import com.updatedemo.util.Dump;
import com.updatedemo.util.FileUtil;
import com.updatedemo.util.RomHelp;
import com.updatedemo.util.SerialManager;

import java.io.File;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Arrays;

public class DownLoadFile {

    private SerialManager mSerialManager;
    private RomHelp romHelp;

    public static final byte AndroidModelID = (byte) 0x80;
    public static final byte AndroidHardwareVersionID = (byte) 0x81;
    public static final byte AndroidFirmwareVersionID = (byte) 0x82;
    public static final byte SNID = (byte) 0x83;
    public static final byte PNID = (byte) 0x84;

    public final int TIME_OUT=500;//等待确认帧的超时，单位ms
    public final int RETRY_TIMES = 3; // 重试次数
    /**
     * 每帧数据的最大长度
     */
    public final int DATA_MAXLEN=1024*16;
    /**
     * 每帧的最大长度
     */
    public final int FRAME_MAXLEN=1+1+DATA_MAXLEN+2;

    public final byte SYN = 0x7E; // 起始/结束标志
    public final byte TYPE_ACK = 0x01; // 确认帧类型
    public final byte TYPE_DATAFRAME = 0x02; // 数据帧类型
    public final byte TYPE_NACK = 0x40; // 否认帧类型
    public final byte TYPE_SYN =0x04; // SYN类型

    public final int SYN_LEN = 1; // 起始/结束标志长度
    public final int FRAME_TYPE_LEN = 1; // 帧类型长度
    public final int FRAME_SN_LEN = 1; // 帧序号长度
    public final int DATAFRAME_MAXLEN = DATA_MAXLEN; // 数据帧最大长度
    public final int CRC_LEN = 2; // CRC校验码长度
    // 500ms    32768000
    // 400ms    26214400
    // 100ms    6553600
    // 10ms     655360
    //  1ms     65536
    public final int timeout_500ms=32768000;
    public final int timeout_400ms=26214400;
    public final int timeout_100ms=6553600;
    public final int timeout_10ms=655360;
    public final int timeout_1ms=65536;

//    int download=0;//1:开始下载

    public byte[] mNextByteArray;

    public int mFrameNoFlag = 0x00; // 帧序号

    static final int[] crc16_table = {
            0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
            0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
            0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
            0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
            0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
            0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
            0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
            0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
            0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
            0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
            0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
            0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
            0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
            0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
            0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
            0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
            0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
            0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
            0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
            0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
            0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
            0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
            0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
            0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
            0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
            0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
            0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
            0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
            0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
            0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
            0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
            0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040};
    /**
     * 下载应用和固件
     * @param androidModel 型号
     * @param androidHardwareVersion 硬件识别码
     * @param androidFirmwareVersion 软件版本
     * @param sn SN号    Serial Number，产品序列号
     * @param pn PN号    Production Number，生产编号
     * @param dir 下载路径(不为null，其他参数可为null)
     * @return 0-成功  -3 ota包不匹配  -4/-1失败 -33串口打开失败 1-OTA测试结束
     */
    public int downloadAppAndFirm(String androidModel,
                                  String androidHardwareVersion, String androidFirmwareVersion,
                                  String sn, String pn, byte[] dir,Context context ) throws IOException {
        // 填充 “型号” 的TLV数据包
        byte[] androidModelBuff = new byte[androidModel.getBytes().length + 2];
        androidModelBuff[0] = AndroidModelID;
        androidModelBuff[1] = (byte) (androidModelBuff.length - 2);
        System.arraycopy(androidModel.getBytes(), 0, androidModelBuff, 2,
                androidModelBuff.length - 2);

        // 填充 “硬件识别码” 的TLV数据包
        byte[] androidHardwareVersionBuff = new byte[androidHardwareVersion
                .getBytes().length + 2];
        androidHardwareVersionBuff[0] = AndroidHardwareVersionID;
        androidHardwareVersionBuff[1] = (byte) (androidHardwareVersionBuff.length - 2);
        System.arraycopy(androidHardwareVersion.getBytes(), 0,
                androidHardwareVersionBuff, 2,
                androidHardwareVersionBuff.length - 2);

        // 填充 “软件版本” 的TLV数据包
        byte[] androidFirmwareVersionBuff = new byte[androidFirmwareVersion
                .getBytes().length + 2];
        androidFirmwareVersionBuff[0] = AndroidFirmwareVersionID;
        androidFirmwareVersionBuff[1] = (byte) (androidFirmwareVersionBuff.length - 2);
        System.arraycopy(androidFirmwareVersion.getBytes(), 0,
                androidFirmwareVersionBuff, 2,
                androidFirmwareVersionBuff.length - 2);

        // 填充 “SN” 的TLV数据包
        if(sn == null||"unknown".equals(sn)){
            sn = "null";
        }
        byte[] snBuff = new byte[sn.getBytes().length + 2];
        snBuff[0] = SNID;
        snBuff[1] = (byte) (snBuff.length - 2);
        System.arraycopy(sn.getBytes(), 0, snBuff, 2, snBuff.length - 2);

        // 填充 “PN” 的TLV数据包
        if(pn == null || "unknown".equals(pn)){
            pn = "null";
        }
        byte[] pnBuff = new byte[pn.getBytes().length + 2];
        pnBuff[0] = PNID;
        pnBuff[1] = (byte) (pnBuff.length - 2);
        System.arraycopy(pn.getBytes(), 0, pnBuff, 2, pnBuff.length - 2);

        // 版本信息之前必须带有一个字节的0x00，表示获取版本成功;JAVA转C，需要在字节数组末尾加0X00表示结束
        byte[] infoBuf = new byte[androidModelBuff.length
                + androidHardwareVersionBuff.length
                + androidFirmwareVersionBuff.length + snBuff.length
                + pnBuff.length + 1 + 1];
        int position = 1;
        System.arraycopy(androidModelBuff, 0, infoBuf, position,
                androidModelBuff.length);
        position += androidModelBuff.length;
        System.arraycopy(androidHardwareVersionBuff, 0, infoBuf, position,
                androidHardwareVersionBuff.length);
        position += androidHardwareVersionBuff.length;
        System.arraycopy(androidFirmwareVersionBuff, 0, infoBuf, position,
                androidFirmwareVersionBuff.length);
        position += androidFirmwareVersionBuff.length;
        System.arraycopy(snBuff, 0, infoBuf, position, snBuff.length);
        position += snBuff.length;
        System.arraycopy(pnBuff, 0, infoBuf, position, pnBuff.length);
        position += snBuff.length;

        byte[] dirBuf = new byte[dir.length+1];//c中以00结束
        System.arraycopy(dir, 0, dirBuf, 0, dir.length);
        LogUtils.d(new String(dir));
        LogUtils.d(new String(dirBuf));

        int ret = -1;
        String appInfo = getAppInfo(context);
        //关闭日志
        LogUtils.allowD=false;
        LogUtils.allowE=false;

//        Log.e("logd","LogUtils.allowD "+LogUtils.allowD);
//        Log.e("loge","LogUtils.allowE "+LogUtils.allowE);

        LogUtils.e("上传信息：" + appInfo);
        if (!appInfo.isEmpty()) {
            try {
                byte[] byteAppInfo = appInfo.getBytes("GBK");
                int len = byteAppInfo.length;
                byte[] byteSendData = new byte[len + 1];
                byteSendData[0] = 0x00;
                System.arraycopy(byteAppInfo, 1, byteSendData, 1, len - 1);
                byteSendData[len] = 0x00;
                LogUtils.e("机器信息：" + Dump.getHexDump(infoBuf));

                ret = downFile(infoBuf, infoBuf.length, dir,byteSendData, byteSendData.length,context);


            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }
        } else {
            ret = downFile(infoBuf, infoBuf.length, dir,new byte[] {0x00}, 1,context);

        }
        return ret;
    }

        public int downFile(byte[] posInfo, int len, byte[] dir ,byte[] appInfo, int appInfoLen, Context context) throws IOException {
        int inUpdateFile=1;
        int shakeHandSucess=0;//握手成功与否判断；0：未握手 1：已握手
        int ret=0;
        byte[] arrayRecdata={};
        byte[] arrayPCSend={};
        mSerialManager = new SerialManager(context);
        mSerialManager.initAnalogSerial();
        int rlen;
        int downType = 0;
        int readLen=0;
        while(inUpdateFile==1){
            LogUtils.e(" 起始帧序号 "+mFrameNoFlag);
            if(shakeHandSucess==0){
                LogUtils.e("shake hand");
                byte[] start=new byte[]{0x06};
                sdtp_send1(start);
            }
            /*收到的数据,完整的一帧*/
            arrayPCSend=sdtp_recv(readLen,1);
            LogUtils.d(" arrayPCSend.length == "+arrayPCSend.length);
            if(arrayPCSend==null||arrayPCSend.length==0){
                LogUtils.e(" --- recv null --- ");
                continue;
            }
            /*  getData
                // array: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]
               // data: [ Data ]
            */
            arrayRecdata=getData(arrayPCSend,arrayPCSend.length);
            ret=arrayRecdata.length;
            LogUtils.d("--------- recv data "+byteArrayToHexString(arrayRecdata));


            if(ret>0){
                LogUtils.e("Receive arrayOutput_data: "+byteArrayToHexString(arrayRecdata));
                byte c=arrayRecdata[0];
                if(c==0x00){//获取版本
                    LogUtils.e("获取版本");
                    shakeHandSucess=1;
                    LogUtils.e("posinfo "+byteArrayToHexString(posInfo));
                    LogUtils.d("posinfo "+byteToString(posInfo));
                    sdtp_send(posInfo);
                    readLen=84;
                }
                else if(c==0x02||c==0x07||c==0x05||c==0x10){
                    LogUtils.e("下载文件");
                    shakeHandSucess=1;
                    byte[] rlen_byteArray=byteArrayCut(arrayRecdata,1,4);
                    LogUtils.d("rlen_byteArray "+byteArrayToHexString(rlen_byteArray));
                    rlen_byteArray=byteArrayReverse(rlen_byteArray);
                    LogUtils.d("rlen_byteArray(反转后) "+byteArrayToHexString(rlen_byteArray));
                    rlen=BytesUtils.bytesToInt(rlen_byteArray);
                    LogUtils.e("(文件大小)rlen "+rlen);

                    if(c == 0x02){
                        downType = 0;
                    }else if(c == 0x05){
                        downType = 5;
                    }else if(c == 0x07){
                        downType = 7;
                    }
                    // 获取下载的文件的 文件名

                    byte[] nameBuf=new byte[ret-5-1];

                    System.arraycopy(arrayRecdata,5,nameBuf,0,ret-5-1);
                    LogUtils.e("nameBuf: "+byteToString(nameBuf));
                    LogUtils.e("nameBuf(hex): "+byteArrayToHexString(nameBuf));
                    LogUtils.e("dir: "+byteToString(dir));
                    byte[] ppath=new byte[dir.length+nameBuf.length];
                    ppath=byteArrayMerge(dir,nameBuf);
                    LogUtils.e("ppath: "+byteToString(ppath));
                    //创建文件
                    File file=new File(byteToString(ppath));
                    file.createNewFile();
//                    OutputStream out=null;
//                    try {
//                        out=new FileOutputStream(file);/*实例化OutputStream*/
//                    }catch (FileNotFoundException e){
//                        e.printStackTrace();
//                    }
                    
                    byte[] startTransmission=new byte[]{0x00};
                    sdtp_send(startTransmission);//发送0x00开始传输


                    LogUtils.d("33333333");
                    LogUtils.d("  rlen"+rlen);
                    byte[] receiveFrame_withoutSYN_fromEscape;
                    byte[] wholeFrame_fromEscape;
                    byte[] readWholeFrame;
                    while (rlen!=0){
                        len=rlen>DATA_MAXLEN?DATA_MAXLEN:rlen;
                        LogUtils.e("len "+len);

                        readWholeFrame=sdtp_recv1(len,1);
                        LogUtils.d("读到的完整的一帧-------------------长度 "+readWholeFrame.length);
//                        LogUtils.e("读到的完整的一帧："+byteArrayToHexString(readWholeFrame));
                        LogUtils.d("获取帧中的数据部分\n" +
                                " array: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]\n" +
                                " data:  [ Data ] -----------------------------");
                        //array: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]
                        //receiveFrame_withoutSYN: [ frameType ][ frameNo ][ Data ][ CRC ]
                        byte[] receiveFrame_withoutSYN=getFrame_withoutSYN(readWholeFrame,readWholeFrame.length);
                        //对receiveFrame_withoutSYN进行转义
                        receiveFrame_withoutSYN_fromEscape=fromEscape(receiveFrame_withoutSYN);
                        //把receiveFrame_withoutSYN还原成完整的一帧（加上开头结尾的0x7e）
                        wholeFrame_fromEscape=byteArrayMerge(new byte[]{SYN},receiveFrame_withoutSYN_fromEscape);
                        wholeFrame_fromEscape=byteArrayMerge(wholeFrame_fromEscape,new byte[]{SYN});

                        byte[] receiveData=getData(wholeFrame_fromEscape,wholeFrame_fromEscape.length);
                        LogUtils.e("  [ Data ]的长度 "+receiveData.length);
//                        byte[] receiveData_escape=fromEscape(receiveData);
                        LogUtils.d("读到的帧的数据部分----------------- ");
//                        LogUtils.d("读到的帧的数据部分："+byteArrayToHexString(receiveData));
                        if(receiveData.length!=len){
                            LogUtils.e("len"+len);
                            LogUtils.e("arrayOutput "+byteToString(arrayRecdata));
                            mSerialManager.serialClose();
                            return  -1;
                        }
                        LogUtils.d("44444444");
                        
                        //写入数据
                        FileUtil.writeFileByBytes(file,receiveData,true);
                        LogUtils.d("55555555");

//                        byte[] pbuf=new byte[]{SYN,TYPE_DATAFRAME,(byte)mFrameNoFlag,0x00,SYN};
                        byte[] pbuf=new byte[]{0x00};
                        sdtp_send(pbuf);
                        rlen-=len;
                        LogUtils.e("  剩余数据长度  rlen  "+rlen);
                    }

                    LogUtils.e("rlen==0,end while");
//                    byte[] tempbuf=new byte[]{SYN,TYPE_DATAFRAME,(byte)mFrameNoFlag,0x00,SYN};
                    byte[] tempbuf=new byte[]{0x00};
                    sdtp_send(tempbuf);
                } else if(c==0x03){
                    LogUtils.e("get reboot cmd");
                    inUpdateFile=0;
                    mFrameNoFlag=0;
                    LogUtils.e(" mFrameNoFlag置零 "+mFrameNoFlag);
                    mSerialManager.serialClose();
                    LogUtils.e("downType"+downType);
                    if(downType == 0){
                        return 0;
                    }else if(downType == 5){
                        return 5;
                    }else if(downType == 7){
                        return 7;
                    }
                }else if(c==0x01){//OTA包不匹配
                    LogUtils.e("    OTA包不匹配     ");
                    inUpdateFile=0;
                    mSerialManager.serialClose();
                    return -3;
                }else if(c==0x04){//清除应用
                    LogUtils.e("need clear apks......");
                    inUpdateFile=0;
                    mSerialManager.serialClose();
                    return 4;
                }else if(c==0x11){
                    LogUtils.e("获取应用");
                    sdtp_send(appInfo);
                    inUpdateFile=0;
                    mSerialManager.serialClose();
                    if(appInfo.length==1){
                        return -5;
                    }
                    return 1;
                }else if(c==0x08){//安装应用无法取消
                    inUpdateFile=0;
                    mSerialManager.serialClose();
                    return 8;
                }else if(c==(0x04|0x08)){
                    inUpdateFile=0;
                    mSerialManager.serialClose();
                    return 12;
                }
            }
        }
        if(shakeHandSucess==0){
            mSerialManager.serialClose();
            return -4;
        }
        LogUtils.e("end......");
        mSerialManager.serialClose();
        return 0;
    }


    /**
     *  发送
     * @param data  数据
     * @return
     */

    public int sdtp_send(byte[] data){

        int frameNum=data.length/DATA_MAXLEN+1;//
        int arrayLen=0;
        byte[] arrayReceiveOneByte=new byte[1];
        byte[] arrayReceive=new byte[12];
        int readLen=6;

        byte[] arrayData=new byte[]{};
        int arrayReceiveLen=0;
        int start=0;    //每帧开始的位置
        int end=0;      //每帧结束的位置
        if(data==null||data.length<=0) {
            return 0;
        }
        //发数据，收确认帧
        //分帧发送，每帧数据最大长度为1024*16（DATA_MAXLEN）字节
        for (int i = 0; i <frameNum ; i++) {
            start=DATA_MAXLEN*i;
            end=(i==(frameNum-1)?data.length:DATA_MAXLEN*(i+1));
            int frameNo=mFrameNoFlag;

            for(int retry=0;retry<RETRY_TIMES;retry++){
                //重试3次
                //发送一帧
                sendOneFrame(TYPE_DATAFRAME,frameNo,data);//帧类型（数据帧）
                // 等待ACK确认帧
                byte[] arrayRecv=null;
                int ret=0;
                do{
                    ret=mSerialManager.serialRead(arrayReceive,readLen,0);
                    LogUtils.e("  (sdtp_send)ret  "+ret);
                    if(ret==-1){
                        try {
                            Thread.sleep(10);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        continue;
                    }
                    while (arrayReceive[ret-1]!=0x7e){
                        mSerialManager.serialRead(arrayReceiveOneByte,1,0);
                        arrayReceive=byteArrayMerge(byteArrayCut(arrayReceive,0,ret),arrayReceiveOneByte);
                        ret++;
                        LogUtils.e("  (sdtp_send)ret(while)  "+ret);
                        LogUtils.d("arrayReceive[ret-1] "+arrayReceive[ret-1]);
                    }
                    arrayReceiveLen=ret;
//                    arrayReceiveLen = recv_a_fame(arrayReceive);
                    LogUtils.e( "(sdtp_send)Receive len:" + arrayReceiveLen);
                    arrayRecv=byteArrayCut(arrayReceive,0,arrayReceiveLen);
                    LogUtils.e( "(sdtp_send)Receive frame:" +byteArrayToHexString(arrayRecv));
                }while (ret==-1);

                // 检查确认帧格式，CRC校验 ---0x7E	帧类型 (1)	帧序号 (1)	数据	CRC16校验 (2byte)	0x7E
                //如果在这里收到数据帧，继续等待ACK确认帧{这是收到确认帧，跳出循环，帧序号+1}--确认帧后到解决
                LogUtils.e("stdp_send checkdata");
                if (!checkData(TYPE_ACK, frameNo, arrayRecv, arrayReceiveLen)) {
                    // 数据非法
                    if ((retry == RETRY_TIMES - 1) ) {
                        // 超过重试次数，结束流程
                        return -1;
                    }
                    // 未超过重试次数，重发数据帧
                    LogUtils.e( "(send)retry send data");
                    continue;
                }
                // 数据合法，跳出重发循环，准备发送下一帧
                break;
            }//for循环
            // 帧序号递增
            addFrameNo();//收到确认帧，+1等待数据
        }
        return 1;
    }

    public int sdtp_send1(byte[] data){
        int frameNum=data.length/DATA_MAXLEN+1;//帧数
        byte[] arrayReceive=new byte[7];
        byte[] arrayReceive_1Byte=new byte[1];
        byte[] arrayData=new byte[]{};
        int arrayReceiveLen=0;
        int start=0;    //每帧开始的位置
        int end=0;      //每帧结束的位置
        if(data==null||data.length<=0) {
            return 0;
        }
        //发数据，收确认帧
        //分帧发送，每帧数据最大长度为1024*16（DATA_MAXLEN）字节
        for (int i = 0; i <frameNum ; i++) {
            start=DATA_MAXLEN*i;
            end=(i==(frameNum-1)?data.length:DATA_MAXLEN*(i+1));
            int frameNo=mFrameNoFlag;

            for(int retry=0;retry<RETRY_TIMES;retry++){

                LogUtils.d("----sdtp_send1   retry--------  "+retry);
                LogUtils.e("--------SN--------  "+mFrameNoFlag);
                //重试3次
                //发送一帧
                sendOneFrame(TYPE_DATAFRAME,frameNo,data);//帧类型（数据帧）
                // 等待ACK确认帧
                byte[] arrayRecv=null;
                int ret=0;
//                ret=mSerialManager.serialRead(arrayReceive,10,32768000);
//                LogUtils.e("  ret  "+ret);
                do{
                    ret=mSerialManager.serialRead(arrayReceive,7,32768000);
                    LogUtils.e("  ret  "+ret);
                    LogUtils.e("frame"+byteArrayToHexString(arrayReceive));
                    LogUtils.e("  frame TYPE  "+arrayReceive[1]);
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }while (ret>0&&(arrayReceive[1]!=TYPE_ACK||arrayReceive[1]!=TYPE_NACK));
                if(ret<=0){
                    LogUtils.d(" ------ ret==-1 ------ ");
                    continue;
                }
                arrayReceiveLen = recv_a_fame(arrayReceive);
                LogUtils.e( "(sdtp_send)Receive len:" + arrayReceiveLen);
                arrayRecv=byteArrayCut(arrayReceive,0,arrayReceiveLen);
                LogUtils.e( "(sdtp_send)Receive frame:" +byteArrayToHexString(arrayRecv));

                // 检查确认帧格式，CRC校验 ---0x7E	帧类型 (1)	帧序号 (1)	数据	CRC16校验 (2byte)	0x7E
                //如果在这里收到数据帧，继续等待ACK确认帧{这是收到确认帧，跳出循环，帧序号+1}--确认帧后到解决
                LogUtils.e("send checkdata");
                LogUtils.e("--------SN--------  "+mFrameNoFlag);
                if (!checkData(TYPE_ACK, frameNo, arrayRecv, arrayReceiveLen)) {
                    // 数据非法
                    if ((retry == RETRY_TIMES - 1) ) {
                        // 超过重试次数，结束流程
                        LogUtils.e( "(stdp_send)超过重试次数，结束流程");
                        return -1;
                    }
                    // 未超过重试次数，重发数据帧
                    LogUtils.e( "(stdp_send)retry send data");
                    continue;
                }
                // 数据合法，跳出重发循环，准备发送下一帧
                // 帧序号递增
                addFrameNo();//收到确认帧，+1等待数据
                break;
            }//for循环

        }//end for
        return 1;
    }

    /**
     *
     * @param length   (没用到)
     * @param timeout
     * @return
     */
    public byte[] sdtp_recv(int length,int timeout){
        int arraySize=0;
        int receive_256Byte=256;
        int receive_1k=1024;
        int receive_4k=4*1024;

        byte[] arrayReceive_256Byte=new byte[receive_256Byte];
        byte[] arrayReceive=new byte[receive_256Byte];
        byte[] arrayReceiveReal=new byte[]{};
        int arrayReceiveLen=0;
        int ret=0;
        byte[] arrayTemp = new byte[] {};
        LogUtils.d("------ sdtp_recv ------");
        do{
            for (int retry = 0; retry < RETRY_TIMES; retry++) {
                LogUtils.d("-----retry--- "+retry+" -------");

                int i=0;
                int recvFinish=0;
                int position=0;
                while (recvFinish==0){
                    LogUtils.d("----------- (stdp_recv)开始读数据 -----------");
                    ret=mSerialManager.serialRead(arrayReceive_256Byte,receive_256Byte,0);
                    LogUtils.e("第"+i+"次ret : "+ret);
                    LogUtils.e("第"+i+"次arrayReceive(前10个byte): "+byteArrayToHexString(byteArrayCut(arrayReceive_256Byte,0,10)));
                    i++;

                    if(ret==-1){
                        LogUtils.d("------ while (recvFinish==0) ret== -1 ------ ");
                        try {
                            Thread.sleep(10);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        break;
                    }//end if
                    if (ret>=0&&ret<receive_256Byte){
                        LogUtils.d("----------- (stdp_recv)读完数据 -----------");
                        recvFinish=1;
                    }//end if
                    LogUtils.e("------ position ------ "+position);
                    System.arraycopy(arrayReceive_256Byte,0,arrayReceive,position,ret);
                    position+=ret;

                }//end while
                if (ret==-1){
                    LogUtils.d("------ for循环(retry) ret== -1 ------ ");
                    continue;
                }
//            LogUtils.e( "(sdtp_recv)Read From PC : " + byteArrayToHexString(arrayReceive));
//            LogUtils.e( "(sdtp_recv)Read From PC -------------------------------" );

                // 数据发送完毕，等待接收返回数据帧
                //获取完整的一帧
                arrayReceiveLen = recv_a_fame(arrayReceive);
                LogUtils.e( "收到数组的长度（0x7e到0x7e）arrayReceiveLen: " + arrayReceiveLen);
                //去除多余的0x00
                arrayReceiveReal=byteArrayCut(arrayReceive,0,arrayReceiveLen);
                LogUtils.d( "读到的数据（前10个byte）: " + byteArrayToHexString(byteArrayCut(arrayReceiveReal,0,10)));
//                LogUtils.d( "Receive data: " + byteArrayToHexString(arrayReceive));

                // 检查数据帧格式，CRC校验
                LogUtils.e("对收到的一帧进行 格式检查以及CRC校验 ");
                LogUtils.d("arrayReceiveReal.length "+arrayReceiveReal.length);
                //校验不通过
                if (!checkData(TYPE_DATAFRAME, mFrameNoFlag, arrayReceiveReal, arrayReceiveReal.length)) {
                    // 超过重试次数，结束流程
                    if ((retry == RETRY_TIMES - 1)) {
                        LogUtils.d("超过重试次数，结束流程");
                        return null;
                    }
                    // 未超过重试次数，发送否认帧，重新等待
                    sendOneFrame(TYPE_NACK, mFrameNoFlag, new byte[]{});
                    LogUtils.d( "retry read confirm");
                    continue;
                }
                // 校验通过，跳出重试循环
                break;
            }//for循环，重试三次
            // 发送确认帧
            if (ret==-1){
                LogUtils.e("------- (do while循环) ret == -1 ------ ");
                break;
            }
            if (ret!=-1){
                LogUtils.d("-----发送ACK---------");
                sendOneFrame(TYPE_ACK, mFrameNoFlag, new byte[]{});
                // 帧序号递增
                addFrameNo();
            }
            //如果数据帧中的数据长度为最大长度16384，则表示后续还有数据帧，回到循环开头继续等待接收返回数据帧
        }while (arrayReceiveReal.length>=DATAFRAME_MAXLEN);
        if (ret > 0){
            LogUtils.e( "从PC读到的帧 Frame Read From PC : " + byteArrayToHexString(arrayReceiveReal));
        }

        return arrayReceiveReal;
    }

    /**
     * 收16k一帧数据
     * @param length(没用到)
     * @param timeout(没用到)
     * @return
     */
    public byte[] sdtp_recv1(int length,int timeout){
        int receive_1k=1024;
        int receive_4k=4*1024;
        int arraySize=DATA_MAXLEN*2;

        byte[] arrayReceive_1Byte=new byte[1];
        byte[] arrayReceive_12Byte=new byte[12];
        byte[] arrayReceive_4k=new byte[receive_4k];
        byte[] arrayReceive=new byte[arraySize];
        byte[] arrayReceiveReal=new byte[]{};
        byte[] arrayReceiveRealFrame=new byte[]{};
        int arrayReceiveLen=0;
        int ret=0;

        byte[] arrayTemp = new byte[] {};
        LogUtils.d("------ sdtp_recv1 ------");
        do{
         for (int retry = 0; retry < RETRY_TIMES; retry++) {
             LogUtils.d("-----retry--- "+retry+" -------");

             int i=0;
             int recvFinish=0;
             int position=0;
             if(arrayReceiveLen>=DATA_MAXLEN){
                 LogUtils.d("-----读null数据帧-------");
//                 ret=mSerialManager.serialRead(arrayReceive_12Byte,12,timeout_1ms);  实际耗时30ms

                 ret=mSerialManager.serialRead(arrayReceive_12Byte,6,0);
                 LogUtils.e("  读null数据帧(sdtp_recv1)ret  "+ret);
                 LogUtils.d("读null数据帧arrayReceive_12Byte "+byteArrayToHexString(arrayReceive_12Byte));
                 if(ret==-1){
                     continue;
                 }
                 while (arrayReceive_12Byte[ret-1]!=0x7e){
                     mSerialManager.serialRead(arrayReceive_1Byte,1,0);
                     arrayReceive_12Byte=byteArrayMerge(byteArrayCut(arrayReceive_12Byte,0,ret),arrayReceive_1Byte);
                     ret++;
                     LogUtils.e("  读null数据帧(sdtp_send)ret(while)  "+ret);
                     LogUtils.d("读null数据帧arrayReceive_12Byte "+byteArrayToHexString(arrayReceive_12Byte));
                 }

             }else if(length<=receive_4k){
                    LogUtils.d("-----读4k数据帧-------");
                    ret=mSerialManager.serialRead(arrayReceive_4k,receive_4k,timeout_10ms);
                    }
                    else {
                        LogUtils.d("-----读16k数据帧-------");
                        ret=mSerialManager.serialRead(arrayReceive_4k,receive_4k,timeout_500ms);
                    }

             LogUtils.e("第"+i+"次ret : "+ret);
             if(ret==-1){
                 LogUtils.d("------ ret== -1 ------ ");
                 continue;
             }//end if
             //ret小于4k，表示读完了
             if(ret<receive_4k){
                 recvFinish=1;
             }
             if(arrayReceiveLen>=DATA_MAXLEN){
                 LogUtils.e("第"+i+"次arrayReceive(前10个byte):------null数据帧 "+byteArrayToHexString(byteArrayCut(arrayReceive_12Byte,0,10)));
                 System.arraycopy(arrayReceive_12Byte,0,arrayReceive,position,ret);
             }else{
                 LogUtils.e("第"+i+"次arrayReceive(前10个byte):------16k数据帧 "+byteArrayToHexString(byteArrayCut(arrayReceive_4k,0,10)));
                 System.arraycopy(arrayReceive_4k,0,arrayReceive,position,ret);
             }

             position+=ret;
             i++;
             while (recvFinish==0){
                 LogUtils.d("----------- (stdp_recv1)开始读数据 -----------");
                ret=mSerialManager.serialRead(arrayReceive_4k,receive_4k,0);
                LogUtils.e("第"+i+"次ret : "+ret);
                LogUtils.e("第"+i+"次arrayReceive(前10个byte): "+byteArrayToHexString(byteArrayCut(arrayReceive_4k,0,10)));
                i++;
                if(ret==-1){
                    LogUtils.d("------ ret== -1 ------ ");
                    continue;
                }//end if
                 if (ret>=0&&ret<receive_4k){
                     LogUtils.d("----------- (stdp_recv1)读完数据 -----------");
                     recvFinish=1;
                 }//end if
                 LogUtils.e("------ position ------ "+position);
                 System.arraycopy(arrayReceive_4k,0,arrayReceive,position,ret);
                 position+=ret;
             }//end while


                // 数据发送完毕，等待接收返回数据帧
                //获取完整的一帧
             arrayReceiveLen=position;
//                arrayReceiveLen = recv_a_fame(arrayReceive);
                LogUtils.e( "收到数组的长度（0x7e到0x7e）arrayReceiveLen: " + arrayReceiveLen);
                arrayTemp=fromEscape(byteArrayCut(arrayReceive,0,arrayReceiveLen));
                LogUtils.e("转义之后的长度 "+arrayTemp.length);
                //去除多余的0x00
//                if(arrayReceiveLen>1024*16){
//                    arrayReceiveRealFrame=byteArrayCut(arrayReceive,0,arrayReceiveLen);
//                }
             if(arrayReceiveLen>1024*16||arrayTemp.length>=7){
                 arrayReceiveRealFrame=byteArrayCut(arrayReceive,0,arrayReceiveLen);
             }
             arrayReceiveReal=byteArrayCut(arrayReceive,0,arrayReceiveLen);
            LogUtils.d( "读到的数据（前10个byte）: " + byteArrayToHexString(byteArrayCut(arrayReceiveReal,0,10)));
//                LogUtils.d( "Receive data: " + byteArrayToHexString(arrayReceive));

                // 检查数据帧格式，CRC校验
                LogUtils.e("对收到的一帧进行 格式检查以及CRC校验 ");
             LogUtils.d("arrayReceiveReal.length "+arrayReceiveReal.length);
             //校验不通过
             if (!checkData(TYPE_DATAFRAME, mFrameNoFlag, arrayReceiveReal, arrayReceiveReal.length)) {
                 // 超过重试次数，结束流程
                 if ((retry == RETRY_TIMES - 1)) {
                     LogUtils.d("超过重试次数，结束流程");
                     return null;
                 }
                 // 未超过重试次数，发送否认帧，重新等待
                 sendOneFrame(TYPE_NACK, mFrameNoFlag, new byte[]{});
                 LogUtils.d( "retry read confirm");
                 continue;
                }
             // 校验通过，跳出重试循环
             break;
         }//for循环，重试三次
            // 发送确认帧
            if (ret!=-1){
                LogUtils.d("-----发送ACK---------");
                sendOneFrame(TYPE_ACK, mFrameNoFlag, new byte[]{});
                // 帧序号递增
                addFrameNo();
            }
         //如果数据帧中的数据长度为最大长度16384，则表示后续还有数据帧，回到循环开头继续等待接收返回数据帧
        }while (arrayReceiveReal.length>=DATAFRAME_MAXLEN);
        LogUtils.e( "从PC读到的帧(前20 byte) Frame Read From PC : " + byteArrayToHexString(byteArrayCut(arrayReceiveRealFrame,0,20)));

        return arrayReceiveRealFrame;
    }












    /**
     * 发送一帧       [ Data ]---->[ SYN ][ frameType ][ frameNo ][ Data ][ CRC ][ SYN ]
     * @param frameType 帧类型
     * @param frameNo   帧序号
     * @param data   [ Data ]  数据
     */
    private void sendOneFrame(byte frameType,int frameNo,byte[] data){
        // arrayInput: [ Data ]
        // 构造帧：加入帧类型和帧序号
        // arrayExtData: [ frameType ][ frameNo ][ Data ]
        byte[] arrayExtData = byteArrayMerge(new byte[]{frameType, (byte)frameNo}, data);
        // 计算CRC校验码
        byte[] crcByte = calculateCRC16(arrayExtData);
        // 构造帧：加入CRC校验码
        // arraySend: [ frameType ][ frameNo ][ Data ][ CRC ]
        byte[] arraySend = byteArrayMerge(arrayExtData, crcByte);

        // 转义字符处理0x7d -> 0x7d 0x5d , 0x7e -> 0x7d 0x5e
        LogUtils.d("(sendOneFrame)转义前 arraySend: "+byteArrayToHexString(arraySend));
        arraySend = toEscape(arraySend);
        LogUtils.d("(sendOneFrame)转义后 arraySend: "+byteArrayToHexString(arraySend));

        // 构造帧：加入起始和结束标志 SYN
        // arraySend: [ SYN ][ frameType ][ frameNo ][ Data ][ CRC ][ SYN ]
        arraySend=byteArrayMerge(new byte[]{SYN},arraySend);
        arraySend=byteArrayMerge(arraySend,new byte[]{SYN});


        int ret=0;
        //发送即向端口中写数据
        ret=mSerialManager.serialWrite(arraySend,arraySend.length,0);
        LogUtils.d("Send to PC : "+byteArrayToHexString(arraySend));
        LogUtils.d("ret: "+ret);
    }

    private boolean checkFCS(byte frameType, int frameNo, byte[] data, int dataLen){
        // data: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]

        // data: [ frameType ][ frameNo ][ Data ][ CRC ]
        data = byteArrayCut(data, SYN_LEN, dataLen - SYN_LEN * 2);
        // 处理转义字符
        data = fromEscape(data);
        dataLen = data.length;

        // 计算CRC校验码
        byte[] calcCRC = calculateCRC16(byteArrayCut(data, 0, dataLen - CRC_LEN));
        // 取出数据中CRC校验码
        byte[] dataCRC = byteArrayCut(data, dataLen - CRC_LEN, CRC_LEN);
        // 比较两者校验码是否一致
        if(!Arrays.equals(calcCRC, dataCRC)) {
            LogUtils.e("crc error");
            return false;
        }
        return true;
    }

    /**
     * 检查帧格式，并进行CRC校验(对[ frameType ][ frameNo ][ Data ][ CRC ]转义)
     * @param frameType 帧类型
     * @param frameNo   帧编号（Sequence Number）
     * @param data      完整的一帧           // data: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]
     * @param dataLen   数据长度
     * @return  true    false
     */
    private boolean checkData(byte frameType, int frameNo, byte[] data, int dataLen) {
        // data: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]
        // 检查起始和结束标志
        LogUtils.d("data[0]"+data[0]+"data[dataLen-1]"+data[dataLen-1]);
        if (data[0] != SYN || data[dataLen - 1] != SYN) {
            LogUtils.e( "format error(起始、结束标志不匹配)");
            return false;
        }
        // 处理转义字符
        // data: [ frameType ][ frameNo ][ Data ][ CRC ]
        data = byteArrayCut(data, SYN_LEN, dataLen - SYN_LEN * 2);
        data = fromEscape(data);
        dataLen = data.length;

        // 计算CRC校验码
        byte[] calcCRC = calculateCRC16(byteArrayCut(data, 0, dataLen - CRC_LEN));
        // 取出数据中CRC校验码
        byte[] dataCRC = byteArrayCut(data, dataLen - CRC_LEN, CRC_LEN);
        // 比较两者校验码是否一致
        if(!Arrays.equals(calcCRC, dataCRC)) {
            LogUtils.e("crc error");
            return false;
        }
        // 检查帧类型和帧序号
        if (data[0] != frameType || byteToInt(data[1]) != frameNo) {
            LogUtils.e("frame error(帧类型、帧序号不匹配)");
            LogUtils.e("frame type "+frameType+"  data[0] "+data[0]);
            LogUtils.e("frameNo "+frameNo+"  data[1] "+byteToInt(data[1]));
            return false;
        }
        return true;
    }
    /**
     *   计算CRC校验码
     *   生成多项式： x^16+x^15+x^12+1    二进制表示： (1 << 16) | (1 << 15) | (1 << 12) | 1 = 1 1001 0000 0000 0001  =0x9001
     */
    private byte[] calculateCRC16(byte[] buffer){
        int crc = 0;
        int len=buffer.length;
        if(buffer == null) {return intToByteArrayReverse(crc);}

        for (int i = 0; i <len ; i++) {
            crc=((crc >> 8) ^ crc16_table[(crc ^ buffer[i]) & 0x00FF]);
        }
        LogUtils.e("crc = "+crc);
        LogUtils.e("crc(hex) = "+byteArrayToHexString(intToByteArrayReverse(crc)));
        return intToByteArrayReverse(crc);
    }

    /**
     *  第二个SYN（0x7e）为一帧的结束
     * @param arrayReceive  byte[] 完整的一帧数据
     * @return 7e到7e的数组
     */
    private int recv_a_fame(byte[] arrayReceive){
        byte[] arrayResult = new byte[] {};
        int flag=0;
        for (int i = 0; i <arrayReceive.length ; i++) {
//            LogUtils.e("arrayResult: "+byteArrayToHexString(arrayResult));
            if (arrayReceive[i]==SYN){
                LogUtils.d("find 0x7e at "+i);
                flag++;
            }
            if(flag==2){//第二个7e
                arrayResult = byteArrayCut(arrayReceive,0,i+1);
                break;
            }
        }
//        LogUtils.e("arrayResult: "+byteArrayToHexString(arrayResult));
        LogUtils.d("----------收到完整的一帧数据------------------");
        return arrayResult.length;
    }

    /**
     * 读缓冲区数据直到遇到结束标志
     */
    private long startTime;
    private long endTime;
    private boolean addFlag=true;
    private int readData(byte[] arrayReceive){
        int arrayReceiveLen=0;

//        if(mNextByteArray!=null){
//            System.arraycopy(mNextByteArray,0,arrayReceive,0,mNextByteArray.length);
//            arrayReceiveLen=mNextByteArray.length;
//            mNextByteArray=null;
//            return arrayReceiveLen;
//        }
        byte[] arrayTemp = new byte[FRAME_MAXLEN];
        byte[] arrayResult = new byte[] {};
        startTime = System.currentTimeMillis();
        int len = 0;
        int flag=0;
        while (len == 0) {
            len = arrayReceive.length;
            LogUtils.e( "len = " + len);
            endTime = System.currentTimeMillis();
            if((endTime - startTime) > 500  ) {
                LogUtils.e( "out of time");
                break;
            }
            if(len > 0) {
                //从头开始读数据
//                do {
//                    arrayReceiveLen++;
//                    arrayTemp = byteArrayCut(arrayTemp, 0, arrayReceiveLen);
//                    arrayResult = byteArrayMerge(arrayResult, arrayTemp);
//                } while (arrayTemp[arrayReceiveLen-1] != SYN);
                for (int i = 0; i <FRAME_MAXLEN ; i++) {
                    arrayResult = byteArrayCut(arrayReceive,0,i+1);
                    LogUtils.e("arrayResult: "+byteArrayToHexString(arrayResult));
                    if (arrayReceive[i]==SYN){
                        flag++;
                    }
                    if(flag==2){
                        break;
                    }
                }
            }
        }
//        for (int i = 1; i < arrayResult.length - 1; i++) {
//            if (arrayResult[i] == 0x7e) {
//                LogUtils.e("find 0x7e at " + i);
//                mNextByteArray = byteArrayCut(arrayResult, i + 1, arrayResult.length - i - 1);
//                arrayResult = byteArrayCut(arrayResult, 0, i + 1);
//                break;
//            }
//        }
        System.arraycopy(arrayResult, 0, arrayReceive, 0, arrayResult.length);
        return arrayResult.length;
    }

    /**
     * byte数组反转 0x55 0xfa 0x0a 0x00   ——————> 0x00 0x0a 0xfa 0x55
     * @param byteArray
     * @return 反转后的数组
     */
    public  byte[] byteArrayReverse(byte[] byteArray){
        int len=byteArray.length;
        byte[] byteArrayReverse=new byte[len];
        for (int i = 0; i <len ; i++) {
            byteArrayReverse[i]=byteArray[len-1-i];
        }
        return byteArrayReverse;
    }
    /**
     * 截取byte数组的一部分
     */
    public byte[] byteArrayCut(byte[] in,int start,int len){
        if(start+len>in.length){
            len=in.length-start;
        }
        byte[] out=new byte[len];
        System.arraycopy(in,start,out,0,len);
        return out;
    }
    /**
     * 合并两个byte数组
     */
    public byte[] byteArrayMerge(byte[] byte1, byte[] byte2) {
        int byteLen1 = byte1.length;
        int byteLen2 = byte2.length;
        byte[] out = new byte[byteLen1 + byteLen2];
        System.arraycopy(byte1, 0, out, 0, byteLen1);
        System.arraycopy(byte2, 0, out, byteLen1, byteLen2);
        return out;
    }
    /**
     * 字符转义
     * 0x7E--->0x7D,0x5E
     * 0x7D--->0x7D,0x5D
     */
    public byte[] toEscape(byte[] in){
        int inLen=in.length;
        byte[] out=new byte[inLen*2];
        int outLen=0;
        for (int i = 0; i <inLen ; i++) {
            if(in[i]==0x7E){
                out[outLen]=0x7D;
                out[outLen+1]=0x5E;
                outLen++;
            }
            else if(in[i]==0x7D){
                out[outLen]=0x7D;
                out[outLen+1]=0x5D;
                outLen++;
            }
            else{
                out[outLen]=in[i];
            }
            outLen++;
        }
        out=byteArrayCut(out,0,outLen);
        return out;
    }
    /**
     * 字符转义(反)
     * 0x7d 0x5e -> 0x7e
     * 0x7d 0x5d -> 0x7d
     */
    public byte[] fromEscape(byte[] in) {
        int inLen = in.length;
        int outLen = 0;
        byte[] out = new byte[inLen];
        for (int i = 0; i < inLen; i++) {
            if (in[i] == 0x7d) {
                if (in[i + 1] == 0x5d) {
                    out[outLen] = 0x7d;
                    i++;
                } else if (in[i + 1] == 0x5e) {
                    out[outLen] = 0x7e;
                    i++;
                } else {
                    out[outLen] = in[i];
                }
            } else {
                out[outLen] = in[i];
            }
            outLen++;
        }
        out = byteArrayCut(out, 0, outLen);
        return out;
    }
    /**
     * int --> 4个字节 0x00,0x00,0x00,0x01
     */
    public byte[] intToByteArray(int a) {
        return new byte[] {
//                (byte) ((a >> 24) & 0xFF),
//                (byte) ((a >> 16) & 0xFF),
                (byte) ((a >> 8) & 0xFF),
                (byte) (a & 0xFF)
        };
    }

    /**
     * 反向
     * int - 4个字节 0x00,0x00,0x00,0x01
     */
    private  byte[] intToByteArrayReverse(int a) {
        return new byte[] {
                (byte) (a & 0xFF),
                (byte) ((a >> 8) & 0xFF),
//                (byte) ((a >> 16) & 0xFF),
//                (byte) ((a >> 24) & 0xFF)
        };
    }

    /**
     * byte转int
     * @param byteToInt（byte类型）
     * @return  result（int类型）
     */
    public int byteToInt(byte byteToInt) {
        int mask = 0xFF;
        int result = byteToInt&mask;
        return result;
    }
    /*
     * 帧序号递增，帧序号为255时，归0
     */
    public void addFrameNo() {
        if (mFrameNoFlag == 0xff) {
            mFrameNoFlag = 0;
        } else {
            mFrameNoFlag++;
        }
    }

    /**
     * 获取一帧中除了SYN的部分
     * @param array     array: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]
     * @param arrayLen
     * @return  [ frameType ][ frameNo ][ Data ][ CRC ]
     */
    public byte[] getFrame_withoutSYN(byte[] array, int arrayLen) {
        // array: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]
        int frame_withoutSYNLen = arrayLen - SYN_LEN * 2;
        byte[] frame_withoutSYN;
        frame_withoutSYN = byteArrayCut(array, SYN_LEN, frame_withoutSYNLen);
        LogUtils.e("------------getFrame_withoutSYN---------------------");
        return frame_withoutSYN;
    }

    /**
     * 获取帧中的数据部分
     * // array: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]
     * // data: [ Data ]
     */
    public byte[] getData(byte[] array, int arrayLen) {
        // array: [ startTag ][ frameType ][ frameNo ][ Data ][ CRC ][ endTag ]
        int dataLen = arrayLen - SYN_LEN * 2-FRAME_TYPE_LEN - FRAME_SN_LEN - CRC_LEN;
        byte[] data;
//        // data: [ frameType ][ frameNo ][ Data ][ CRC ]
//        byte[] data = byteArrayCut(array, SYN_LEN, dataLen);
//        data = fromEscape(data);
//        dataLen = data.length - FRAME_TYPE_LEN - FRAME_SN_LEN - CRC_LEN;
        // data: [ Data ]
        data = byteArrayCut(array, SYN_LEN + FRAME_TYPE_LEN + FRAME_SN_LEN, dataLen);
//        LogUtils.e("cmd: " + byteArrayToHexString(data));
        LogUtils.e("cmd----------------------");
        return data;
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

    /**
     * 字节数组转为普通字符串（ASCII对应的字符）
     *
     * @param byteArray
     *            byte[]
     * @return String
     */
    public static String byteToString(byte[] byteArray) {
        String result = "";
        char temp;

        int length = byteArray.length;
        for (int i = 0; i < length; i++) {
            temp = (char) byteArray[i];
            result += temp;
        }
        return result;
    }

    public String getAppInfo(Context context) {
        String appInfo = "";
        byte[] ret = new byte[]{};

        ArrayList<ApplicationUtil.AppInfo> appList = new ArrayList<ApplicationUtil.AppInfo>();
        appList = ApplicationUtil.getAllApplicationInfo(context);
        for (int i = 0; i < appList.size(); i++) {
            System.out.println(appList.get(i).toString());
            if (context.getPackageName().equals(appList.get(i).packageName)) {
                continue;
            }
            appInfo += "|" + appList.get(i).toString();
        }
        return appInfo;
    }

    public String getIsDownload() {
        return isDownload;
    }

    public void setIsDownload(String isDownload) {
        this.isDownload = isDownload;
    }

    private String isDownload = "null";


    private String tomsCipher = "none";
    public String getTomsCipher() {
        return tomsCipher;
    }
    public void setTomsCipher(String tomsCipher) {
        this.tomsCipher = tomsCipher;
    }
}
