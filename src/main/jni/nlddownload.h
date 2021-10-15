/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	nlddownload.h
 * @brief	nld文件下载类
 * @version	1.0
 * @author: ym
 * @date	2021/04/14
 ******************************************************************************/

#ifndef _NLDDOWNLOAD_H_
#define _NLDDOWNLOAD_H_
#include "nld.h"
#include "pubdef.h"
#include "usbserial.h"
#include <queue>
#include <string>
using namespace std;

#define NEW_MAX_BUFF_LEN 1024*100    ///<定义用于发送下载命令的最大的缓冲空间,100k
#define NEW_FRAME_BUFF_LEN 1024*24   ///<nld当前下载发送数据帧大小 24k
#define NLD_TIME_OUT	10000		 ///<接收超时时间 ms


/**
 *@brief	与POS交互命令字定义
*/
typedef enum POS_MSG_TYPE_E
{	
	POS_SHAKEHAND			= 0x0100,	/*pos握手*/
	POS_REQUEST				= 0x0200,	/*pos请求操作*/
	POS_DELAPP_ALL			= 0x0400,	/*删除pos全部应用*/
	POS_DELAPP_PART			= 0x0500,	/*删除pos部分应用*/
	POS_GETAPP				= 0x0600,	/*获取应用信息*/
	POS_DOWNAPP				= 0x0700,	/*下载应用*/
	POS_INSTAPP				= 0x0800,	/*安装应用*/
	POS_END					= 0xff00	/*流程结束命令*/
}POS_MSG_TYPE_E;



/**
 *@brief	下载协议报文头部格式
*/
typedef struct _MSG_HEAD
{
	U16 uMagic;			/*包头固定填充,主动发起的时候填充0xaaaa，响应命令的时候0xcccc */
	U16 uSurport;		/*命令是否支持  1:支持   0:不支持*/
	U16 uCommand;		/*交互命令*/
	U16 uProVer;		/*交互协议版本，版本初始定义为1.0，用*/
	U16 uTotalNum;		/*报文总数*/
	U16 uSeq;			/*当前序列*/
	U16 uFieldCount;	/*buff中包含几个组成部分*/
	U32 uDataLen;		/*buff中的长度*/
}MSG_HEAD;



/**
 *@brief	中端平台应用下载类
*/
class CNLDDownload{
public:
	CNLDDownload(void);
	~CNLDDownload(void);

	void SetClearFlag(bool bClear);
	int  DownloadProcess(const char* pszFileList,const char* pszDev);
	void GetInfoCmd(U16 uProtocol);
	int  ShakeHand(void);
	bool GetPOSInfo(char* pszPosInfo,int* pnLen);
	bool ClearApp(void);
	bool DownloadApp(const char* pszDownFileList,int nMaxSend);
	bool GetNextFile (void);
	void DownloadEnd(void);
	int GetAppList(const char* pszDev);
	bool GetAppProcess(int* nAppCount,char* pszAppList);
	void ParseAppList(string strFileList);
	bool DelAppList(const char* pszAppList,const char* pszDev);
	bool DelAppProcess(int nAppCount,const char* pszAppList);
	
private:

	queue <string> m_queFilepath;				///< 该队列用来存储需要下载到POS的文件
	queue <unsigned char> m_queFiletype;		///< 用来存储文件的类型
	int m_ArrInfoCount[10];						///< 各种协议类型所获取的数量
	int m_ArrInfoCmd[10][100];					///< 存放协议类型的命令字
	CUsbSerial m_objUsbSerial;					///< usbserail通讯对象
	bool m_bClear;								///< 是否清空

};

#endif

