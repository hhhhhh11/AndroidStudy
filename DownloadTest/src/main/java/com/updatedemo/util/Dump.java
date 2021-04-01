package com.updatedemo.util;

/**
 * 绯荤粺浜岃繘鍒舵棩蹇楁墦鍗板伐鍏风被
 * 
 * @author szshen
 *
 * @since ver1.0
 */
public class Dump {
	private static final byte[] highDigits;

	private static final byte[] lowDigits;

	// initialize lookup tables
	static {
		final byte[] digits = { '0', '1', '2', '3', '4', '5', '6', '7', '8',
				'9', 'A', 'B', 'C', 'D', 'E', 'F' };

		int i;
		byte[] high = new byte[256];
		byte[] low = new byte[256];

		for (i = 0; i < 256; i++) {
			high[i] = digits[i >>> 4];
			low[i] = digits[i & 0x0F];
		}

		highDigits = high;
		lowDigits = low;
	}

	/**
	 * 灏嗗瓧鑺傛祦杞崲鎴愪竴涓彲闃呰鐨�16杩涘埗琛ㄨ堪鐨勫瓧绗︿覆
	 * <p>
	 * 
	 * @param bytes
	 *            杈撳叆瀛楄妭
	 * @param offset
	 *            鍋忕Щ閲�
	 * @param length
	 *            闀垮害
	 * @return 16杩涘埗琛ㄨ堪鐨勫瓧绗︿覆
	 *
	 * @since ver1.0
	 */
	public static String getHexDump(byte[] bytes, int offset, int length) {

		if (bytes == null || bytes.length == 0)
			return "empty";
		if (offset >= bytes.length) {
			return "out of length,totallen:" + bytes.length + ",offset:"
					+ offset;
		}

		StringBuffer out = new StringBuffer();

		int byteValue = bytes[offset] & 0xFF;
		out.append((char) highDigits[byteValue]);
		out.append((char) lowDigits[byteValue]);

		for (int i = offset + 1; (i < bytes.length && (i - offset) < length); i++) {
			out.append(' ');
			byteValue = bytes[i] & 0xFF;
			out.append((char) highDigits[byteValue]);
			out.append((char) lowDigits[byteValue]);
		}
		return out.toString();
	}

	/**
	 * 灏嗗瓧鑺傛祦杞崲鎴愪竴涓彲闃呰鐨�16杩涘埗琛ㄨ堪鐨勫瓧绗︿覆
	 * <p>
	 * 
	 * @param bytes
	 *            杈撳叆瀛楄妭
	 * @return 16杩涘埗琛ㄨ堪鐨勫瓧绗︿覆
	 * @since ver1.0
	 */
	public static String getHexDump(byte[] bytes) {
		return getHexDump(bytes, 0, bytes.length);
	}

}
