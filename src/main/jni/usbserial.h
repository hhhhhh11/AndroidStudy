/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	usbserial.h
 * @brief	Download Communication class for *.NLC;*.NLD
 * @version	1.0
 * @author: ym
 * @date	2021/03/20
 ******************************************************************************/

#ifndef _USBSERIAL_H_
#define _USBSERIAL_H_

#include<termios.h> 

 


 /** 
   @brief usb-serial通讯类，实现usb-serial的打开，关闭，读写数据。 该类适用于类unix系统与mac系统
*/
class CUsbSerial{

public:
	CUsbSerial();
	~CUsbSerial();
		
	bool OpenPort(const char* pszDev);
	void FlushPort(void);
	void ClosePort(void);
	void SetBlock(bool bBlock);
	void SetPreInfo(int nLen,int nSleep);
	int SendBuffData(const char* pzSendData,int nDataLen);
	int ReadBuffData( char* pszReadData,int nReadLen,int nLimit=1000);
	
private:
	int m_fd;						///<通讯句柄					
	struct termios 	oldtio;			///<终端结构
	struct termios	newtio;			///<终端结构	
	int m_nPreLen;					///<非阻塞模式，每帧发送数据大小			
	int m_nPreSleep;				///<非阻塞模式，每帧数据发送后，延时时间
	
};


#endif