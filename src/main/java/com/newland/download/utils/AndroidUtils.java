package com.newland.download.utils;

import android.content.Context;
import android.os.Build;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/**
 * @author lin
 * @version 2021/5/24
 */
public class AndroidUtils {

    public static void initFile(Context mContext, File file) throws IOException
    {
        if (!file.exists())
        {
            InputStream inputStream = mContext.getAssets().open(file.getName());
            FileOutputStream outputStream = new FileOutputStream(file);
            byte[] bytes = new byte[1024];
            int len ;
            while ((len = inputStream.read(bytes)) != -1)
            {
                outputStream.write(bytes,0,len);
            }
            inputStream.close();
            outputStream.close();
        }
    }

    /**
     * 获取当前时间，时区中国
     * @return
     */
    public static String getDate() {
        long time = System.currentTimeMillis();
        SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.CHINA);
        String date = simpleDateFormat.format(time);
        return date;
    }

    public static boolean isEmpty(String str){
        if (str==null || "".equals(str)){
            return true;
        }
        return false;
    }

    /**
     * 获取外置存储路径
     * @param context 环境上下文
     * @param type 外置存储类型 SD卡/U盘
     * @return 外置存储路径数组(一个或多个)
     * 直接写外置存储需要添加权限android.permission.WRITE_MEDIA_STORAGE
     * A7及以上平台该权限不对第三方应用开放，所以应用需要为系统预置应用(即在system/priv-app目录下)或者具有system uid签名
     */
    public static File[] getExternalStorage(Context context, StorageType type) {
        StorageManager storageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        ArrayList<File> files = new ArrayList<>();
        try {
            if (Build.VERSION.SDK_INT < Build.VERSION_CODES.M) {
                //6.0以下检测方法 获取对象是StorageVolume
                Method getVolumeList = StorageManager.class.getDeclaredMethod("getVolumeList");
                StorageVolume[] volumeList = (StorageVolume[]) getVolumeList.invoke(storageManager);
                if (volumeList != null) {
                    for (StorageVolume volume : volumeList) {
                        Method method = volume.getClass().getDeclaredMethod("isRemovable");
                        boolean isRemovable = (boolean) method.invoke(volume);
                        if (isRemovable) {
                            method = volume.getClass().getDeclaredMethod("getPathFile");
                            File file = (File) method.invoke(volume);
                            String path = file.getPath();
                            //根据路径名称判断是否USB或SD卡
                            if (type == StorageType.SD && path.contains("sdcard")) {
                                files.add(file);
                            } else if (type == StorageType.USB && path.contains("usb")) {
                                files.add(file);
                            }
                        }
                    }
                }
            } else {
                //6.0及以上的检测方法 获取对象是VolumeInfo
                Method getVolumes = StorageManager.class.getDeclaredMethod("getVolumes");
                List<Object> getVolumeInfo = (List<Object>) getVolumes.invoke(storageManager);
                //获取对象是VolumeInfo
                for (Object obj : getVolumeInfo) {
                    Field getType = obj.getClass().getField("type");
                    //存储类型
                    int storageType = getType.getInt(obj);
                    //外置存储 TYPE_PUBLIC
                    if (storageType == 0) {
                        Method method = obj.getClass().getDeclaredMethod("getInternalPath");
                        File file = (File) method.invoke(obj);
                        if (file==null){
                            continue;
                        }
                        method = obj.getClass().getDeclaredMethod("getDisk");
                        Object diskInfo = method.invoke(obj);
                        if (type == StorageType.USB) {
                            //通过反射接口判断是否为USB
                            method = diskInfo.getClass().getDeclaredMethod("isUsb");
                            boolean isUsb = (boolean) method.invoke(diskInfo);
                            if (isUsb) {
                                files.add(file);
                            }
                        } else if (type == StorageType.SD) {
                            //通过反射接口判断是否为SD卡
                            method = diskInfo.getClass().getDeclaredMethod("isSd");
                            boolean isSdCard = (boolean) method.invoke(diskInfo);
                            if (isSdCard) {
                                files.add(file);
                            }
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return files.toArray(new File[0]);
    }

    public enum StorageType {

        // USB
        USB,
        // SD卡
        SD;
    }
}
