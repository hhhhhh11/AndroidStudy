#include "arddownload.h"
#include "function.h"
#include "lzss.h"
#include "inirw.h"
#include "zlib.h"
#include "common.h"
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <errno.h>
#include <dlfcn.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>  
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>





static NLC_INFO Arr_Ard[MAX_ARD_NUM];		//存放多ARD文件的数组，目前最大的数量支持MAX_DOWNFILE_NUM个ARD包
extern char g_szAppPath[MAX_PATH];
extern char g_szMessage[MSGSUM][100];
extern PFCALLBACK gCallBack ;

//fastboot 命令 
#define PRODUCT_ID "0x18d1"
#define GETDEVICES "devices"
#define GETVERSION "version"
#define GETVARIOUS	"GetVariousInfo"
#define GETPRODUCT "product"
#define GETSERIALNO	"serialno"
#define GETSECURE	"secure"
#define DOWNLOADBIN	"download"
#define REBOOTBOOTLOADER "reboot-bootloader"
#define REBOOT		"reboot"



/**
 * @brief 构造
*/
CArdDownload::CArdDownload(void)
{
	m_bWriteLog = 1;
	m_nSlaveCount = 0;
	m_nErgodic = -1 ;			
	
}

/**
 * @brief 析构
*/
CArdDownload::~CArdDownload(void)
{
}




/**
 * @brief 	初始化Arr_Ard数组
 * @return	void      
*/
void CArdDownload::InitMultiNlcArr(void)
{
	for (int i=0;i<MAX_ARD_NUM;i++)
	{
		Arr_Ard[i].bIsUsed=FALSE;
		memset(Arr_Ard[i].szFileName,0,sizeof(Arr_Ard[i].szFileName));
		memset(Arr_Ard[i].szFilePath,0,sizeof(Arr_Ard[i].szFilePath));
		Arr_Ard[i].nFirmPackVer=-1;
		Arr_Ard[i].nFullPack=-1;
		Arr_Ard[i].nGroupCount=-1;
	}
	return;
}

/**
 * @brief 		解析传入的版本信息，(pos返回或本地配置文件中读取)
 * @param[in]  strVer    	传入的设备版本信息
 * @return					解析到的第三至第7字段信息       
*/
string CArdDownload::SplitSubString(string strVer)
{
	int nPos = -1;
	int i = 0;
	string strRight;
	string strRet;
	//根据默认3-7域
	while(1)
	{
		nPos = strVer.find('.');  
		if (nPos == string::npos)  
		{  
			break;  
		}  
		if (i >= 2)
		{
			strRet = strRight.substr(0,strRight.rfind('.'));
			break;
		}
		strRight = strVer.substr(nPos+1);
		strVer = strRight;
		i++;
	}
	return strRet;
}

/**
 * @brief 	解析传入的版本信息，(pos返回或本地配置文件中读取)
 * @param[in]  strVer    	传入的设备版本信息
 * @return			解析第二字段model    	
*/
string CArdDownload::SplitSecondString(string strVer)
{
	int iPos;
	int i=0;
	string strRight;
	string strRet;
	while(1)
	{
		iPos = strVer.find('.');  
		if (iPos == string::npos)  
		{  
			break;  
		}  
		if (1 == i)
		{
			strRet = strRight.substr(0,strRight.find('.'));
			break;
		}
		strRight = strVer.substr(iPos+1);
		strVer = strRight;
		i++;
	}
	return strRet;
}




/**
 * @brief 读取newconfig.ini
 * @param[in] pszFileName    传入的文件名称
 * @param[in] nIndex   	  unzip目录下目录索引
 * @return
 * @li 	0		成功
 * @li 	-1 		失败
*/
int CArdDownload::ReadGroupAtt(const char* pszFileName,int nIndex)
{


	int i,j;
	int nCount = 0; 
	string strFile,strMark,strMark_1, strSectorName, strFullPath,strPackName;
	char szIndex[128] = {0};
	char szTmp[128] = {0};
	char msg[256] = {0};
	
	CIniRW ini;
	if((access(pszFileName,F_OK)) == -1)  
	{
		CFunCommon::DlSprintf("Failed: %s doesn't  exist!\n",pszFileName);
		return -1;

	}
	ini.iniFileLoad(pszFileName);
	Arr_Ard[nIndex].bIsUsed=true;	
	Arr_Ard[nIndex].nGroupCount = ini.iniGetInt("GROUPCOUNT","count",-1);
	if (-1 == Arr_Ard[nIndex].nGroupCount || 0 == Arr_Ard[nIndex].nGroupCount)
	{
		Arr_Ard[nIndex].nFirmPackVer = 0;
		Arr_Ard[nIndex].nFullPack = 0;
	}
	else
	{	
		Arr_Ard[nIndex].nFirmPackVer=ini.iniGetInt("ATTR","PACKVER",-1);
		Arr_Ard[nIndex].nFullPack=ini.iniGetInt("ATTR","FULLFLAG",0);
		if ((Arr_Ard[nIndex].nGroupCount > 0) && (-1 == Arr_Ard[nIndex].nFirmPackVer))
			Arr_Ard[nIndex].nFirmPackVer = 1;
	}
	
	for(i=0; i<Arr_Ard[nIndex].nGroupCount; i++)
	{
		strMark = "GROUP";
		sprintf(szIndex,"%d",i);
		strMark += szIndex;
		memset(szTmp, 0x00, sizeof(szTmp));
		//CFunCommon::DlSprintf("strMark=%s\n",strMark.c_str());
		ini.iniGetString(strMark.c_str(),"ver",szTmp,sizeof(szTmp),"Error");
		strcpy(Arr_Ard[nIndex].szGroupFileAtt[i].GrAttitude.szGroupVer,szTmp);	
	
		Arr_Ard[nIndex].szGroupFileAtt[i].GrAttitude.GroupCount=ini.iniGetInt(strMark.c_str(),"count",0);
		Arr_Ard[nIndex].szGroupFileAtt[i].GrAttitude.iBoot=ini.iniGetInt(strMark.c_str(),"boot",0);
		
		for(j=0; j<Arr_Ard[nIndex].szGroupFileAtt[i].GrAttitude.GroupCount; j++)
		{
			strFile = "FILE";
			sprintf(szIndex,"%d",j);
			strFile += szIndex;
			memset(szTmp, 0x00, sizeof(szTmp));
			ini.iniGetString(strMark.c_str(),strFile.c_str(),szTmp,sizeof(szTmp),"Error");
			strcpy(Arr_Ard[nIndex].szGroupFileAtt[i].szGroupFile[j],szTmp);
		}
		
	}
	memset(Arr_Ard[nIndex].szBiospara,0,sizeof(Arr_Ard[nIndex].szBiospara));
	nCount =ini.iniGetInt("FILECOUNT","count",0); 
	Arr_Ard[nIndex].nBinFileCount = nCount;
	for (i = 0; i<nCount; i++)
	{
		memset(szIndex,0,sizeof(szIndex));
		sprintf(szIndex,"FILE%d",i);
		memset(szTmp,0,sizeof(szTmp));
		ini.iniGetString(szIndex,"sector",szTmp,sizeof(szTmp),"Error");
		strcpy(Arr_Ard[nIndex].szBiospara[i].szSector,szTmp);
		//CFunCommon::DlSprintf("tmp=%s\n",tmp);
		memset(szTmp,0,sizeof(szTmp));
		ini.iniGetString(szIndex,"fullname",szTmp,sizeof(szTmp),"Error");
		strcpy(Arr_Ard[nIndex].szBiospara[i].szFullname,szTmp);
		//CFunCommon::DlSprintf("tmp=%s\n",tmp);
		memset(szTmp,0,sizeof(szTmp));
		ini.iniGetString(szIndex,"ver",szTmp,sizeof(szTmp),"Error");
		strcpy(Arr_Ard[nIndex].szBiospara[i].szVer,szTmp);
		//CFunCommon::DlSprintf("tmp=%s\n",tmp);
		Arr_Ard[nIndex].szBiospara[i].cIsupdate=ini.iniGetInt(szIndex,"isupdate",0); 
		//CFunCommon::DlSprintf("cIsupdate=%d\n",Arr_Ard[nIndex].szBiospara[i].cIsupdate);
	}
	strcpy(Arr_Ard[nIndex].szFileName , pszFileName);
	ini.iniFileFree();
	return 0;
}





/**
 * @brief 	生成boot下载文件列表
 * @param[in]  csFPath    	传入的unzip下具体子目录
 * @param[in]  csBootName     	传入的boot名称
 * @param[in]  nIndex  	全局变量Arr_Nlc需要使用到的索引
 * @return
 * @li  0        	成功
 * @li  -1        	失败
*/
int CArdDownload::MakeBootDownFile_Android(string strFPath, string strBootName,int nIndex)
{
	string strSector;
	if (0 == Arr_Ard[nIndex].nBinFileCount) 
			return -1;
		
	for (int i = 0; i < Arr_Ard[nIndex].nBinFileCount; i++)
	{
		if (strcmp(strBootName.c_str(),Arr_Ard[nIndex].szBiospara[i].szFullname)==0)
		{
			strSector = Arr_Ard[nIndex].szBiospara[i].szSector;
				break;
		}
	}
	strSector += "|";
	m_strBootSectorFile += strFPath+strBootName + '*';
	m_strBootSectorFile += strSector;
	return 0;
}

/**
 * @brief 	生成非boot固件下载文件列表
 * @param[in]  strFPath    	传入的unzip下具体子目录
 * @param[in]  gfAtt     	   传入的分组信息
 * @param[in]  nIndex  	全局变量Arr_Nlc需要使用到的索引
 * @return
 * @li  0        	成功
 * @li  -1        	失败
*/

int CArdDownload::MakeBiosDownFile_Android(string strFPath,GROUP_FILE_ATT gfAtt,int nIndex)
{
	int nCount = -1;
	string strSectorName, strFullPath;

	m_strBiosDownList = "";

	nCount = Arr_Ard[nIndex].nBinFileCount;
	if (0 == nCount )
		return -1;

	for (int k=0;k<gfAtt.GrAttitude.GroupCount;k++)
	{
		for (int i = 0; i < nCount; i++)
		{
			strSectorName = Arr_Ard[nIndex].szBiospara[i].szSector;
			strFullPath = Arr_Ard[nIndex].szBiospara[i].szFullname;
	
			if (strFullPath == gfAtt.szGroupFile[k])
			{
				m_strBiosDownList += strFPath+strFullPath + '*';
				m_strBiosDownList += strSectorName + '|';
				break;
			}
		}
	}

	return 0;
}

/**
 * @brief 	解析传入的ard文件或list列表
 * @param pszFileName    	传入文件名
 * @return
 * @li  0        	成功
 * @li  -1        	失败
*/
int CArdDownload::ParseFileList(const char* pszFileName)
{
	string strSuffix,strTmp, strRelativePath, strRelativePub ;
	CIniRW ini;
	int nPos = 0,nDownType = 0;
	char szTmp[10] = {0};
	char szFile[MAX_PATH+20] = {0};
	char szFileName[512] = {0};

	strTmp = pszFileName;
	nPos = strTmp.rfind('.'); 
	strSuffix = strTmp.substr(nPos+1);
	m_nSlaveCount = 0;
	InitMultiNlcArr();	
	if((access(pszFileName,F_OK)) == -1)  
	{
		CFunCommon::DlSprintf("Failed:%s not exist,please check!\n",pszFileName);
		return -1;
	}
	transform(strSuffix.begin(), strSuffix.end(), strSuffix.begin(), ::toupper); 
	if("ARD" == strSuffix )
	{
		nDownType = sType_ANDROID_FIRM;
		//解压相对路径还未处理
		if(!UnCompressARD(pszFileName,0))
			return -1;
		snprintf(szFile,sizeof(szFile),"%s/unzip/0/newconfig.ini",g_szAppPath);
		ReadGroupAtt(szFile,0);
		return 0;
	}
	else if("LIST" == strSuffix )
	{
		
		ini.iniFileLoad(pszFileName);
		nDownType = ini.iniGetInt("BootHex","DownType",0);
	}
	if(sType_ANDROID_FIRM != nDownType  )
	{
		CFunCommon::DlSprintf("Failed: download type error,please check list file\n");
		ini.iniFileFree();
		return -1;
	}
	
	snprintf(szFile,sizeof(szFile),"rm -rf %s/unzip/",g_szAppPath);
	CFunCommon::ExecSystem(szFile);

	
	strRelativePath = pszFileName;
	nPos = strRelativePath.rfind('/'); 
	if(nPos != string::npos)
	{
		strRelativePub = strRelativePath.substr(0,nPos+1);
	}
		
	
	memset(szFileName,0,sizeof(szFileName));
	
	ini.iniGetString("BootHex","Master",szFileName,sizeof(szFileName),"");

	if(!strRelativePub.empty())
		snprintf(Arr_Ard[0].szFileName,sizeof(Arr_Ard[0].szFileName),"%s%s",strRelativePub.c_str(),szFileName);
	else
		strcpy(Arr_Ard[0].szFileName,szFileName);
	//CFunCommon::DlSprintf("Arr_Ard[0].szFileName=%s\n",Arr_Ard[0].szFileName);
	m_nSlaveCount  = ini.iniGetInt("BootHex","SlaveCount",0);	
	//CFunCommon::DlSprintf("m_nSlaveCount = %d\n",m_nSlaveCount);
	ini.iniFileFree();
	if(strlen(Arr_Ard[0].szFileName) != 0)
	{
		if((access(Arr_Ard[0].szFileName,F_OK)) == -1)  
		{
			CFunCommon::DlSprintf("Failed: %s not exist,please check!\n",Arr_Ard[0].szFileName);
			ini.iniFileFree();
			return -1;
		}
		strcpy(Arr_Ard[0].szNLCName,Arr_Ard[0].szFileName);
		if(!UnCompressARD(Arr_Ard[0].szFileName,0))
			return -1;
		sprintf(szFile,"%s/unzip/0/newconfig.ini",g_szAppPath);
		ReadGroupAtt(szFile,0);
		
	}
	
	for(int i=1; i<=m_nSlaveCount; i++)
	{
		sprintf(szTmp,"Slave%d",i);
		chdir(g_szAppPath);
		//CFunCommon::DlSprintf("pszFileName=%s\n",pszFileName);
		ini.iniFileLoad(pszFileName);
		memset(szFileName,0,sizeof(szFileName));
		ini.iniGetString("BootHex",szTmp,szFileName,sizeof(szFileName),"");
		if(!strRelativePub.empty())
			snprintf(Arr_Ard[i].szFileName,sizeof(Arr_Ard[i].szFileName),"%s%s",strRelativePub.c_str(),szFileName);
		else
			strcpy(Arr_Ard[i].szFileName,szFileName);

		//CFunCommon::DlSprintf("Arr_Ard[i].szFileName=%s\n",Arr_Ard[i].szFileName);
		if((access(Arr_Ard[i].szFileName,F_OK)) == -1)  
		{
			CFunCommon::DlSprintf("Failed: %s not exist,please check!\n",Arr_Ard[i].szFileName);
			ini.iniFileFree();
			return -1;
		}	

		strcpy(Arr_Ard[i].szNLCName,Arr_Ard[i].szFileName);

		if(!UnCompressARD(Arr_Ard[i].szFileName,i))
			return -1;
		snprintf(szFile,sizeof(szFile),"%s/unzip/%d/newconfig.ini",g_szAppPath,i);
		ReadGroupAtt(szFile,i);
	}
	ini.iniFileFree();
	return 0;
}



/**
 *@brief 	解压ard包
 *@param pszARD    	ARD包
 *@param nIndex    	目录索引，与unzip下的目录对应
 *@return
 *@li  true        	成功
 *@li  false		失败	
*/
bool CArdDownload::UnCompressARD(const char* pszARD,int nIndex)
{

	FILE *fp = NULL;
	FILE *out = NULL;
	char szUnCompressDir[MAX_PATH]={0};
	char szCmd[MAX_PATH + 20]={0};
	char szIni[MAX_PATH]={0};
	int  nLen = 0, nSize = 0,nFileNum = 0;
	struct  timeval  start;
  	struct  timeval  end;
 	unsigned long timer;
	bool bNewArd;	//true zlib算法，false lzss算法
	unsigned int i = 0,j = 0;
	char szFileName[255][MAX_PATH] = {0};
	
  	gettimeofday(&start,NULL);
  
	if (NULL == pszARD)
			return false;
	
	if (NULL == (fp=fopen(pszARD,"rb")) )
	{
		CFunCommon::DlSprintf("Failed: invalid File:%s,Please Check!",pszARD);
		return false;
	}
	fclose(fp);
	
	
	snprintf(szUnCompressDir,sizeof(szUnCompressDir),"%s/unzip/%d",g_szAppPath,nIndex);
	snprintf(szCmd,sizeof(szCmd),"rm -rf %s/unzip/%d",g_szAppPath,nIndex);
	CFunCommon::ExecSystem(szCmd);
	snprintf(szCmd,sizeof(szCmd),"mkdir -p %s/unzip/%d",g_szAppPath,nIndex);
	CFunCommon::ExecSystem(szCmd);

	char szFilehead[MAX_PATH] = {0};	
	if (NULL == (fp = fopen(pszARD, "rb")) ) 
	{
		CFunCommon::DlSprintf("Failed to open %s.\n",pszARD);
		return false;
	}

	fread(szFilehead, sizeof(unsigned char), 4, fp);
	if (memcmp(szFilehead, "\xAB\xCD\xEF\xAF", 4) == 0) 
	{
		bNewArd = false;
	}
	else if(memcmp(szFilehead, "\xAB\xAB\xAB\xAB", 4) == 0)
	{
		bNewArd = true;
	}
	else
	{
		CFunCommon::DlSprintf("Failed: File contents are illegal\n");
		fclose(fp);
		return false;
	}	
	chdir(szUnCompressDir);

	char szStatus[512] = {0};
	sprintf(szStatus,"Parsing %s ......",pszARD);
	(*gCallBack)(szStatus,-1);
	//CFunCommon::DlSprintf("Parsing %s ......",pszARD);
	fflush(stdout);
	fread(szFilehead, sizeof(unsigned char), 1, fp);

	unsigned int unPreLen = 0;
	unsigned int unTotalLen = 0;
	unsigned int unReadLen = 0;
	unsigned int unFileLen[255] = {0};
	unsigned long unDest = 0;
	unsigned char uszDigest[20] = {0};
	unsigned char uszPrebuf[16384*16] = {0};
	unsigned char uszTmpbuf[16384*16] = {0};
	int nCount = 0;
	if (bNewArd)
	{
		j += 4;
		j += 1;
		nCount = szFilehead[0];
		union hash_state md;
		sha1_init(&md);
		memset(&szFileName,0,sizeof(szFileName));
		for (i=0; i<nCount; i++) 
		{
			fread(szFilehead, sizeof(unsigned char), 1, fp);
			j += 1;
			nLen = szFilehead[0];
			fread(szFilehead, sizeof(unsigned char), nLen, fp);
			j += nLen;
			szFilehead[nLen] = 0;
			strcpy(szFileName[i],szFilehead);
			fread(&nLen, sizeof(unsigned int), 1, fp);
			j += sizeof(unsigned int);
			unFileLen[i] = nLen;		
		}
		for (i=0; i<nCount; i++)
		{

			if(NULL == (out = fopen(szFileName[i],"wb")))
			{
				CFunCommon::DlSprintf("Failed to Oping %s\n.", szFileName[i]);
				fclose(fp);
				return false;
			}
			unTotalLen = unFileLen[i];
			unReadLen = 0;
			while (1)
			{
				if (unTotalLen == unReadLen)
				{
					fclose(out);
					break;
				}

				if (unReadLen > unTotalLen)
				{
					CFunCommon::DlSprintf("Failed to reading  ard ");
					goto UnCompressARD_END;
				}
				if(fread(&unPreLen,1,sizeof(unsigned int),fp) != sizeof(unsigned int))
				{
					CFunCommon::DlSprintf("Failed to reading  ard1 \n.");
					goto UnCompressARD_END;
				}
				unReadLen += sizeof(unsigned int);

				nLen = fread(uszTmpbuf,1,unPreLen,fp);
				if(nLen != unPreLen)
				{
					CFunCommon::DlSprintf("Failed to reading  ard failed2.\n");
					goto UnCompressARD_END;
				}
				unReadLen += nLen;

				unDest = sizeof(uszPrebuf);
				if (uncompress(uszPrebuf,&unDest,uszTmpbuf,nLen) != Z_OK)
				{
					CFunCommon::DlSprintf("Failed to uncompress file.\n",0);
					goto UnCompressARD_END;
				}
				if(unDest <= 0)
				{
					CFunCommon::DlSprintf("Failed to uncompress file.\n");
					goto UnCompressARD_END;
				}

				if(fwrite(uszPrebuf,1,(unsigned)unDest,out) != unDest)
				{
					CFunCommon::DlSprintf("Failed to Writing compress file.\n",0);
					goto UnCompressARD_END;
				}
				sha1_process(&md, uszPrebuf, unDest);

			}
		}
		sha1_done(&md,uszDigest);

		if (fread(uszTmpbuf,1,20,fp) != 20)
		{
			CFunCommon::DlSprintf("Failed to reading ard.\n",0);
			fclose(fp);
			return false;

		}
		fclose(fp);
		if (memcmp(uszDigest, uszTmpbuf, 20) != 0) 
		{
			CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[17]);
			return false;
		}
		
	}
	else
	{
		
		nFileNum = szFilehead[0];
		nCount = nFileNum;
		FILE ** fouts = new PFILE[nCount];
		unsigned int* oulen = new unsigned int[nCount];
	
		memset(fouts, 0x00, nCount);
		for (i = 0 ; i < nCount; i ++) 
		{
			fread(szFilehead, sizeof(unsigned char), 1, fp);
			nLen = szFilehead[0];	
			fread(szFilehead, sizeof(unsigned char), nLen, fp);
			szFilehead[nLen] = 0;
			fread(&nLen, sizeof(unsigned int), 1, fp);
			oulen[i] = nLen;
			fouts[i] = fopen((char *)szFilehead, "wb");
		}

		nLen = ftell(fp);
		fseek(fp, 0, SEEK_END);
		nSize = ftell(fp) - nLen - 20;
		fseek(fp, nLen, SEEK_SET);
		CLzss lzss;
		lzss.DeCompress(fp, nSize,fouts, oulen, nFileNum);	
		for (i = 0 ; i < nCount; i ++) 
			fclose(fouts[i]);	

		fseek(fp, nLen+nSize, SEEK_SET);
		unsigned char uszDigest[20];
		fread(uszDigest, sizeof(unsigned char), 20, fp);
		fclose(fp);
		delete[] fouts;
		delete[] oulen;
		if (memcmp(uszDigest, lzss.GetDigest(), 20) != 0) 
		{
			CFunCommon::DlSprintf("Failed: File validation failed\n");
			return false;
		}
	}
	snprintf(szIni,sizeof(szIni),"%s/newconfig.ini",szUnCompressDir);
	if(access(szIni,F_OK) == 0)
	{
		gettimeofday(&end,NULL);	
	//	CFunCommon::DlSprintf("[ %us ]\n",end.tv_sec-start.tv_sec);
		return true;
	}
UnCompressARD_END:
	fclose(fp);
	fclose(out);	
	return false;
}


/**
 * @brief 	下载日志
 * @param pszLogs    	日志内容
 * @return void
*/
void CArdDownload::LogFirm(const char* pszLogs)
{
	string strLogpath,strLogFile,strCmd;
	char date[256] = {0};
	struct tm *t;
    time_t tt;
    time(&tt);
    t = localtime(&tt);
	FILE *fp = NULL;
	if(0 == m_bWriteLog)
	{
		return;
	}

	sprintf(date,"%4d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
	strLogpath = g_szAppPath;
	strLogpath += "/log/";
	strLogpath += date;
	if(access(strLogpath.c_str(),F_OK) != 0)
	{
		strCmd = "mkdir -p " ;
		strCmd += strLogpath.c_str();
		CFunCommon::ExecSystem((char *)strCmd.c_str());		
	}
	strLogFile = strLogpath + "/android_firm.txt";
	fp=fopen(strLogFile.c_str(),"a+");
	if (fp == NULL)
	{
		CFunCommon::DlSprintf("Failed to create %s:%s\n",strLogFile.c_str(),strerror(errno)); 
		return;
	}
	sprintf(date,"%02d:%02d:%02d ",t->tm_hour, t->tm_min, t->tm_sec);
	fwrite(date,strlen(date),1,fp);
	fwrite(pszLogs,strlen(pszLogs),1,fp);
	fwrite("\n",1,1,fp);
	fclose(fp);
	return;
}	


/**
 * @brief 	从para.ini读取是否写日志标志 m_bWriteLog
 * @return void
*/
void CArdDownload::GetLogTag(void)
{
	CIniRW ini;
	char szBuf[MAX_PATH + 20]={0};
	snprintf(szBuf,sizeof(szBuf),"%s/%s",g_szAppPath,"para.ini");
	if((access(szBuf,F_OK)) == -1)  
	{
		CFunCommon::DlSprintf("Failed: para.ini not exist.\n");
		return ;
	}
	ini.iniFileLoad(szBuf);
	m_bWriteLog = ini.iniGetInt("PARA","AZGJLOG",-1);
	ini.iniFileFree();
	return;
}


/**
 * @brief 	获取调用fastboot命令返回的信息
 * @param[in]   fastboot命令参数
 * @param[out]  fastboot返回信息
 * @return
 * @li  0        	成功
 * @li  <0			失败
*/
int CArdDownload::GetExecSystem(const char *in,char *out)
{
	FILE *fp = NULL;
	int nSize = 0;
	char szTxtPath[MAX_PATH + 20]={0};
	char szBuff[1024]={0};	
	char szContent[1024]={0};
	int nPos = -1;
	snprintf(szTxtPath,sizeof(szTxtPath),"%s/unzip/1.txt",g_szAppPath);
	if (NULL == (fp = fopen(szTxtPath, "rb"))) 
	{
		CFunCommon::DlSprintf("Failed to open file %s\n",szTxtPath);
		return -1;
	}
	fseek(fp, 0L, SEEK_END);  
    nSize = ftell(fp);  
	if(0 == nSize)
	{
		CFunCommon::DlSprintf("Failed:%s is empty.\n",szTxtPath);
		fclose(fp);
		return -2;
	}
	fseek(fp,0L,SEEK_SET);
	//fastboot devices，ubuntu下需要root用户才能运行
	if(strcmp(in,GETDEVICES) == 0)
	{
		while( fgets(szBuff, sizeof(szBuff), fp))
    	{
        	if(strstr(szBuff, "fastboot") != NULL)     
         	{
                memcpy(out,szBuff,7);			
				fclose(fp);
				return 0;
              
        	 }
       
  		  }
		fclose(fp);
		return -1;
	}
	//取boot 版本信息
	if(strcmp(in,GETVERSION) == 0)
	{
		while( fgets(szBuff, sizeof(szBuff), fp))
    	{
			 nPos= CFunCommon::GetIndexOf(szBuff,"version:");
			if(-1 != nPos)
			{
	
				memset(szContent,0,sizeof(szContent));
				memcpy(szContent,szBuff+nPos+strlen("version:"),strlen(szBuff)-strlen("version:"));
				//CFunCommon::DlSprintf("szContent = %s\n",szContent);
				CFunCommon::Trim(szContent);
				strcpy(out,szContent);
				//CFunCommon::DlSprintf("out = %s\n",out);
				fclose(fp);
				return 0;
			}         	 
       
  		 }
		fclose(fp);
		return -1;
	}

	//取pos product
	if(strcmp(in,GETPRODUCT) == 0)
	{
		while( fgets(szBuff, sizeof(szBuff), fp))
    	{
			 nPos= CFunCommon::GetIndexOf(szBuff,GETPRODUCT);
			if(-1 != nPos )
			{
				strcpy(out,szBuff);
				fclose(fp);
				return 0;
			}         	 
  		 }
		fclose(fp);
		return -1;
	}
	//取pos serail
	if(strcmp(in,GETSERIALNO) == 0)
	{
		while( fgets(szBuff, sizeof(szBuff), fp))
    	{
			nPos = CFunCommon::GetIndexOf(szBuff,GETSERIALNO);
			if(-1 != nPos )
			{
				strcpy(out,szBuff);
				fclose(fp);
				return 0;
			}         	 
  		 }
		fclose(fp);
		return -1;
	}
	//取pos secure
	if(strcmp(in,GETSECURE) == 0)
	{
		while( fgets(szBuff, sizeof(szBuff), fp))
    	{
        	
			 nPos= CFunCommon::GetIndexOf(szBuff,GETSECURE);
			if(-1 != nPos)
			{
	
				strcpy(out,szBuff);
				fclose(fp);
				return 0;
			}         	 
  		 }
		fclose(fp);
		return -1;
	}
	//取pos fastboot 下载返回结果
	if(strcmp(in,DOWNLOADBIN) == 0)
	{
		while( fgets(szBuff, sizeof(szBuff), fp))
    	{
        	
			nPos= CFunCommon::GetIndexOf(szBuff,"ERROR:");
			if(-1 != nPos )
			{
				//最新mac系统的提示，但可以下载，过滤
				if(-1 == CFunCommon::GetIndexOf(szBuff,"Couldn't create a device interface iterator"))
				{
					memcpy(out,szBuff+nPos+strlen("ERROR:"),strlen(szBuff)-nPos-strlen("ERROR:"));	
					fclose(fp);
					return -1;
				}
				
			}  

			nPos = CFunCommon::GetIndexOf(szBuff,"FAILED");
			if(-1 != nPos)
			{
				memcpy(out,szBuff+nPos+strlen("FAILED"),strlen(szBuff)-nPos-strlen("FAILED"));	
				fclose(fp);
				return -1;
			}  
			nPos = CFunCommon::GetIndexOf(szBuff,"finished.");
			if(-1 != nPos)
			{
				nPos = CFunCommon::GetIndexOf(szBuff,"time:");
				memcpy(out,szBuff+nPos+strlen("time:"),strlen(szBuff)-nPos-strlen("time:"));
				fclose(fp);
				return 0;
			}  
  		 }
		fclose(fp);
		return -1;
	}
	if(strcmp(in,REBOOTBOOTLOADER) == 0)
	{
		while( fgets(szBuff, sizeof(szBuff), fp))
    	{
			nPos= CFunCommon::GetIndexOf(szBuff,"OKAY");
			if(-1 == nPos)
			{
				strcpy(out,szBuff);
				fclose(fp);
				return 0;
			}         	 
  		 }
		fclose(fp);
		return -1;
	}

	if(strcmp(in,REBOOT) == 0)
	{
		while( fgets(szBuff, sizeof(szBuff), fp))
    	{
			nPos = CFunCommon::GetIndexOf(szBuff,"finished");
			if(-1 == nPos)
			{
				strcpy(out,szBuff);
				fclose(fp);
				return 0;
			}         	 
  		 }
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return -1;
}

/**
 * @brief 	fastboot调用线程
 * @param[in] arg    CArdDownload类this指针
 * @return void
*/
void* CArdDownload::ThreadFunc(void *arg)
{
	CArdDownload *ard = (CArdDownload*)arg;
	CFunCommon::ExecSystem(ard->m_szCmd);
	ard->bExitThread = true;
	
}

/**
 * @brief 	下载
 * @param[in] pszDownFileList    下载文件列表
 * @param[in] nNum    		当前下载的ARD
 * @return
 * @li  0        	成功
 * @li  -1			失败
*/
int CArdDownload::Download(const char *pszDownFileList,int nNum)
{
	char szTotal[4096] = {0},			
		szDownFullPath[50][256] = {0},
		szDownFileName[50][256] = {0},
		szPatitionName[50][32] = {0};
	int nTotalFileCount=0,	
		nLen,nRet=0,	
		i,j,nOffset=0;
	char szCmd[1024]={0};
	char szOut[1204]={0};
	nLen = strlen(pszDownFileList);
	memset(szTotal,0,sizeof(szTotal));	
	strcpy(szTotal,pszDownFileList);
(*gCallBack)("55555555555555",-1);
	for (i=0; i<nLen; i++)  
	{
		if ('*' == szTotal[i])
		{
			szTotal[i] = '\0';
			memset(szDownFullPath[nTotalFileCount],0,256);
			strcpy(szDownFullPath[nTotalFileCount],szTotal+nOffset);
			szTotal[i] = '*';

			for (j=strlen(szDownFullPath[nTotalFileCount])-1; j>0; j--)
			{
				if ('/' == szDownFullPath[nTotalFileCount][j])
				{
					memset(szDownFileName[nTotalFileCount],0,256);
					strcpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1);
					break;
				}
			}

			for (j=i;;j++)
			{
				if ('|' == szTotal[j])
					break;
			}

			memset(szPatitionName[nTotalFileCount],0,32);
			memcpy(szPatitionName[nTotalFileCount],szTotal+i+1,j-i-1);
			nOffset = j+1;			
			nTotalFileCount++;
		}
	}
	//CFunCommon::DlSprintf("Downloading %s\n",Arr_Ard[nNum].szNLCName);  
	
	for (i=0; i<nTotalFileCount; i++)
	{
		if(strstr(szPatitionName[i],"aboot") != NULL || strstr(szPatitionName[i],"uboot"))
		{
			if(1 == m_nAboot)
			{
				//OutputDebugString("continue");
				continue;
			}		
		}
	
		if (0 == m_nErgodic || 1 ==  m_nErgodic|| 2 == m_nErgodic )
		{
			if (!strstr(szPatitionName[i],"userdata") && !strstr(szPatitionName[i],"splash")  && !strstr(szPatitionName[i],"privdata1") )
			{
				continue;
			}
		}
		nRet = -1;
		string strTmp,strTmp1;
		strTmp = szDownFullPath[i];
		int pos = 0;
		pos = strTmp.rfind("/");
		if(pos != string::npos)
			strTmp1 = strTmp.substr(pos+1);
		else
			strTmp1 = strTmp;
		// CFunCommon::DlSprintf("	--> Downloading %s",strTmp1.c_str());

		//估算下载时间
		struct stat stat_buf;
		int iTimes,nProgress; //等待判断时间
		unsigned long lFineLen = 0;
		int j = 0;
		stat(szDownFullPath[i], &stat_buf) ;
		lFineLen = stat_buf.st_size;
		iTimes = lFineLen/(1024*1024*5);  //以5M/s计算
		iTimes += 80;
		pthread_t nThreadId = 0;;
		fflush(stdout);
		sprintf(m_szCmd,"./fastboot flash  %s %s -i %s 2>unzip/1.txt",szPatitionName[i],szDownFullPath[i],PRODUCT_ID);
		bExitThread = false;
		if(pthread_create(&nThreadId, NULL, ThreadFunc, this))
		{
			CFunCommon::DlSprintf("Failed to create thread\n");
			return -1;
		}
		while(!bExitThread)
		{  
			j++;
			if (j >= iTimes)
			{
				CFunCommon::ExecSystem("killall -9 fastboot");
			//	pthread_cancel(nThreadId);
				CFunCommon::DlSprintf("download error,Unknown error");
				return -1;
				
			}
			nProgress = (j*250)/iTimes;
			if (nProgress >= 100)
			{
				nProgress = 100;
			}
			(*gCallBack)(szDownFullPath[i],nProgress);
		
			usleep(1000*1000);
		} 

		//CFunCommon::DlSprintf("szCmd=%s\n",szCmd);
		if(GetExecSystem(DOWNLOADBIN,szOut) != 0)
		{	
			CFunCommon::Trim(szOut);
			CFunCommon::DlSprintf("Failed:%s\n",szOut);
			return -1;
		}
		(*gCallBack)(szDownFullPath[i],100);
	//	CFunCommon::DlSprintf("	[ %s ]\n",szOut);
		memset(szOut,0,sizeof(szOut));
	}
	return 0;
}

/**
 * @brief 	等待设备连接，暂时不用
 * @return
 * @li  0        	成功
 * @li  <0			失败
*/
int CArdDownload::WaitForConnect(void)
{

	char szCmd[512] = {0};
	char szOut[512] = {0};
	int i = 0;
	chdir(g_szAppPath);
	sprintf(szCmd,"./fastboot devices -i %s >unzip/1.txt",PRODUCT_ID);
	for(i=0;i<60;i++)
	{
		CFunCommon::ExecSystem(szCmd);
		if(GetExecSystem(GETDEVICES,szOut)!= 0)
		{	
			memset(szOut,0,sizeof(szOut));
			usleep(1000*1000);
			continue;
		}
		else
			return 0;
		//CFunCommon::DlSprintf("szOut = %s\n",szOut);
		
	}
	return -1;
	
}

/**
 * @brief 	下载流程
 * @param[in] pszFileName    命令行传入的ard或list
 * @return
 * @li  0        	成功
 * @li  -1			失败
*/
int CArdDownload::DownloadProcess(const char *pszFileName)
{

	string strCurFirmPath,strCfgVerFull,strPosVerFull,strCfgVer,strPosVer;
	string strDefinition="";    //pos 返回5个字段判断信息
	string strDefSecond="";		//pos第二字段，模块
	string m_csLog;
	bool bFind = false; //是否有查找到匹配的组
	bool bBoot= false;//对于master表示：是否重启，与this->aboot配合判断aboot是否下载
	bool bNewLogic = false;
	char szCmd[256]={0};
	char szIn[256]={0};
	char szOut[256]={0};
	char szVarous[256]={0};
	int nConfig = 0,nPos = 0,iIndex = -1,i,j,nRet = -1;
	strCurFirmPath = g_szAppPath;
	strCurFirmPath += "/unzip/0/";
	//CFunCommon::DlSprintf("strCurFirmPath = %s\n",strCurFirmPath.c_str());
	m_nAboot = 0;							//是否下载aboot,1不下载，0下载
	string iniDefinit,msg;
	char szVersion[256] = {0};
	char szVariousInfo[512]={0}; 	//pos上送信息,日志使用
	int bi=0;


	if(ParseFileList(pszFileName) != 0)
		return -1;
	//CFunCommon::DlSprintf("Arr_Ard[nIndex].bIsUsed=%d\n",Arr_Ard[0].bIsUsed);
	chdir(g_szAppPath);

#if 0
	sprintf(szCmd,"./fastboot devices -i %s >unzip/1.txt",PRODUCT_ID);
	//CFunCommon::DlSprintf("szCmd=%s\n",szCmd);
	for(int i=0;i<1000;i++)
	{	
		CFunCommon::ExecSystem(szCmd);
		if(GetExecSystem(GETDEVICES,szOut)!= 0)
		{	
			if(i == 0)
				CFunCommon::DlSprintf("Waiting for device connection\n");
			usleep(1000*100);	
		}
		else
		{
			//mac系统识别到正确加载完设备需要时间
			usleep(1000*5000);
				break;
		}
	}

	if(strlen(szOut) == 0)
	{
		CFunCommon::DlSprintf("Please confirm the connection to the Newland device and download again\n");
		return -1;
	}
	#endif
	//CFunCommon::DlSprintf("szOut = %s\n",szOut);
	memset(szOut,0,sizeof(szOut));
	sprintf(szCmd,"./fastboot getvar product -i %s 2>unzip/1.txt",PRODUCT_ID);
	(*gCallBack)(szCmd,-1);
	CFunCommon::ExecSystem(szCmd);

	if(GetExecSystem(GETPRODUCT,szOut)!= 0)
	{	
		CFunCommon::DlSprintf("Failed to get product.\n");
		return -1;
	}
	strcpy(szVarous,szOut); 
//	CFunCommon::DlSprintf("szVarous = %s.\n",szVarous);
	memset(szOut,0,sizeof(szOut));
	sprintf(szCmd,"./fastboot getvar serialno -i %s 2>unzip/1.txt",PRODUCT_ID);
	//CFunCommon::DlSprintf("szCmd=%s\n",szCmd);
	CFunCommon::ExecSystem(szCmd);
	
	if(GetExecSystem(GETSERIALNO,szOut) != 0)
	{	
		CFunCommon::DlSprintf("Failed to get serialno.\n");
		return -1;
	}
	strcat(szVarous,szOut); 
	memset(szOut,0,sizeof(szOut));
	sprintf(szCmd,"./fastboot getvar secure -i %s 2>unzip/1.txt",PRODUCT_ID);
	//CFunCommon::DlSprintf("szCmd=%s\n",szCmd);
	CFunCommon::ExecSystem(szCmd);
	
	if(GetExecSystem(GETSECURE,szOut) != 0)
	{	
		CFunCommon::DlSprintf("Failed to get secure info.\n");
		return -1;
	}
	strcat(szVarous,szOut); 
	memset(szOut,0,sizeof(szOut));
	
	CFunCommon::ReplaceAnsi(szVarous,'\n',' ');
	//CFunCommon::DlSprintf("%s\n",szVarous);
	 if ((strstr(szVersion,".HD") != NULL) || (strstr(szVersion,".QHD") != NULL ))
	 {
		
		//取第三到第八字段
		strDefinition = SplitSubString(szVersion);
		//取第二字段
		strDefSecond = SplitSecondString(szVersion);

	 }
 	sprintf(szCmd,"./fastboot getvar version -i %s 2>unzip/1.txt",PRODUCT_ID);
	//CFunCommon::DlSprintf("szCmd=%s\n",szCmd);
	CFunCommon::ExecSystem(szCmd);
	
	if(GetExecSystem(GETVERSION,szOut)!= 0)
	{	
		CFunCommon::DlSprintf("Failed to get version.\n");
		return -1;
	}
	strcpy(szVersion,szOut);
	// CFunCommon::DlSprintf("POS aboot :%s\n",szVersion);
	memset(szOut,0,sizeof(szOut));
	
	// CFunCommon::DlSprintf("POS INFO :%s\n",szVariousInfo);
	if (Arr_Ard[0].bIsUsed)
	{
		bFind = false;
		bBoot = false;
		iIndex = -1;	
		bNewLogic = false;
		string sector = Arr_Ard[0].szBiospara[0].szSector;
		//这三个分区按新规则，否则还是按旧规则
		//OutputDebugString(tp->sector);
		if (sector == "userdata" || sector == "splash" || sector == "privdata1")
		{
			bNewLogic = true;
		}
		//CFunCommon::DlSprintf("sector = %s\n",sector.c_str());
		
//有返回HD信息，且是目标三个分区
		if (!strDefinition.empty() && bNewLogic)
		{
			for (bi=0;bi<Arr_Ard[0].nGroupCount;bi++)
			{
				iniDefinit="";
				strCfgVerFull=Arr_Ard[0].szGroupFileAtt[bi].GrAttitude.szGroupVer;
				//CFunCommon::DlSprintf("strCfgVerFull = %s\n",strCfgVerFull.c_str());
				if(strCfgVerFull.find(".HD") != string::npos || strCfgVerFull.find(".QHD") != string::npos )
				{
					iniDefinit = SplitSubString(strCfgVerFull);
				}
			
				if (iniDefinit == strDefinition)
				{
					break;
				}
			}
		
		}
	
		for (i=bi;i<Arr_Ard[0].nGroupCount;i++)
		{
			strCfgVerFull = Arr_Ard[0].szGroupFileAtt[i].GrAttitude.szGroupVer;
			//CFunCommon::DlSprintf("strCfgVerFull = %s\n",strCfgVerFull.c_str());
			strPosVerFull = szVersion;
			nConfig = strCfgVerFull.find("*");
			if (nConfig > 0)
				strCfgVer = strCfgVerFull.substr(0,nConfig);
			else
				strCfgVer = strCfgVerFull;

		//	CFunCommon::DlSprintf("strCfgVer = %s\n",strCfgVer.c_str());
			nPos = strPosVerFull.find("*");
			if (nPos > 0)
				strPosVer = strPosVerFull.substr(0,nPos);
			else
				strPosVer = strPosVerFull;
			//CFunCommon::DlSprintf("strPosVer = %s\n",strPosVer.c_str());
			
			if (!strDefinition.empty() && bNewLogic)
			{
				bFind = true;
				iIndex=i;
			//获取版本信息
				if (nConfig>0)
					//strCfgVer=strCfgVerFull.Right(strCfgVerFull.GetLength()-nConfig-1);
					strCfgVer = strCfgVerFull.substr(nConfig+1);
				if (nPos > 0)
					//strPosVer=strPosVerFull.Right(strPosVerFull.GetLength()-nPos-1);
					strPosVer = strPosVerFull.substr(nPos+1);
				nConfig = strCfgVer.find("_T",0);
				if (nConfig > 0)
					strCfgVer = strCfgVer.substr(0,nConfig);
				nPos = strPosVer.find("_T",0);
				if (nPos > 0)
					strPosVer = strPosVer.substr(nPos);
			//	CFunCommon::DlSprintf("strCfgVer = %s\n",strCfgVer.c_str());
			//	CFunCommon::DlSprintf("strPosVer = %s\n",strPosVer.c_str());
				if (atoi(strPosVer.c_str())>atoi(strCfgVer.c_str()))  
				{
				
					bBoot = false;
					m_nAboot = 1;
					break;
				}
				else if (atoi(strPosVer.c_str())<=atoi(strCfgVer.c_str()))
				{
				
					if(strPosVer == strCfgVer)
					{
						bBoot = false;
					}
					else
					{
						bBoot = true;
					}
					m_nAboot = 0;
					break;
				}
				else
				{
					break;
				}
			}
	
			//pos未返回分辨率信息的，通过匹配机型判断
			else if (strCfgVer == strPosVer)   
			{
			//	CFunCommon::DlSprintf("strCfgVer = %s\n",strCfgVer.c_str());
				bFind = true;
				iIndex = i;
			//获取版本信息
				if (nConfig > 0)
				{
					strCfgVer = strCfgVerFull.substr(nConfig+1);
				}
					
				if (nPos > 0)
				{
					strPosVer = strPosVerFull.substr(nPos+1);
				}
					
				nConfig = strCfgVer.find("_T",0);
				if (nConfig > 0)
				{
					strCfgVer = strCfgVer.substr(nConfig+1);
				}
					
				nPos = strPosVer.find("_T",0);
				if (nPos > 0)
				{
					strPosVer = strPosVer.substr(0,nPos+1);
				}
					

				//CFunCommon::DlSprintf("strCfgVer = %s\n",strCfgVer.c_str());
				//CFunCommon::DlSprintf("strPosVer = %s\n",strPosVer.c_str());
					//pos版本比固件包的版本高，不下载aboot，不重启
				
				if (atoi(strPosVer.c_str())>atoi(strCfgVer.c_str()))  
				{
					//CFunCommon::DlSprintf("pos > config\n");
					bBoot = false;
					m_nAboot = 1;
					break;
				}
				else if (atoi(strPosVer.c_str()) <= atoi(strCfgVer.c_str()))
				{
					if(atoi(strPosVer.c_str()) == atoi(strCfgVer.c_str()))
					{
						bBoot = false;
					}
					else
					{
						bBoot = true;
					}
				
					m_nAboot = 0;
					break;
				}
				else
				{
					break;
				}
			}	
			

		}  	


		if (!bFind)  //如果通过分辨率匹配5字段，与匹配机器两种情况都不匹配，提示退出
		{		
			CFunCommon::DlSprintf("Failed:No matching model found.\n");
			return -1;
		}
		
		//当pos版本小于配置，且newconfig.ini中所在组有配置boot=1，下载aboot，并重启bootloader，
		if ((Arr_Ard[0].szGroupFileAtt[i].GrAttitude.iBoot == 1) && bBoot) 
		{
			nRet = MakeBootDownFile_Android(strCurFirmPath,Arr_Ard[0].szGroupFileAtt[iIndex].szGroupFile[0],0);
			if (0 != nRet)
			{			
				return -1;
			}
		//	CFunCommon::DlSprintf("m_strBootSectorFile = %s\n",m_strBootSectorFile.c_str());
	 		
			if (Download((char *)m_strBootSectorFile.c_str(),0) != 0)
			{
				return -1;
			}	
			bBoot = false;
			m_csLog += "->";m_csLog += strCfgVerFull;m_csLog += ":";
			LogFirm((char *)m_csLog.c_str());
			sprintf(szCmd,"./fastboot reboot-bootloader -i %s 2>unzip/1.txt",PRODUCT_ID);
			//CFunCommon::DlSprintf("szCmd=%s\n",szCmd);
			CFunCommon::ExecSystem(szCmd);
			if(GetExecSystem(REBOOTBOOTLOADER,szOut) != 0)
			{	
				CFunCommon::DlSprintf("Failed:reboot-bootloader\n");
				return -1;
			}
			memset(szOut,0,sizeof(szOut));
			usleep(1000*1000);
			//CFunCommon::DlSprintf("Download aboot success!!!\n");
			
			return 2; 
			
		}

		
		//不重启bootloader,下载，
		//当pos版本小于配置，且newconfig.ini中所在组有配置boot=0，下载（aboot，与其它分区），不重启bootloader
		//当pos版本大于配置，m_nAboot=1,不下载aboot，下载其他分区，不重启bootloader
		//当pos版本等于配置，m_nAboot=0,下载（aboot，与其它分区），不重启bootloader
		if (((Arr_Ard[0].szGroupFileAtt[i].GrAttitude.iBoot == 0) && bBoot)|| !bBoot )
		{
			MakeBiosDownFile_Android(strCurFirmPath,Arr_Ard[0].szGroupFileAtt[iIndex],0);
			//CFunCommon::DlSprintf("m_strBiosDownList = %s\n",m_strBiosDownList.c_str());
			if (Download((char *)m_strBiosDownList.c_str(),0) != 0)
			{
				return -1;
			}
			m_csLog += "->";m_csLog += strCfgVerFull;m_csLog += ":";
			LogFirm((char *)m_csLog.c_str());
			//LogFirm(m_csLog.c_str());
		}
	}

	m_nAboot = 0;
	char szTmp[20] = {0};
	//CFunCommon::DlSprintf("m_nSlaveCount = %d\n",m_nSlaveCount);
	for (j=1; j<=m_nSlaveCount; j++)  //此处用于后续的多个NLC的下载，此处的下载流程均不发重载的命令，直到最后的下载结束后再发重启命令
	{
	
		sprintf(szTmp,"/unzip/%d/",j);
		strCurFirmPath = g_szAppPath;
		strCurFirmPath += szTmp;
		m_nErgodic = -1;
		bNewLogic = false;
		string strSector = Arr_Ard[j].szBiospara[j].szSector;
		//CFunCommon::DlSprintf("strCurFirmPath = %s\n",strCurFirmPath.c_str());
		//这三个分区按新规则，否则还是按旧规则
		if ("userdata" == strSector || "splash" == strSector || "privdata1" == strSector)
		{
			bNewLogic = true;
		}
		
		if (Arr_Ard[j].bIsUsed)
		{
		
			bFind = false;
			iIndex = -1 ;
			bi = 0;
				//有返回HD信息，且是目标三个分区
			if (!strDefinition.empty() && bNewLogic)
			{
				iniDefinit = "";
				for (bi=0; bi<Arr_Ard[j].nGroupCount; bi++)
				{
					strCfgVerFull = Arr_Ard[j].szGroupFileAtt[bi].GrAttitude.szGroupVer;
					if(strCfgVerFull.find(".HD") != string::npos || strCfgVerFull.find(".QHD") != string::npos )
					{
						iniDefinit = SplitSubString(strCfgVerFull);
					}
					if (iniDefinit == strDefinition)
					{
							m_nErgodic = 0;
							break;
					}	
				}
				//这段只是日志使用
				if (bi == Arr_Ard[j].nGroupCount)
				{
					bi = 0;
					for (bi=0; bi<Arr_Ard[j].nGroupCount; bi++)
					{
						strCfgVerFull = Arr_Ard[j].szGroupFileAtt[bi].GrAttitude.szGroupVer;	
						iniDefinit = SplitSecondString(strCfgVerFull);
						if (iniDefinit == strDefSecond)
						{
							m_nErgodic = 1;
							break;
						}	
					}
				}
		
				if (bi == Arr_Ard[j].nGroupCount)
				{
					bi = 0;
					for (bi=0; bi<Arr_Ard[j].nGroupCount; bi++)
					{
						strCfgVerFull = Arr_Ard[j].szGroupFileAtt[bi].GrAttitude.szGroupVer;	
						
						if (strCfgVerFull.find("QUECTEL") != string::npos)
						{
							m_nErgodic = 2;
							break;
						}	
					}
				}
			}
		
			for (i=bi; i<Arr_Ard[j].nGroupCount; i++)
			{
			
				strCfgVerFull = Arr_Ard[j].szGroupFileAtt[i].GrAttitude.szGroupVer;
				strPosVerFull = szVersion;
				nConfig = strCfgVerFull.find("*");
				if (nConfig > 0)
				{
					strCfgVer = strCfgVerFull.substr(0,nConfig);
				}
				
				else
				{
					strCfgVer = strCfgVerFull;
				}
				
				nPos = strPosVerFull.find("*");
				if (nPos > 0)
				{
					strPosVer=strPosVerFull.substr(0,nPos);
				}
					
				else
				{
					strPosVer = strPosVerFull;
				}
					


				if (!strDefinition.empty() && bNewLogic)
				{
					
					//OutputDebugString(msg);
					//OutputDebugString("m_nSlaveCount");
					//OutputDebugString(strDefinition);
					bFind = true;
					iIndex = i;
			
					
					strCfgVer = strCfgVerFull.substr(nConfig+1);
					strPosVer = strPosVerFull.substr(nPos+1);
					
					nConfig = strCfgVer.find("_T",0);
					if (nConfig > 0)
					{
						strCfgVer = strCfgVer.substr(nConfig+1);
					}
						
					nPos = strPosVer.find("_T",0);
					if (nPos > 0)
					{
						strPosVer = strPosVer.substr(nPos+1);
					}
						
					//CFunCommon::DlSprintf("j=%d strCfgVer = %s\n",j,strCfgVer.c_str());
					//CFunCommon::DlSprintf("j=%d strPosVer = %s\n",j,strPosVer.c_str());
					if (atoi(strPosVer.c_str())>atoi(strCfgVer.c_str()))    
					{
						m_nAboot = 1;
					}
					break;
				}
				
				//先匹配到对应的机型和相关信息
				else if (strCfgVer == strPosVer)   
				{
					bFind = true;
					iIndex = i;
						//获取版本信息
					strCfgVer = strCfgVerFull.substr(nConfig+1);
					strPosVer = strPosVerFull.substr(nPos+1);
					
					
					nConfig = strCfgVer.find("_T",0);
					if (nConfig > 0)
					{
						strCfgVer = strCfgVer.substr(nConfig+1);
					}
						
					nPos = strPosVer.find("_T",0);
					if (nPos > 0)
					{
						strPosVer = strPosVer.substr(nPos+1);
					}
						
					if (atoi(strPosVer.c_str())>atoi(strCfgVer.c_str()))   //pos版本比固件包的版本高，不下载aboot
					{
						m_nAboot = 1;
					}
					break;
				}
				
			}  
	
			if (!bFind)  
			{				
				CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[39]);
				return -1;
			}
		
			MakeBiosDownFile_Android(strCurFirmPath,Arr_Ard[j].szGroupFileAtt[iIndex],j);	
		//	CFunCommon::DlSprintf("m_strBiosDownList=%s\n",(char *)m_strBiosDownList.c_str());
			if (Download((char *)m_strBiosDownList.c_str(),j)!=0)
			{
					return -1;
			}
			m_csLog +="->";m_csLog+= strCfgVerFull;m_csLog+=":";
			LogFirm((char *)m_csLog.c_str());
			
									
		}
		
	}
	sprintf(szCmd,"./fastboot reboot -i %s 2>unzip/1.txt",PRODUCT_ID);
	//CFunCommon::DlSprintf("szCmd=%s\n",szCmd);
	CFunCommon::ExecSystem(szCmd);
	if(GetExecSystem(REBOOT,szOut) != 0)
	{	
		CFunCommon::DlSprintf("Failed:reboot error\n");
		return -1;
	}
	memset(szOut,0,sizeof(szOut));
	sprintf(szCmd," rm -rf %s/unzip/*",g_szAppPath);
	CFunCommon::ExecSystem(szCmd);
	CFunCommon::DlSprintf("%s\n",DOWN_SUCC);
	return 0;
}



