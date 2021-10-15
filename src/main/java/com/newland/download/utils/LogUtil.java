package com.newland.download.utils;

import android.util.Log;

/**
 * 自定义日志类 DEBUG true 调试模式
 * 
 * @author maxw
 * @date 2014-1-6
 *
 */
public class LogUtil {

	public static boolean DEBUG = true;

	public static void v(String msg) {
		if (DEBUG) {
			String tag = getTagString();
			Log.v(tag, msg);
		}
	}

	public static void v(String msg, Throwable tr) {
		if (DEBUG) {
			String tag = getTagString();
			Log.v(tag, msg, tr);
		}
	}

	public static void d(String msg) {
		if (DEBUG) {
			String tag = getTagString();
			Log.d(tag, msg);
		}
	}

	public static void d(String msg, boolean debug) {
		if (debug) {
			String tag = getTagString();
			Log.d(tag, msg);
		}
	}

	public static void d(String msg, Throwable tr) {
		if (DEBUG) {
			String tag = getTagString();
			Log.d(tag, msg, tr);
		}
	}

	public static void i(String msg) {
		if (DEBUG) {
			String tag = getTagString();
			Log.i(tag, msg);
		}
	}

	public static void i(String msg, Throwable tr) {
		if (DEBUG) {
			String tag = getTagString();
			Log.i(tag, msg, tr);
		}
	}

	public static void w(String msg) {
		if (DEBUG) {
			String tag = getTagString();
			Log.w(tag, msg);
		}
	}

	public static void w(String msg, Throwable tr) {
		if (DEBUG) {
			String tag = getTagString();
			Log.w(tag, msg, tr);
		}
	}

	public static void e(String msg) {
		if (DEBUG) {
			String tag = getTagString();
			Log.e(tag, msg);
		}
	}

	public static void e(String msg, boolean debug) {
		if (debug) {
			String tag = getTagString();
			Log.e(tag, msg);
		}
	}

	public static void e(String msg, Throwable tr) {
		if (DEBUG) {
			String tag = getTagString();
			Log.e(tag, msg, tr);
		}
	}

	public static String getTagString(){
		StackTraceElement ste = new Throwable().getStackTrace()[2];
		String tagStr = "Download:" + ste.getClassName() + "_" + ste.getMethodName()+"_"+ste.getLineNumber()+":";
		return tagStr;
	}

}
