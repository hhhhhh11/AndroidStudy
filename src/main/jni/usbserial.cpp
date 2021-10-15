/**************************************************************************
* Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
*
* @file usbserail.cpp
* @brief  
* @version 1.0
* @author ym
* @date 2021-4-22
**************************************************************************/
#include"usbserial.h"
#include"pubdef.h"
#include "function.h"
#include<stdio.h>  
#include<sys/types.h>  
#include<sys/stat.h>  
#include<fcntl.h>  
#include<unistd.h>
#include<string>
#include<sys/time.h>
#include<errno.h>

extern bool g_bConnect;
/**
 * @brief	构造函数
*/
CUsbSerial::CUsbSerial()
{
	m_nPreLen = 4096;			
	m_nPreSleep = 10000;			
	m_fd = -1;					//usb-serail 句柄
}

/**
 * @brief	析构函数
*/
CUsbSerial::~CUsbSerial()
{
	if(m_fd>0)
		close(m_fd);
}

/**
 * @brief	打开usb-serial设备
 * @param[in] pszDev	设备号： ttyACM/ttyUSB
 * @return
 * @li  true		成功
 * @li  false		失败
*/
bool CUsbSerial::OpenPort(const char* pszDev)
{	    

  	if (m_fd > 0)
  	{
		close(m_fd);
  	}	
	if((m_fd = open(pszDev,O_RDWR|O_NOCTTY|O_NDELAY)) < 0)  
	
	//if((m_fd = open(pszDev,O_RDWR | O_NOCTTY)) < 0)  	//打开口的时间要很久
	{  
		CFunCommon::DlSprintf("Failed to open %s:%s.\n",pszDev,strerror(errno));
		return false;       
	}   
	fcntl(m_fd, F_SETFL, 0); //设置为阻塞模式
	tcgetattr(m_fd,&oldtio);
	bzero(&newtio,sizeof(newtio));
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag |= CS8; 
	newtio.c_cflag &= ~PARENB;
	newtio.c_cflag &= ~CSTOPB; 
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  
	newtio.c_oflag &= ~OPOST;   
	newtio.c_oflag &= ~(ONLCR | OCRNL);
	//newtio.c_cflag &= ~CRTSCTS;
	cfsetispeed(&newtio,B115200);
	cfsetospeed(&newtio,B115200); 
	newtio.c_iflag = IGNPAR;
	newtio.c_iflag &= ~ (IXON | IXOFF | IXANY |INLCR | ICRNL | IGNCR);
	
	newtio.c_cc[VMIN]=0;  
	newtio.c_cc[VTIME]=0;
	tcflush(m_fd,TCIFLUSH);
	tcsetattr(m_fd,TCSANOW,&newtio);
	return true;	
}

/**
 * @brief	设置串口通讯配置，只有设置非阻塞模式时有用
 * @param[in] nLen 		设置发送每帧数据大小，单位字节
 * @param[in] nSleep  	设置发送每帧数据数据后，延时时间，单位微秒
 * @return	void
*/
void CUsbSerial::SetPreInfo(int nLen,int nSleep)
{
	m_nPreLen = nLen;
	m_nPreSleep = nSleep;
}

/**
 * @brief	往usb-serail发送数据
 * @param[in] pszSendData 待发送的数据
 * @param[in] nDataLen 	发送的数据长度
 * @return
 * @li -1	发送失败
 * @li >0  实际发送的数据长度
*/
int CUsbSerial::SendBuffData(const char* pszSendData,int nDataLen)
{
	int nTotalLen = 0;		//总长度
	int nHasSend = 0;		//已发送
	int nSend = 0;
	int nRet = 0;
	if (NULL == pszSendData || nDataLen <= 0)
	{
		return -1;
	}
		
	nTotalLen =  nDataLen;
	
	while(nTotalLen > 0)
	{

		nSend = nTotalLen>m_nPreLen?m_nPreLen:nTotalLen;
		nRet = write(m_fd, (unsigned char *)pszSendData+nHasSend, nSend);
		if(nRet != nSend)
		{
			CFunCommon::DlSprintf("Failed to write,errinfo:%s\n",strerror(errno));
			return -1;
		}
		nHasSend += nRet;
		nTotalLen -= nRet;	
		//usleep(m_nPreSleep);  //非阻塞时使用
	}
	
	return nHasSend;
}

/**
 * @brief	设置usb-serial通讯是否阻塞模式
 * @param[in] bBlock 是否阻塞模式
 * @return	void
*/
void CUsbSerial::SetBlock(bool bBlock)
{
	int nFlags = 0;
	if (bBlock)
	{
		nFlags = fcntl(m_fd,F_GETFL,0);
		nFlags &= ~O_NONBLOCK;
		fcntl(m_fd,F_SETFL,nFlags);

	
	}
	else{
		nFlags = fcntl(m_fd,F_GETFL,0);
		nFlags |= O_NONBLOCK;
		fcntl(m_fd,F_SETFL,nFlags);
	}
	
}


/**
 * @brief	从usb-serail读取数据
 * @param[in] pReadData 读取的数据
 * @param[in] nReadLen 	数据长度
 * @param[in] nLimit    超时时间 ms
 * @return
 * @li 读取数据长度 = nReadLen	成功
 * @li 读取数据长度 != nReadLen	失败
*/
int CUsbSerial::ReadBuffData(char* pszReadData,int nReadLen,int nLimit)
{
	int nHaveRead = 0;	//已读取
	int nRet = 0;
	int nTimeOut = nLimit;	
	
	while (nTimeOut > 0)
	{
		nRet = 0;
		if (nHaveRead == nReadLen)
		{
			return nHaveRead;
		}	
		if (0 == nTimeOut)
		{ 
			return nHaveRead;
		}	
		nRet = read(m_fd,pszReadData+nHaveRead,1);
		if (1 == nRet)
		{		
			nHaveRead++;	
			//CFunCommon::DlSprintf("nHaveRead = %d,pszReadData = %02x \n",nHaveRead,pszReadData[nHaveRead-1]);	
			
		}
		else
		{  
			usleep(1000);
			nTimeOut--;
		}
		if (!g_bConnect){
			return -1;
		}
	}
	return nHaveRead; 



}

/**
 * @brief	清空usb-serail串口读写缓冲区
 * @return 	void
*/
void CUsbSerial::FlushPort(void)
{
	tcflush(m_fd,TCIOFLUSH);
}

/**
 * @brief	关闭usb-serial
 * @return	void
*/
void CUsbSerial::ClosePort(void)
{
	if(m_fd > 0)
	{
		close(m_fd);	
		tcsetattr(m_fd,TCSANOW,&oldtio); 
	}
	
}
