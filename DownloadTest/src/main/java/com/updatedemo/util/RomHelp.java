package com.updatedemo.util;

import android.app.usage.StorageStatsManager;
import android.content.Context;
import android.os.Build;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.util.Log;

import androidx.annotation.RequiresApi;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;
import java.util.Locale;
import java.util.UUID;

/**
 * @author lin
 * @version 2020/7/14
 */
public class RomHelp {

    private Context context;
    public RomHelp(Context context) {
        this.context = context;
    }

    private static final String TAG = "RomHelp";

    public long[] getStorageSize() {
        //5.0 查外置存储

        StorageManager storageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        float unit = 1024;
        long total = 0L, used = 0L, totalSize = 0L, systemSize = 0L;
        int version = Build.VERSION.SDK_INT;
        if (version < Build.VERSION_CODES.M) {//小于6.0
            try {
                Method getVolumeList = StorageManager.class.getDeclaredMethod("getVolumeList");
                StorageVolume[] volumeList = (StorageVolume[]) getVolumeList.invoke(storageManager);
                long availableSize = 0;
                if (volumeList != null) {
                    Method getPathFile = null;
                    for (StorageVolume volume : volumeList) {
                        if (getPathFile == null) {
                            getPathFile = volume.getClass().getDeclaredMethod("getPathFile");
                        }
                        File file = (File) getPathFile.invoke(volume);
                        totalSize += file.getTotalSpace();
                        availableSize += file.getUsableSpace();
                    }
                }
                total += totalSize;
                Log.d(TAG, "totalSize = " + getUnit(totalSize, unit) + " ,availableSize = " + getUnit(availableSize, unit));
            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }
        } else {
            try {
                Method getVolumes = StorageManager.class.getDeclaredMethod("getVolumes");//6.0
                List<Object> getVolumeInfo = (List<Object>) getVolumes.invoke(storageManager);
                for (Object obj : getVolumeInfo) {
                    totalSize = 0L;
                    systemSize = 0L;
                    Field getType = obj.getClass().getField("type");
                    int type = getType.getInt(obj);
                    if (type == 1) {//TYPE_PRIVATE
                        totalSize = 0L;
                        //获取内置内存总大小
                        if (version >= Build.VERSION_CODES.O) {//8.0
                            unit = 1000;
                            Method getFsUuid = obj.getClass().getDeclaredMethod("getFsUuid");
                            String fsUuid = (String) getFsUuid.invoke(obj);
                            totalSize = getTotalSize( fsUuid);//8.0 以后使用
                        } else if (version >= Build.VERSION_CODES.N_MR1) {//7.1.1
                            Method getPrimaryStorageSize = StorageManager.class.getMethod("getPrimaryStorageSize");//5.0 6.0 7.0没有
                            totalSize = (long) getPrimaryStorageSize.invoke(storageManager);
                        }
                        systemSize = 0L;
                        Method isMountedReadable = obj.getClass().getDeclaredMethod("isMountedReadable");
                        boolean readable = (boolean) isMountedReadable.invoke(obj);
                        if (readable) {
                            Method file = obj.getClass().getDeclaredMethod("getPath");
                            File f = (File) file.invoke(obj);
                            if (totalSize == 0) {
                                totalSize = f.getTotalSpace();
                            }
                            systemSize = totalSize - f.getTotalSpace();
                            used += totalSize - f.getFreeSpace();
                            total += totalSize;
                        }
                    } else if (type == 0) {//TYPE_PUBLIC
                        //外置存储
                        Method isMountedReadable = obj.getClass().getDeclaredMethod("isMountedReadable");
                        boolean readable = (boolean) isMountedReadable.invoke(obj);
                        if (readable) {
                            Method file = obj.getClass().getDeclaredMethod("getPath");
                            File f = (File) file.invoke(obj);
                            //used += f.getTotalSpace() - f.getFreeSpace();
                            //total += f.getTotalSpace();
                        }
                    } else if (type == 2) {//TYPE_EMULATED

                    }
                    //Log.d(TAG, "type:" + type + ", total = " + getUnit( total, unit ) +", totalSize = " + getUnit(totalSize, unit) + ", used(with system) =" + getUnit(used, unit) + ", free =" + getUnit(total - used, unit));
                }
                Log.d(TAG, "total =" + getUnit( total, unit ) + " used(with system) =" + getUnit( used, unit ) + " free =" + getUnit(total - used, unit));
            } catch (SecurityException e) {
                Log.e(TAG, "缺少权限：permission.PACKAGE_USAGE_STATS");
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        Log.e(TAG, "getStorageSize: " + used + ",total:" + total );
        return new long[]{used,total};
    }


    /**
     * API 26 android O
     * 获取总共容量大小，包括系统大小
     */
    @RequiresApi(api = Build.VERSION_CODES.O)
    public long getTotalSize(String fsUuid) {
        try {
            UUID id;
            if (fsUuid == null) {
                id = StorageManager.UUID_DEFAULT;
            } else {
                id = UUID.fromString(fsUuid);
            }
            StorageStatsManager stats = context.getSystemService(StorageStatsManager.class);
            return stats.getTotalBytes(id);
        } catch (NoSuchFieldError | NoClassDefFoundError | NullPointerException | IOException e) {
            e.printStackTrace();
            return -1;
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    public long getFreeBytes(String fsUuid) {
        try {
            UUID id;
            if (fsUuid == null) {
                id = StorageManager.UUID_DEFAULT;
            } else {
                id = UUID.fromString(fsUuid);
            }
            StorageStatsManager stats = context.getSystemService(StorageStatsManager.class);
            return stats.getFreeBytes(id);
        } catch (NoSuchFieldError | NoClassDefFoundError | NullPointerException | IOException e) {
            e.printStackTrace();
            return -1;
        }
    }

    public String getUnit(float size, float unit) {
        int index = 0;
        String[] units = {"B", "KB", "MB", "GB", "TB"};
        while (size > unit && index < 4) {
            size = size / unit;
            index++;
        }
        return String.format(Locale.getDefault(), " %.2f %s", size, units[index]);
    }
}
