/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	function.h
 * @brief	通用函数
 * @version	1.0
 * @author: ym
 * @date	2021/04/16
 ******************************************************************************/
#ifndef  _FUNCTION_H_
#define  _FUNCTION_H_
#include <string>  
using namespace std;  

 /** 
   @brief 常用工具类，主要为字符串处理，与路径处理接口
*/
class CFunCommon {

public:
	CFunCommon();
	virtual ~CFunCommon();
    static bool GetAbsolutePath (char* pszAbsolutePath) ;
    static bool CopyFile(const char* pszSrc,const char* pszDst);
    static bool ExecSystem(const char* pszCmd);
    static bool DeleteDir(const char* pszdir);
    static void DeleteIndexString(string& strSrc,  const string strMark) ;
    static int  ReplaceAnsi(char* pszSrc,char cOld,char cNew);
    static int GetIndexOf( char* pszSrc,  char* pszMark ) ;
    static int Trim(char* pszStr);
    static int GetAbsolutePathFileName(char* pszPath,char* pszFileName);
    static int GetAbsolutePath(char* pszPath,char* pszFileName);
    static bool GetFileNameFromFolder(string strFolderPath,string& strFile);
    static int  ReplaceAll(char* pszSrc,char cOld,char cNew);

    static int DlSprintf(const char * format, ...);
#if 0

void LanguageShow(char *strMessage);
//int GetfilePath(char * strFileName);
//int ReadFileContent(char *strPath, char *strContent);



int GetFileSize(char *szFilePath,char *szSize);
int IsAbsolutePath(char *strPath);
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen);
int u2g(char *inbuf,int inlen,char *outbuf,int outlen) ;
int g2u(char *inbuf,int inlen,char *outbuf,int outlen) ;
int GetFileReleaseDate(char *szFilePath, char *szDateBuff);
void substring(char *dest,char *src,int start,int end)  ;
#endif

};
#endif
