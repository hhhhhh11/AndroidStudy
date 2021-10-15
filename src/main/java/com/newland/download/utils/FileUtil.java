package com.newland.download.utils;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.CharArrayWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.math.BigDecimal;
import java.nio.charset.StandardCharsets;

/**
 * 文件操作工具类
 * @author maxw
 *
 */
public class FileUtil {

	private static final String TAG = "FileOperation";
	private static final boolean D = true;

	/**
	 * 默认缓存大小
	 */
	private static final int DefaultBufferSize = 100;

	/********************************************************/
	/** bt字节参考量 */
	public static final long SIZE_BT = 1024L;
	/** KB字节参考量 */
	public static final long SIZE_KB = SIZE_BT * 1024L;
	/** MB字节参考量 */
	public static final long SIZE_MB = SIZE_KB * 1024L;
	/** GB字节参考量 */
	public static final long SIZE_GB = SIZE_MB * 1024L;
	/** TB字节参考量 */
	public static final long SIZE_TB = SIZE_GB * 1024L;
	public static final int SACLE = 2;

	/**
	 * 以用户可读方式获取文件大小
	 *
	 * @param size
	 * @return
	 */
	public static String getFileSize(long size) {
		if (size >= 0 && size < SIZE_BT) {
			return size + "B";
		} else if (size >= SIZE_BT && size < SIZE_KB) {
			return size / SIZE_BT + "KB";
		} else if (size >= SIZE_KB && size < SIZE_MB) {
			return size / SIZE_KB + "MB";
		} else if (size >= SIZE_MB && size < SIZE_GB) {
			BigDecimal longs = new BigDecimal(Double.valueOf(size + "").toString());
			BigDecimal sizeMB = new BigDecimal(Double.valueOf(SIZE_MB + "").toString());
			String result = longs.divide(sizeMB, SACLE, BigDecimal.ROUND_HALF_UP).toString();
			return result + "GB";
		} else {
			BigDecimal longs = new BigDecimal(Double.valueOf(size + "").toString());
			BigDecimal sizeMB = new BigDecimal(Double.valueOf(SIZE_GB + "").toString());
			String result = longs.divide(sizeMB, SACLE, BigDecimal.ROUND_HALF_UP).toString();
			return result + "TB";
		}
	}
	/**
	 * 以字节为单位读取文件，常用于读取二进制文件，如图片、声音、影像等文件
	 * 
	 * @param fileName
	 *            文件名
	 * @return 字节数组
	 */
	public static byte[] readFileByBytes(File file) {

		BufferedInputStream bufferedInputStream = null;
		byte[] tempBuf = new byte[DefaultBufferSize];
		int byteRead = 0;
		ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();

		try {
			bufferedInputStream = new BufferedInputStream(new FileInputStream(
					file));
			while ((byteRead = bufferedInputStream.read(tempBuf)) != -1) {
				byteArrayOutputStream.write(tempBuf, 0, byteRead);
			}
			bufferedInputStream.close();
			return byteArrayOutputStream.toByteArray();
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}



	/**
	 * 以字节为单位写文件，常用于生成二进制文件
	 * 
	 * @param file
	 *            文件
	 * @param contentBytes
	 *            要写入文件的字节数组
	 * @param append
	 *            是否以追加的方式写文件
	 */
	public static void writeFileByBytes(File file, byte[] contentBytes,
                                        boolean append) {
		BufferedOutputStream bufferedOutputStream = null;

		try {
			if (!file.exists()) {
				file.createNewFile();
			}
			bufferedOutputStream = new BufferedOutputStream(
					new FileOutputStream(file, append));
			bufferedOutputStream.write(contentBytes);
			bufferedOutputStream.flush();
			bufferedOutputStream.close();
		} catch (Exception e) {
			e.printStackTrace();
			return;
		}
	}

	/**
	 * 以字符为单位读取文件，常用于读取文本、数字等类型的文件
	 * 
	 * @param fileName
	 *            文件名
	 * @return 字符数组
	 */
	public static char[] readFileByChars(File file) {

		BufferedReader bufferedReader = null;
		CharArrayWriter charArrayWriter = new CharArrayWriter();
		char[] tempBuf = new char[DefaultBufferSize];
		int charRead = 0;

		try {
			bufferedReader = new BufferedReader(new InputStreamReader(
					new FileInputStream(file)));
			while ((charRead = bufferedReader.read(tempBuf)) != -1) {
				charArrayWriter.write(tempBuf, 0, charRead);
			}
			bufferedReader.close();
			return charArrayWriter.toCharArray();
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
	}

	public static void writeFileByChars(File file, char[] content,
                                        boolean append) {

		BufferedWriter bufferedWriter = null;

		try {
			bufferedWriter = new BufferedWriter(new OutputStreamWriter(
					new FileOutputStream(file)));
			bufferedWriter.write(content);
			bufferedWriter.flush();
			bufferedWriter.close();
		} catch (Exception e) {
			e.printStackTrace();
			return;
		}
	}

	/**
	 * 以行为单位读取文件，常用于读取以行为单位为格式的文件
	 * 
	 * @param fileName
	 *            文件名
	 * @return 文件内容
	 */
	public static String readFileByLines(File file) {

		if(file == null || !file.exists())
			return null;
		BufferedReader bufferedReader = null;
		StringBuilder stringBuilder = new StringBuilder();

		try {
			bufferedReader = new BufferedReader(new InputStreamReader(
					new FileInputStream(file)));
			String tempString = null;
			while ((tempString = bufferedReader.readLine()) != null) {
				stringBuilder.append(tempString).append("\r\n");
			}
			bufferedReader.close();
			return stringBuilder.toString();
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		} finally {
			if (bufferedReader != null) {
				try {
					bufferedReader.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}

	/**
	 * 以行为单位读取文件，常用于读取以行为单位为格式的文件
	 * 
	 * @param fileName
	 *            文件名
	 * @return 文件内容
	 * @throws IOException
	 */
	public static String readFileByLines2(File file) throws IOException {

		BufferedReader bufferedReader = null;
		StringBuilder stringBuilder = new StringBuilder();

		try {
			bufferedReader = new BufferedReader(new InputStreamReader(
					new FileInputStream(file), StandardCharsets.UTF_8));
			String tempString = null;
			while ((tempString = bufferedReader.readLine()) != null) {
				stringBuilder.append(tempString).append("\n");
			}
			bufferedReader.close();
			return stringBuilder.toString();
		} finally {
			if (bufferedReader != null) {
				try {
					bufferedReader.close();
				} catch (IOException e) {
					e.printStackTrace();
				} 
			}
		}
	}

	/**
	 * 以字符流方式写文件，常用于保存文本文件
	 * 
	 * @param fileName
	 *            文件名
	 * @param content
	 *            要保存的内容
	 */
	public static void writeFileByString(File file, String content,
                                         boolean append) {

		BufferedWriter bufferedWriter = null;

		try {
			if (!file.exists()) {
				file.createNewFile();
			}
			bufferedWriter = new BufferedWriter(new OutputStreamWriter(
					new FileOutputStream(file, append)));
			bufferedWriter.write(content);
			bufferedWriter.flush();
			bufferedWriter.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * 拷贝文件
	 * 
	 * @param srcFile
	 *            源文件
	 * @param dstFile
	 *            目标文件
	 * @return
	 */
	public static boolean copyFileByBytes(File srcFile, File dstFile) {

		 int byteRead = 0;

		// 目标文件已经存在就不进行复制
		if (dstFile.exists()) {
			return true;
		} else {
			if (srcFile.exists()) {
				if (D)
					System.out.println("Begin to move file!");

				try {
					 InputStream is = new FileInputStream(srcFile);
					 FileOutputStream fos = new FileOutputStream(dstFile);
					
					 byte[] buffer = new byte[4096];
					 while ((byteRead = is.read(buffer)) != -1) {
					 fos.write(buffer, 0, byteRead);
					 }
					if (D)
						System.out.println("Move file end!");

				} catch (Exception e) {
					e.printStackTrace();

					return false;
				}
				return true;
			}
		}
		return false;
	}


	public static boolean coverFileByBytes(File srcFile, File dstFile) {

		int byteRead = 0;
		InputStream is = null;
		FileOutputStream fos = null;
		if (srcFile.exists()) {
			if (D)
				System.out.println("Begin to move file!");

			try {
				 is = new FileInputStream(srcFile);
				 fos = new FileOutputStream(dstFile);

				byte[] buffer = new byte[4096];
				while ((byteRead = is.read(buffer)) != -1) {
					fos.write(buffer, 0, byteRead);
				}
				if (D)
					System.out.println("Move file end!");

			} catch (Exception e) {
				e.printStackTrace();

				return false;
			}finally {
				if (is != null) {
					try {
						is.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
				if (fos != null) {
					try {
						fos.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
			return true;
		}
		return false;
	}
}
