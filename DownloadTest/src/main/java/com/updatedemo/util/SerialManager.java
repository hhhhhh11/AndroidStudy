package com.updatedemo.util;

import android.content.Context;
import android.newland.AnalogSerialManager;
import android.newland.content.NlContext;

import com.lidroid.xutils.util.LogUtils;

public class SerialManager {

	private final AnalogSerialManager mAnalogSerialManager;

	public SerialManager(Context context) {

//		this.mAnalogSerialManager = (AnalogSerialManager) context
//				.getSystemService(NlContext.ANALOG_SERIAL_SERVICE);

		this.mAnalogSerialManager = (AnalogSerialManager) context
				.getSystemService(NlContext.ANALOG_SERIAL_SERVICE);

	}

	public void initAnalogSerial() {
		LogUtils.e("串口：" + mAnalogSerialManager);
		int ret=0;
		ret=mAnalogSerialManager.open();
		LogUtils.e("ret: "+ret);
		mAnalogSerialManager.setconfig(115200, 0, "8N1NN".getBytes());
		clearSerial();
	}

	public int serialWrite(byte[] buf, int lengthMax, int timeoutSec) {
		int ret = mAnalogSerialManager.write(buf, lengthMax, timeoutSec);
		return ret;
	}

	public int serialRead(byte[] buf, int lengthMax, int timeoutSec) {
		int ret = mAnalogSerialManager.read(buf, lengthMax, timeoutSec);
		return ret;
	}

	public int serialClose() {
		int ret = mAnalogSerialManager.close();
		return ret;
	}

	/**
	 * 清空串口
	 */
	public void clearSerial() {
		int repeatTimes = 3;
		byte[] buffer = new byte[256];
		int ret = -1;
		while (repeatTimes > 0) {
			ret = mAnalogSerialManager.read(buffer, buffer.length, 0);
			if (ret > 0) {
				repeatTimes = 3;
			} else {
				repeatTimes--;
			}
		}
	}
}
