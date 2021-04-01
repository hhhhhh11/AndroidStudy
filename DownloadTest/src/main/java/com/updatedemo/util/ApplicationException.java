package com.updatedemo.util;

import java.text.MessageFormat;

/**
 * 应用异常
 *     
 * @author maxw  
 * @date 2014-2-25 
 *
 */
public class ApplicationException extends Exception {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	public int code;
	
	public ApplicationException(int code, String msg, Throwable e) {
		super(msg, e);
		this.code = code;
	}
	
	public ApplicationException(int code, String msg) {
		super(msg);
		this.code = code;
	}
	
	public int getCode(){
		return code;
	}
	
	@Override
	public String getLocalizedMessage() {
		return MessageFormat.format("{{0}}", super.getLocalizedMessage());
	}

}
