package com.newland.download.bean;


public enum DownType {

	/**
	 * 下载应用
	 */
	DOWN_APP(0),
	/**
	 * 下载固件
	 */
	DOWN_FIRM(1),
	/**
	 * 获取应用列表
	 */
	DOWN_GETAPP(2),
	/**
	 * 删除应用
	 */
	DOWN_DELAPP(3);
	private int value;

	private DownType(int value){
		this.value = value;
	}
	
	public int getValue() {
		return value;
	}
}
