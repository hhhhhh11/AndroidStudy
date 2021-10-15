/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	arq.h
 * @brief	低端设备应用于固件下载，高端设备应用下载，通讯类
 * @version	1.0
 * @author: ym
 * @date	2021/04/12
 ******************************************************************************/

#ifndef _ARQ_H_
#define _ARQ_H_

#include "pubdef.h"
#include <termios.h>


typedef unsigned char seq_nr_t;         /**< Frame Seq type */
typedef unsigned char frame_type_t;     /**< Frame type type */

/** Frame structure, not including FCS  */
typedef struct {
	frame_type_t type;             /* Frame type */
	seq_nr_t seq_nr;               /* Frame seq */
	unsigned char data[1];       /* Frame data */
}frame_t;


#define FSTX 0x7E   /**< Frame Start Character */
#define FDLE 0x7D   /**< Escape Character */

#define FDATA   0x02    /* Data Frame */
#define FACK    0x01    /* ACK Frame */
#define FNAK    0x40    /* NAK Frame */
#define FSYN    0x04    /* Frame Seq Syn Advice, from host, slave do not */

#define IS_FACK(frame)          ((frame)->type & FACK)
#define IS_FNAK(frame)          ((frame)->type & FNAK)
#define IS_FDATA(frame)         ((frame)->type & FDATA)

#define FRAME_HEAD 2
#define FRAME_HOLD (FRAME_HEAD + 2)

#define IS_PREVIOUS_SEQ(a, b) (((unsigned char)((a) - 1)) == (b))

#define SENDBUFFER_16K (0) 
#define SENDBUFFER_4K  (1)

 #define  ANDROID_CLEAR 0x04
 #define  ANDROID_INTERRUPT 0x08
#define  ANDROID_GETAPP 0x11  //获取应用
#define  ANDROID_DELAPP 0x12  //删除应用	
#define  ANDROID_GETAPN 0x13  //获取apn需要的mmc信息
#define NLP_FIRM_TIMEOUT 10000


 /**
 *  @brief 低端平台cruxplus,scropipo平台的应用与固件下载，智能平台全平台应用下载类;
*/
class CARQ  
{
public:
	
	CARQ(bool bClear,int nRetry);
	~CARQ();

	bool OpenPort(const char* pszDev);
	void InitBuffer(int nFlag);
	void FlushPort(void);
	void ClosePort(void);
	int PacketFrame(frame_t* fr, frame_type_t type, unsigned char* pszData, int nDatalen);
	int SendFrame(frame_t* fr, int nHold);
	int RecvFrame(int nTimeout);
	int ReadByteWithTimeout(int nLimit);
	int CheckFcsFrame(unsigned char* frame, int nLen);
	
	int GetMposPlat(int nType=1); 





	/*************************************Android App Start******************************************************/
	int Android_Shakehand(void);  //返回 0：16k版本  1:4K版本，-1：握手失败
	bool Android_GetVersion(char* pszVer);
	int Android_DownLoad_APP(const char* pszFileList);		
	bool Android_GetAppInfo(string& strAppList);
	



	/*************************************NLP Scrop Start******************************************************/
	int ARQSend(unsigned char* pszData, int nDataLen);
	int ARQRecv(unsigned char* pszOut, int nDataLen, int nTimeout);
	int Scrop_DownLoad_APP(const char* pszFileList);		//传入参数下载文件列表,将nlp与android应用下载区分开
	int Scrop_DownLoad_Firm(const char* pszFileList);		//传入参数下载文件列表,将nlp与android应用下载区分开
	int ARQSendMposEnd(unsigned char* pszData, int nLen);

	/*************************************NLP Crux Plus Start******************************************************/
	//帧格式：| 0x02 | CMD | Len_l | Len_h | Data | Checksum |
    //对于 USB CDC： Checksum = 0x0000
    //对于 RS232： Checksum = CRC16-IBM G(x) = x^16 + x^15 + x^2 + 1
	int CRC16DataSend(unsigned char* pszData, int nDataLen); 
	int HDLCSend(unsigned char ucCmd, unsigned char* pszData, int nDataLen);
	int HDLCRecv(unsigned char* pszData, int nTimout, int ck);
	int CruxPlus_DownLoad_APP(const char* pszFileList);
	int CruxPlus_DownLoad_Firm(const char* pszFileList);
	
private:
	int m_fd;
	int m_nRetry;						///<重发次数
	bool m_bClearApp;					///<是否清空应用
	int m_syn;						  	///<序列号同步
	seq_nr_t m_start;                	///< 当前帧序列			      
	frame_t * m_in;                   	///<接收缓冲区
	frame_t * m_out;                   	///<发送缓冲区
	frame_t * m_acknak;               	///<ACK/NAK缓冲区

	unsigned char m_ucFSTX;				///<scrop与智能应用下载协议数据帧第一个字符 
	unsigned char m_ucFDLE;				///<scrop与智能应用下载协议转义字符 ，0x7E转义为0x7D，0x5E，0x7D转义为0x7D，0x5D
	unsigned char m_ucEs1;				///<scrop与智能应用下载协议转义后的第一个字符 
	unsigned char m_ucEs2;				///<scrop与智能应用下载协议转义后的第二个字符 
	
	int m_nFrameLen;					///<发送数据帧长度
	int m_nFrameMaxSize;				///<每帧发送最大有效数据，不包括协议头，校验
	struct termios oldtio;				///<终端结构
	struct termios newtio;				///<终端结构
	

	
};

#endif 
