package com.newland.download.bean;


public enum PosPlat {

	/**
	 * 高端
	 */
	PLAT_ANDROID(0),
    /**
     * 中端
	 */
	PLAT_LINUX(1),
    /**
     * 低端
	 */
	PLAT_MPOS(2);

	private int value;

	private PosPlat(int value){
		this.value = value;
	}
	
	public int getValue() {
		return value;
	}
}
