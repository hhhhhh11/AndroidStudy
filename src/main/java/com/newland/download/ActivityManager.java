package com.newland.download;

import android.app.Activity;

import com.newland.download.utils.LogUtil;

import java.lang.ref.SoftReference;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

public class ActivityManager {

    /**
     * 注意，此处为单例模式，在BaseActivity中，只会返回一个对象。否则，每次被继承的BaseActivity在子Activity被创建的时候，
     * 都会得到一个新的对象。每个新的对象下，又会创建自己的HashMap，效果就是，一个HashMap只存了一个activity，
     * 显然与我们想要的结果不一样。
     * 所以，必须使用单例模式
     */

    private static ActivityManager activityManager;

    public static ActivityManager getActivityManager() {
        if (activityManager == null) {
            activityManager = new ActivityManager();
        }
        return activityManager;
    }

    //此处，可有可无。
    private ActivityManager() {
    }

    /**
     * task map，用于记录activity栈，方便退出程序（这里为了不影响系统回收activity，所以用软引用）
     */
    private final HashMap<String, SoftReference<Activity>> taskMap = new HashMap<String, SoftReference<Activity>>();

    /**
     * 往应用task map加入activity
     */
    public final void putActivity(Activity atv) {
        taskMap.put(atv.toString(), new SoftReference<Activity>(atv));
        LogUtil.i("" + atv);
    }

    /**
     * 往应用task map加入activity
     */
    public final void removeActivity(Activity atv) {
        taskMap.remove(atv.toString());
    }

    /**
     * 清除应用的task栈，如果程序正常运行这会导致应用退回到桌面
     */
    public final void finishAllActivity() {
        for (Iterator<Entry<String, SoftReference<Activity>>> iterator = taskMap
                .entrySet().iterator(); iterator.hasNext(); ) {
            SoftReference<Activity> activityReference = iterator.next()
                    .getValue();
            Activity activity = activityReference.get();
            LogUtil.i("" + activity);
            if (activity != null) {
                activity.finish();
            }
        }
        taskMap.clear();
    }
}
