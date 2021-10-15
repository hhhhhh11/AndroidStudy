/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	arddownload.h
 * @brief	智能设备固件下载类
 * @version	1.0
 * @author: ym
 * @date	2021/05/11
 ******************************************************************************/
#ifndef _ARDDOWNLOAD_H
#define _ARDDOWNLOAD_H
#include "pubdef.h"
#include <string>
using namespace std;


 /** 
   @brief 智能固件下载类，实现ard包解包，newconfig配置文件解析，下载规则判断，与下载功能实现
*/
class CArdDownload{

public:
		CArdDownload(void);
		virtual ~CArdDownload(void);
		void InitMultiNlcArr(void);
		string SplitSubString(string strVer);
		string SplitSecondString(string strVer);
		int ParseFileList(const char* pszFileName);
		bool UnCompressARD(const char* pszARD,int nIndex);		
		int DownloadProcess(const char *pszFileName);
		int GetExecSystem(const char* in,char *out);
		int ReadGroupAtt(const char* pszFileName,int nIndex);	
		int MakeBootDownFile_Android(string strFPath, string strBootName,int nIndex);
		int MakeBiosDownFile_Android(string strFPath,GROUP_FILE_ATT gfAtt,int nIndex);
		void LogFirm(const char* pszLogs);
		void GetLogTag(void);
		int Download(const char* pszDownFileList,int nNum);
		int WaitForConnect(void);
		static void* ThreadFunc(void *arg);

private:
	string m_strBootSectorFile;	///<boot下载文件列表 	
	string m_strBiosDownList;	///<非boot固件下载文件列表
	int m_bWriteLog;			///<是否记录固件下载日志
	int m_nAboot;				///<是否下发aboot	
	int m_nSlaveCount ;			///<list文件配置slave数量	
	int m_nErgodic ;			///<是否3个特殊判断规则	
	char m_szCmd[256];			///<fastboot线程使用
	bool bExitThread;			///<fastboot线程是否已退出
		
};

#endif
