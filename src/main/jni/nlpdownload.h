/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	nlpdownload.h
 * @brief	nlp文件下载与智能平台应用下载
 * @version	1.0
 * @author: ym
 * @date	2021/04/16
 ******************************************************************************/

#ifndef _NLPDOWNLOAD_H
#define _NLPDOWNLOAD_H
#include "pubdef.h"
#include "common.h"
#include <vector>
using namespace std;


 /** 
   @brief MPOS与智能设备应用 下载类
*/
class CNLPDownload{
	
public:
	
	CNLPDownload();
	~CNLPDownload();
	void UnpackZipFile(const char *pszZipFile);
	bool ParseAndroidFileList(const char* pszFileName,bool bList);
	bool FindZIPFile(string strFolderPath,string strFile);
	void ParseAndroidList(string strFileList);
	void MakeAndroidDownFile(string strVersion);	//android 的ota包下载，需要根据获取的boot版本信息判断，所有组包在通讯类中处理，其它nld，nlp等应用下载，直接在main函数处理即可
	int GetAppList(const char* pszDev);
	int DownloadProcess(const char* pszFileList,const char* pszDev,bool bClear=false,int nType=sType_MPOS_APP);
private:
	int m_nFileCount; 											///<android应用数量
	int m_nFileType[MAX_DOWNFILE_NUM];							///<文件类型	
	char m_szDownFullPath[MAX_DOWNFILE_NUM][MAX_PATH] ;			///<android 下载应用路径	
	string m_strDownZip;										///<zip解压后的待下载文件全路径	
	string m_strDownList;										///<待下载文件列表	
	string m_strDownZipTmp;										///<zip解压后的待下载文件	
	vector<OTA_ZIP_INFO>m_vec_otainfo; 							///<ota包结构 容器
};

#endif
