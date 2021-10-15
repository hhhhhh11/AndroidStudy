package com.newland.download.utils;

import android.os.Environment;
import android.util.SparseBooleanArray;

import com.newland.download.bean.User;

import java.io.File;
import java.util.List;

/**
 * @author lin
 * @version 2021/5/24
 */
public class Constants {

    /**
     * 下载目录
     */
    public static final File downloadFile = new File("/Share/NlDownload");
    public static final String PATH_KEY = "path";
    public static final String PATH_NAME_KEY = "name";

    public static String KEY_FILE_PATH = "KEY_FILE_PATH";

    public static File nptDriver = new File("/Share/npt_driver.sh");
    public static File serdev = new File("/Share/serdev.sh");

    public static File fastbootFile = new File("/Share/npt_driver.sh");
    public static File unzipFile = new File("/Share/serdev.sh");


    //文件类型
    public static String FILE_LIST = ".list";

    public static boolean isAdmin = true;

    public static String rootPath = "/Share/NlDownload";
    public static String rootSdPath =  "/storage/sdcard1";
    public static String rootUsbPath =  "/storage/usbotg";
    public static String rootInPath =  "/storage/sdcard0";

    public static String logPath =  "/storage/sdcard0/downloadLog.txt";
    public static String logName =  "downloadLog.txt";


    public static class UserType{

        /** 主管 */
        public final static int DIRECTOR = 0;
        /** 系统管理员 */
        public final static int SYSTEM_NAMAGER = 1;
        /** 一般操作员 */
        public final static int USER = 2;
    }

    public static User userToSetPwd;
    public static final String SYSTEM_DIALOG_REASON_KEY = "reason";
    public static final String SYSTEM_DIALOG_REASON_HOME_KEY = "homekey";

}
