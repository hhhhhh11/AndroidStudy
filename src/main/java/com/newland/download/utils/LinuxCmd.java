package com.newland.download.utils;

import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;

/**
 * 执行linux命令
 * 
 * @author maxw
 * 
 */
public class LinuxCmd {

	private static final boolean D = true;
	/**
	 * 执行linux命令
	 * 
	 * @param path
	 *            工作目录
	 * @param cmd
	 *            执行指令
	 * @throws IOException 
	 */
	public static String execCmd(String path, String cmd) throws IOException {
		Log.e(LinuxCmd.class.getSimpleName(), cmd);
		// 创建进程生成器
		ProcessBuilder processBuilder = new ProcessBuilder("/system/bin/sh");
		// 设置新建进程的工作目录
		File dir = new File(path);
		processBuilder.directory(dir);
		// 将errorinputstream也定义到inputstream中
		processBuilder.redirectErrorStream(true);
		// 启动新进程
		Process process = processBuilder.start();

		// 获取进程输入流
		BufferedReader br = new BufferedReader(new InputStreamReader(
				process.getInputStream()));
		// 获取进程输出流
		BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(
				process.getOutputStream()));
		// 执行linux命令似乎必须用PrintWriter
		PrintWriter pw = new PrintWriter(bw, true);

		// 执行命令
		pw.println(cmd);
		// 执行结束，退出
		pw.println("exit");

		StringBuffer sb = new StringBuffer();
		String tempStr;
		while ((tempStr = br.readLine()) != null) {
			sb.append(tempStr);
			sb.append("\n");
		}
		if(D)
			System.out.println("the output is : " + sb.toString());

		br.close();
		bw.flush();
		bw.close();
		pw.flush();
		pw.close();
		process.destroy();
		return sb.toString();
	}
}
