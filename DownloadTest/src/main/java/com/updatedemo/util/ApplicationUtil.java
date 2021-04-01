package com.updatedemo.util;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;

import java.util.ArrayList;
import java.util.List;

public class ApplicationUtil {


	public static ArrayList<AppInfo> getAllApplicationInfo2(Context context) {
		ArrayList<AppInfo> appList = new ArrayList<AppInfo>(); // 用来存储获取的应用信息数据
		Intent intent = new Intent(Intent.ACTION_MAIN, null);
		intent.addCategory(Intent.CATEGORY_LAUNCHER);
		List<ResolveInfo> apps = context.getPackageManager().queryIntentActivities(intent, 0);
		for (int i = 0; i < apps.size(); i++) {
			ResolveInfo info = apps.get(i);
			AppInfo tmpInfo = new AppInfo();
			tmpInfo.appName =  (String) info.activityInfo.loadLabel(context.getPackageManager()).toString();
			tmpInfo.packageName = info.activityInfo.packageName;
			try{
				tmpInfo.versionName = context.getPackageManager().getPackageInfo(tmpInfo.packageName,0).versionName;
			}catch (PackageManager.NameNotFoundException e){
				e.printStackTrace();
			}
			appList.add(tmpInfo);

		}
		return appList;
	}


	/**
	 * 获取非系统应用
	 * @param context
	 * @return
	 */
	public static ArrayList<AppInfo> getAllApplicationInfo(Context context) {
		ArrayList<AppInfo> appList = new ArrayList<AppInfo>(); // 用来存储获取的应用信息数据
	    List<PackageInfo> packages = context.getPackageManager().getInstalledPackages(0);
	    
	    for (int i = 0; i < packages.size(); i++) {  
	    	PackageInfo packageInfo = packages.get(i);
	    	if ((packageInfo.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
	    		AppInfo tmpInfo = new AppInfo();
	 	        tmpInfo.appName = packageInfo.applicationInfo.loadLabel(context.getPackageManager()).toString();
	 	        tmpInfo.versionName = packageInfo.versionName;
	 	        tmpInfo.type = "0";
	 	        tmpInfo.packageName = packageInfo.packageName;
	 	        appList.add(tmpInfo);
	    	}
	    }
	    return appList;
	}
	
	public static String getPackageNameByAppName(Context context, String appName) {
		ArrayList<AppInfo> appList = getAllApplicationInfo(context);
		for (int i = 0; i < appList.size(); i++) {
			if (appName.equals(appList.get(i).appName)) {
				return appList.get(i).packageName;
			}
		}
		return "";
	}
	
	public static class AppInfo {
	    public String appName ;     // 应用名
	    public String versionName ; // 版本名
	    public String type;         // 应用类型
	    public String packageName;  // 包名

	    @Override
	    public String toString() {
			return appName + "*" + versionName + "*" + "0" + "*" + packageName;//加包名
	    }
	}
}


