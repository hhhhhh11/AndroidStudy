package com.updatedemo.util;

import android.util.Log;

import java.nio.ByteBuffer;

public class BCDUtil {

	public static byte[] DecToBCDArray(long num) {
		int digits = 0;
 
		long temp = num;
		while (temp != 0) {
			digits++;
			temp /= 10;
		}
 
		int byteLen = digits % 2 == 0 ? digits / 2 : (digits + 1) / 2;
		boolean isOdd = digits % 2 != 0;
 
		byte bcd[] = new byte[byteLen];
 
		for (int i = 0; i < digits; i++) {
			byte tmp = (byte) (num % 10);
 
			if (i == digits - 1 && isOdd)
				bcd[i / 2] = tmp;
			else if (i % 2 == 0)
				bcd[i / 2] = tmp;
			else {
				byte foo = (byte) (tmp << 4);
				bcd[i / 2] |= foo;
			}
 
			num /= 10;
		}
 
		for (int i = 0; i < byteLen / 2; i++) {
			byte tmp = bcd[i];
			bcd[i] = bcd[byteLen - i - 1];
			bcd[byteLen - i - 1] = tmp;
		}
 
		return bcd;
	}
	
	public static byte[] DecToBCDArray(long num, int bcdArrayLenth){
		byte[] bcd = DecToBCDArray(num);
		ByteBuffer byteBuffer = ByteBuffer.allocate(bcdArrayLenth);
		int fill = bcdArrayLenth - bcd.length;
		while(fill-->0){
			byteBuffer.put((byte) 0);
		}
		byteBuffer.put(bcd);
		return byteBuffer.array();
	}
	
	public static int bcd2Int() {
		return 0;
	}
	
	/**
	 * 将BCD码转换成字符串
	 * 
	 * 主旨：按4位处理
	 * 
	 * @param b BCD字节
	 * @param len	BCD字节长度 eg：2个字节的BCD，len就是2*2，因为一个字节8位，拆分为2个4位
	 * @return
	 */
	public static String bcd2Str(byte[] b, int len) {
		StringBuilder d = new StringBuilder(len);
		int start = ((len & 1) == 1)? 1 : 0;	// 一个字节高4位标记为0，低4位标记为1，eg：2个字节就拆分为0,1,2,3
		for(int i = start; i < len; i++){
			int shift = ((i & 1) == 1 ? 0 : 4); // 基数位为低位，不需要移动，偶数位位高位，需要向左移动4位
			char c = Character.forDigit((b[i>>1] >> shift) & 0x0f, 16);// i>>1
			if(c == 'd')
				c = '=';
			d.append(c);
		}
		return d.toString();
	}
	
	public static int bcd2Int(byte[] b, int len){
		String intStr = bcd2Str(b, len);
		Log.e("", "{intStr}"+intStr);
		return Integer.parseInt(intStr);
	}
}
