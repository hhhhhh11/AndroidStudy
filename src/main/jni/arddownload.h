/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	arddownload.h
 * @brief	�����豸�̼�������
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
   @brief ���ܹ̼������࣬ʵ��ard�������newconfig�����ļ����������ع����жϣ������ع���ʵ��
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
	string m_strBootSectorFile;	///<boot�����ļ��б� 	
	string m_strBiosDownList;	///<��boot�̼������ļ��б�
	int m_bWriteLog;			///<�Ƿ��¼�̼�������־
	int m_nAboot;				///<�Ƿ��·�aboot	
	int m_nSlaveCount ;			///<list�ļ�����slave����	
	int m_nErgodic ;			///<�Ƿ�3�������жϹ���	
	char m_szCmd[256];			///<fastboot�߳�ʹ��
	bool bExitThread;			///<fastboot�߳��Ƿ����˳�
		
};

#endif
