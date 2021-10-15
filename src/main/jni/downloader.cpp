/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	downloader.cpp
 * @brief	命令行下载工具主程序入口
 * @version	1.0
 * @author: ym
 * @date	2021/04/3
 ******************************************************************************/
#include "pubdef.h"
#include "downloader.h"
#include "function.h"
#include "arddownload.h"
#include "nlpdownload.h"
#include "nlcdownload.h"
#include "inirw.h"
#include "nlddownload.h"
#include "nld.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h> 
#include <signal.h>  
#include <sys/wait.h>  
#include <libgen.h>
#include <stdio.h>


#define DOWNLOSD_VER  "Downaloder v0.1\nAll Rights Reserved (C) 2021 Fujian Newland Payment Product&Technology Department.\n"

char *Usage = "Usage:\n\
	-v  View version information\n\
	-a  Download Application\n\
	-f  Download Firmware\n\
	-c  Clear all Application on POS\n\
	-d  Device eg. windows:COMx;linux:ttyACMx/ttyUSBx;mac:tty.usbmodemxx\n\
	-l  Get App list\n\	
	-del Delete specified app(Multiple files using '|' connection) \n\
\n\nExample:\n\n\
 Download Application\n\	
 	Linux POS:	dl -a Test.nld/Test.list -d ttyACM0 (-c)\n\
	Android POS:	dl -a Test.apk/Test.list -d ttyACM0 (-c)\n\
	RTOS POS:	dl -a Test.nlp/Test.list -d ttyACM0 (-c)\n\
\n Download Firmware\n\
	Linux POS:	dl -f Test.nlc/Test.list -d ttyACM0 \n\
	Android POS:	dl -f Test.ard/Test.list (root permission is required) \n\
	RTOS POS:	dl -f Test.nlp/Test.list -d ttyACM0\n\
\nGet App List\n\ 	
	Linux POS:	dl -l linux -d ttyACM0 \n\	
	Android POS:	dl -l android -d ttyACM0\n\
\n Delete App \n\	
	Linux POS:	dl -del_linux appName1|appName2 -d ttyACM0 \n\	
	Android POS:	dl -del_android appName1|appName2 -d ttyACM0\n";

char g_szAppPath[1024] = {0}; 			////<下载工具的绝对路径	
char g_szMessage[MSGSUM][100];			///<下载错误信息，从配置文件读取，用于所有平台

char g_szResult[1024*5];					///<下载接口输出结果全局变量 
bool g_bConnect = false;			///设备是否断开连接

static string g_strFilelist;
enum DOWN_TYPE {
    DOWN_APP,          //下载应用  
    DOWN_FIRM,    		 //下载固件             
    DOWN_GETAPP,        //获取应用列表
    DOWN_DELAPP         //删除应用
}DOWN_TYPE;

enum POS_PLAT{
    PLAT_ANDROID,       //智能设备   
    PLAT_LINUX,         //linux设备
    PLAT_MPOS           //mpos设备
}POS_PLAT;


void handle(int sig)  
{  
    if (sig == SIGCHLD)  
    {  
        int pid;  
        int status;  
        //CFunCommon::DlSprintf("recv SIGCHLD\n");  
        while((pid = waitpid(-1, &status, WNOHANG)) > 0)  
        {  
        }  
    }  
} 





/**
 *@brief 根据命令传入参数生成下载文件列表（命令行参数的包名或list中配置的包名或文件名，不包括解析后的文件名，例如NLC,ARD，OTA中解压后的文件名，这些文件名在各模块业务处理类或通讯类中解析）
 *@param[in] pszFileName    命令行传入的包文件或list
 *@param[in] bList   	 是否list文件
 *@param[in] nDownType   下载类型
 *@return
 *@li 0            成功
 *@li 其它          失败
*/
int MakeDownFileString(char* pszFileName,bool bList,int nDownType)  
{
	int iRult = 0;
	int nPos = -1;
	char szBuff[128] = {0};
	char szFile[MAX_PATH]={0};
	U32 nAppTotalSize = 0;
	//struct stat stat_buf;
	char szAppInfo[4096*10] = {0};
	string szRelativePath; //list文件全路径
	string szRelativePub;	//list路径（不包含文件名）
	BOOL bFind = false;
	FILE *f1 = NULL;
	FILE *fp = NULL;
	string strConver,strRight,strLeft;

	ST_DOWNFILE_ATT downfileAtt[MAX_DOWNFILE_NUM]; //最多支持200个应用程序下载
	g_strFilelist = "";

	if(bList)
	{
		
		szRelativePath = pszFileName;
		nPos = szRelativePath.rfind('/'); 
		if(nPos != string::npos)
			szRelativePub = szRelativePath.substr(0,nPos+1);
	}


	//RTOS NLP下载
	if (sType_MPOS_APP == nDownType || sType_MPOS_FIRM == nDownType)
	{
		//2种加载方式，，list，nlp	
		if(!bList)
		{
			g_strFilelist = pszFileName;
			//判断是否NLP格式文件
			if (NULL == (f1 = fopen(pszFileName, "rb")) ) 
			{
				CFunCommon::DlSprintf("Failed to open file %s\n",pszFileName);
				return -2;
			}
			memset(szBuff,0,sizeof(szBuff));
			fread(szBuff, 1, 8, f1);	
			if (memcmp(szBuff, "Scorpiop", 8)) 
			{
		 		fclose(f1);
				CFunCommon::DlSprintf("Failed:%s is not a valid NLP file!",pszFileName);
				return -1;
			}
			memset(szBuff,0,sizeof(szBuff));
			fread(szBuff, 1, 8, f1);
			if (0x01 != szBuff[4] && 0x00 != szBuff[4] && 0x02 != szBuff[4]) 
			{
				fclose(f1);
				CFunCommon::DlSprintf("Failed:%s is not a valid NLP file!",pszFileName);
				return -1;						
			}
			//NLP类型
			if (szBuff[4] == 0x01) 
					g_strFilelist += "*1|";
			else if (szBuff[4] == 0x00) 
					g_strFilelist += "*0|";
			else if (szBuff[4] == 0x02) //匹配x509的证书格式0：表示无证书，1：表示312长度的证书，2：x509证书
					g_strFilelist += "*2|";
			fclose(f1);
			return 0;
						
		}
		else //list
		{
			//解析应用配置的list文件
			if(sType_MPOS_APP == nDownType)
			{
				if (NULL == (fp = fopen(pszFileName, "rb"))  ) 
				{
					CFunCommon::DlSprintf("Failed to open file %s\n",pszFileName);
					return -2;
				}
				while( fgets(szBuff, sizeof(szBuff), fp))
				{
					if(strstr(szBuff, "[FileList]") != NULL)     
					{
						bFind = true;
						continue;
					}
					if(bFind)
					{
						
						if(strstr(szBuff, "[") != NULL)  
							break;
					
						strConver = szBuff;
						nPos = strConver.find('=');  
						if (nPos == string::npos)  	
							continue;  
						strRight = strConver.substr(nPos+1);
						strLeft = strConver.substr(0,nPos);
						CFunCommon::DeleteIndexString(strLeft," ");
						if(!szRelativePub.empty())
							strLeft = szRelativePub + strLeft;
						//CFunCommon::DlSprintf("strLeft=%s\n",strLeft.c_str());
						//nlp类型文件
						if(strLeft.find(".nlp")!= string::npos || strLeft.find(".NLP")!= string::npos )
						{
							g_strFilelist += strLeft;
							if (NULL == (f1 = fopen(strLeft.c_str(), "rb")) ) 
							{
								CFunCommon::DlSprintf("Failed to open file %s\n",strLeft.c_str());
								return -2;
							}
							memset(szBuff,0,sizeof(szBuff));
							fread(szBuff, 1, 8, f1);	
							if (memcmp(szBuff, "Scorpiop", 8)) 
							{
								fclose(f1);
								CFunCommon::DlSprintf("Failed:%s is not a valid NLP file!\n",strLeft.c_str());
								return -1;
							}
							memset(szBuff,0,sizeof(szBuff));
							fread(szBuff, 1, 8, f1);
							if (0x01 != szBuff[4]   && 0x00 != szBuff[4]   && 0x02 != szBuff[4]) 
							{
								fclose(f1);
								CFunCommon::DlSprintf("Failed:%s is not a valid NLP file!\n",strLeft.c_str());
								return -1;						
							}
							if (szBuff[4] == 0x01) 
								g_strFilelist += "*1|";
							else if (szBuff[4] == 0x00) 
								g_strFilelist += "*0|";
							else if (szBuff[4] == 0x02) 
								g_strFilelist += "*2|";
							fclose(f1);
						}
						else //非NLP文件
						{
							g_strFilelist += strLeft;
							g_strFilelist += "*8|";
						}
							
					}
		
				}
				fclose(fp);
				//CFunCommon::DlSprintf("g_strFilelist = %s\n",g_strFilelist.c_str());
				return 0;
							
			}
			
			else
			{ //解析固件配置的list
				CIniRW ini;
				char szTmp[256] = {0};
				int nType=0,nCount=0;

				
				ini.iniFileLoad(pszFileName);
				nType = ini.iniGetInt("BootHex","DownType",0);
				if(5 != nType)
				{
					CFunCommon::DlSprintf("Failed:DownType error,please check!\n");
					return -1;
				}
				nCount = ini.iniGetInt("BootHex","SlaveCount",0); 
				memset(szTmp,0,sizeof(szTmp));
				ini.iniGetString("BootHex","Master",szTmp,sizeof(szTmp),"");
				if(strlen(szTmp) > 0)
				{
					//list 里面配置的是相对路径
					if(szTmp[0] != '/')
					{
						g_strFilelist += szRelativePub;
					}
					g_strFilelist += szTmp;
					g_strFilelist += "*1|";
				}
			//	CFunCommon::DlSprintf("g_strFilelist=%s\n",g_strFilelist.c_str());
				for(int i=0; i<nCount; i++)
				{
					char szSlave[10] = {0};
					sprintf(szSlave,"Slave%d",i+1);
					memset(szTmp,0,sizeof(szTmp));
					ini.iniGetString("BootHex",szSlave,szTmp,sizeof(szTmp),"");
					if(strlen(szTmp) > 0)
					{
						//list 里面配置的是相对路径
						if(szTmp[0] != '/')
						{	
							g_strFilelist += szRelativePub;			
						}
						g_strFilelist += szTmp;
						g_strFilelist += "*1|";
					}
					//	CFunCommon::DlSprintf("g_strFilelist=%s\n",g_strFilelist.c_str());
				}
				ini.iniFileFree();
				

			}	
		}
		return 0;
	}

   //单个NLD
	if(sType_App == nDownType )
	{	
		g_strFilelist = pszFileName;
		g_strFilelist += "*0|";
		return 0;
	}


	
	//list 加载单应用nld
	if(sType_NLD == nDownType)
	{
		

		int noNldFile = 0;
		string strMainNld;
		char tmpNld[MAX_PATH] = {0};
		if (NULL == (fp = fopen(pszFileName, "rb"))) 
		{
				CFunCommon::DlSprintf("Failed to open file %s\n",pszFileName);
				return -2;
		}
		while(fgets(szBuff, sizeof(szBuff), fp))
    	{
    		
        	if(strstr(szBuff, "[FileList]") != NULL)     
         	{
         		bFind = true;
				 continue;
        	}
			if(bFind)
			{		
				if(strstr(szBuff, "[") != NULL)  
					break;	
				strConver = szBuff;
				nPos = strConver.find('=');  
				if (nPos == string::npos)  	
					continue;  
				strRight = strConver.substr(nPos+1);
				strLeft = strConver.substr(0,nPos);
				CFunCommon::DeleteIndexString(strLeft," ");
				if(!szRelativePub.empty())
					strLeft = szRelativePub + strLeft;
				//CFunCommon::DlSprintf("strLeft=%s\n",strLeft.c_str());
				//CFunCommon::DlSprintf("strRight=%s\n",strRight.c_str());		
				if(access(strLeft.c_str(),F_OK) != 0)
				{	
					CFunCommon::DlSprintf("Failed to open file %s\n",strLeft.c_str());
					return -2;
				}
				if(strLeft.find(".nld")!= string::npos || strLeft.find(".NLD")!= string::npos )
				{
					g_strFilelist += strLeft;				
					if(strRight.find("Main NLD")!= string::npos || strRight.find("主程序")!= string::npos )
					{
						strMainNld = strLeft;
						//CFunCommon::DlSprintf("strMainNld=%s\n",strMainNld.c_str());
						g_strFilelist += "*0|";
					}						
					else
						g_strFilelist += "*1|";
						
				}
				else
				{
					//非NLD文件，打包成NLD下载
					if(strMainNld.empty())
					{
						CFunCommon::DlSprintf("Failed:not find main nld,please check!\n");
						fclose(fp);
						return -1;
					}
				//	CFunCommon::DlSprintf("strMainNld.c_str()=%s,strLeft.c_str()=%s\n",strMainNld.c_str(),strLeft.c_str());
					CreateParaNld(strMainNld.c_str(),strLeft.c_str(),0, 0);
					snprintf(tmpNld,sizeof(tmpNld),"%s/unnzp/para_0.0.NLD",g_szAppPath);
				//	CFunCommon::DlSprintf("tmpNld = %s\n",tmpNld);
					g_strFilelist += tmpNld;
					g_strFilelist += "*1|";
				//	CFunCommon::DlSprintf("g_strFilelist=%s\n",g_strFilelist.c_str());
					noNldFile++;
				}
					
  		 	 }			
			if(noNldFile > 1)
			{
				CFunCommon::DlSprintf("Failed:list file error,one main rom Can only carry 1 ordinary file,please check!\n");
				fclose(fp);
				return -1;
			}
		}
		fclose(fp);
			//CFunCommon::DlSprintf("g_strFilelist = %s\n",g_strFilelist.c_str());
		return 0;
	}

	//list 加载多应用多参数
	if(sType_App_MultiPara == nDownType )
	{
		int noNldFile = 0;
		string strMainNld;
		char tmpNld[MAX_PATH] = {0};
		int nGroup = 0;
		if (NULL == (fp = fopen(pszFileName, "rb")) ) 
		{
			CFunCommon::DlSprintf("Failed to open file %s\n",pszFileName);
			return -2;
		}
		fclose(fp);

		CIniRW ini;
		ini.iniFileLoad(pszFileName);
		nGroup = ini.iniGetInt("Set","GroupNum",0);
		ini.iniFileFree();
		//CFunCommon::DlSprintf("GroupNum=%d\n",nGroup);
		for(int i=0; i<nGroup; i++)
		{
			char szGroup[10] = {0};
			fp = fopen(pszFileName, "rb");
			sprintf(szGroup,"Group%d",i+1);
			noNldFile = 0;
			bFind = false;
			while( fgets(szBuff, sizeof(szBuff), fp))
    		{	
        		if(strstr(szBuff, szGroup) != NULL)     
         		{
         			bFind = true;
					 continue;
        		}
				if(bFind)
				{
					//CFunCommon::DlSprintf("szBuff=%s\n",szBuff);
					if(strstr(szBuff, "[") != NULL)  
						break;
				
					strConver = szBuff;
					nPos = strConver.find('=');  
					if (nPos == string::npos)  	
						continue;  
					strRight = strConver.substr(nPos+1);
					strLeft = strConver.substr(0,nPos);
					CFunCommon::DeleteIndexString(strLeft," ");
					if(!szRelativePub.empty())
						strLeft = szRelativePub + strLeft;

					if(access(strLeft.c_str(),F_OK) != 0)
					{	
						CFunCommon::DlSprintf("Failed to open file %s\n",strLeft.c_str());
						return -2;
					}
					if(strLeft.find(".nld")!= string::npos || strLeft.find(".NLD")!= string::npos )
					{
							g_strFilelist += strLeft;
					
						if(strRight.find("Main NLD")!= string::npos || strRight.find("主程�?")!= string::npos )
						{
							strMainNld = strLeft;
							//CFunCommon::DlSprintf("strMainNld=%s\n",strMainNld.c_str());
							g_strFilelist += "*0|";
						}
						
						else
							g_strFilelist += "*1|";
						
					}

					else
					{
						if(strMainNld.empty())
						{
							CFunCommon::DlSprintf("not find main nld,please check!\n");
							fclose(fp);
							return -1;
						}
					//	CFunCommon::DlSprintf("strMainNld.c_str()=%s,strLeft.c_str()=%s\n",strMainNld.c_str(),strLeft.c_str());
						CreateParaNld(strMainNld.c_str(),strLeft.c_str(),i, i);
						sprintf(tmpNld,"%s/unnzp/para_%d.%d.NLD",g_szAppPath,i,i);
					//	CFunCommon::DlSprintf("tmpNld = %s\n",tmpNld);
						g_strFilelist += tmpNld;
						g_strFilelist += "*1|";
					//	CFunCommon::DlSprintf("g_strFilelist=%s\n",g_strFilelist.c_str());
						noNldFile++;
					}
					
  		 		 }			
				if(noNldFile > 1)
				{
					CFunCommon::DlSprintf("Failed:list file error,one main rom Can only carry 1 ordinary file,please check!\n");
					fclose(fp);
					return -1;
				}
			}
			fclose(fp);	
		}
	//	CFunCommon::DlSprintf("g_strFilelist = %s\n",g_strFilelist.c_str());
		return 0;
	}

	if(nDownType == sType_ANDROID_APP)
	{
		if(!bList)
		{
			//OTA类型为1，下载需要再次解析
			g_strFilelist = pszFileName;
			//CFunCommon::DlSprintf("sFileName = %s\n",sFileName);
			if(strstr(pszFileName,".OTA") != NULL || strstr(pszFileName,".ota") != NULL||strstr(pszFileName,".ZIP") != NULL || strstr(pszFileName,".zip") != NULL)
			{
				g_strFilelist += "*1|";
				//UnpackZipFile(pszFileName);
			}
			else
			{
				g_strFilelist += "*8|";
			}
			return 0;
		}
		else
		{
			if (NULL == (fp = fopen(pszFileName, "rb")) ) 
			{
				CFunCommon::DlSprintf("Failed to open file %s.\n",pszFileName);
				return -2;
			}
			while( fgets(szBuff, sizeof(szBuff), fp))
    		{
        		if(strstr(szBuff, "[FileList]") != NULL)     
         		{
         			bFind = true;
					continue;
        		}
				if(bFind)
				{
					if(strstr(szBuff, "[") != NULL)  
						break;
				
					strConver = szBuff;
					nPos = strConver.find('=');  
					if (nPos == string::npos)  	
						continue;  
					strRight = strConver.substr(nPos+1);
					strLeft = strConver.substr(0,nPos);
					CFunCommon::DeleteIndexString(strLeft," ");
					if(!szRelativePub.empty())
						strLeft = szRelativePub + strLeft;
					//CFunCommon::DlSprintf("strLeft=%s\n",strLeft.c_str());
					//ota
					if(strLeft.find(".ota")!= string::npos || strLeft.find(".OTA")!= string::npos ||strLeft.find(".zip")!= string::npos||strLeft.find(".ZIP")!= string::npos)
					{
						g_strFilelist += strLeft;
						g_strFilelist += "*1|";
					
					}
					else 
					{
						g_strFilelist += strLeft;
						g_strFilelist += "*8|";
					}
						
        		 }
       
  		 	 }
			fclose(fp);
			return 0;
		}
	}
	
	return -1;
}

void SetUsbStatus(bool bConnect){
	g_bConnect = bConnect;
}

/**
 *@brief 读取返回的错误信息
 *@param[in] pszInfo    freopen.out路径
 *@param[out] pszInfo    printf从定向到freopen.out的信息
 *@return void
*/
void ReadResultInfo(char* pszFile,char* pszInfo)
{
	strcpy(pszInfo,g_szResult);
	#if 0
	FILE* fp = NULL;
	int nFileSize = 0;
    fp = fopen(pszFile, "rb");
	if (fp != NULL) 
	{	
		fseek(fp,0,SEEK_END);
		nFileSize = ftell(fp);	
		fseek(fp,0,SEEK_SET);	
		fread(pszInfo,nFileSize,1,fp);
		pszInfo[nFileSize] = '\0';
		//CFunCommon::DlSprintf("pszInfo = %s\n",pszInfo);
		fclose(fp);
	}
	else{
		pszInfo[0] = '\0';
	}
		#endif
}

PFCALLBACK gCallBack = NULL;
/**
 * @brief	dl.so 调用接口
 * @param[in] nType	DOWN_TYPE
 * @param[in] nPlat    POS_PLAT
 * @param[in] pszDev    设备号
 * @param[in] pszDownFile   要下载的文件，可以是新大陆指定类型包文件，若有多个非list加载的文件，通过"|"连接，或者list文件
 * @param[in] pszAppList    删除的应用名，通过‘|’连接,不是删除应用，传入NULL
 * @param[in] bClear    是否清空应用，true 清空，false 不清空
 * @param[out] pszResult    返回错误信息
 * @return
 * @li  0		成功
 * @li  <0		错误
*/
int DownLoaderInterface(int nType,int nPlat,char* pszDev,char* pszDownFile,char *pszAppList,bool bClear,char* pszResult,int (*pnCallBackStatus)(char* pszCurFile ,int nProgress))
{
	CIniRW ini;
	bool bList = false;			//是否通过list配置	

	char szCmd[512] = {0};
	char szUnnzp[MAX_PATH] = {0};		//unnzp 存放ota包解压文件
	char szUnzip[MAX_PATH] = {0};		//unzip NLC,ARD解包路径 
	char szRunPath[MAX_PATH] = {0};		//运行dl.so的路径

	int fd[2];  
	char r_buf[100];                              
    char w_buf[20]= "hello word!";  

	int nPos = 0;
	int nCountSep = 0;				//路径中'/'数量
	
	char szDlRunCmd[256] = {0};	//downloader自身运行 运行参数
	char szFile[MAX_PATH] = {0};		//被下载文件全路径
	char szDev[128] = {0};		//设备号
	char szTmp[256] = {0};
	char szEngIni[MAX_PATH] = {0};	//Multi_eng.ini
	char* pszExt = NULL;		//被下载文件后缀 
	int nDownType = 0;			//下载类型
	int nRet = -1;

	memset(g_szAppPath,0,sizeof(g_szAppPath));
	memset(g_szMessage,0,sizeof(g_szMessage));
	g_strFilelist = "";
	string strTmp;
	memset(g_szResult,0,sizeof(g_szResult));
	gCallBack = pnCallBackStatus;
	
	//n910上使用的so库为jni调用，jni不能获取到so的路径，除非chdir到这个路径，需要通过参数传入， 而linux版本的的so库可以获取自身所在路径，为了代码一致兼容性，加次判断
	if (strlen(pszResult) > 0)
	{
		if(chdir(pszResult) == -1)
		{
			CFunCommon::DlSprintf("Failed to chdir %s : %s\n",pszResult,strerror(errno));
			strcpy(pszResult,g_szResult);
			return -1;
		}
	}
	
	
	if(nType < 0 || nType > 3)
	{
		CFunCommon::DlSprintf("Failed:param nType is error\n");
		strcpy(pszResult,g_szResult);
		return -1;
	}
	if(nPlat < 0 || nPlat > 2)
	{
		CFunCommon::DlSprintf("Failed:param nPlat is error\n");
		strcpy(pszResult,g_szResult);
		return -1;
	}
	if(DOWN_DELAPP == nType)
	{
		if( NULL == pszAppList)
		{
			CFunCommon::DlSprintf("Failed:param pszAppList is error\n");
			strcpy(pszResult,g_szResult);
			return -1;
		}

	}
	if(NULL == pszDev)
	{
		CFunCommon::DlSprintf("Failed:param pszDev is error\n");
		strcpy(pszResult,g_szResult);
		return -1;
	}
	strcpy(szDev,pszDev);

	//获取应用列表	
	if (DOWN_GETAPP == nType)
	{
		if (PLAT_ANDROID == nPlat )
		{
			nDownType = sType_GET_APP;
			CNLPDownload nlp;
			nRet = nlp.GetAppList(szDev);
			strcpy(pszResult,g_szResult);
			return nRet;
			
			
		}
		else if(PLAT_LINUX == nPlat)
		{
			CNLDDownload nld;
			nRet = nld.GetAppList(szDev);
			strcpy(pszResult,g_szResult);
			return nRet;
		}
		else{
			CFunCommon::DlSprintf("%s\n",Usage);
			strcpy(pszResult,g_szResult);
			return -1;
		}
	}

	//删除linux应用
	if (DOWN_DELAPP == nType)
	{
		CNLDDownload nld;
		 if(nld.DelAppList(pszAppList,szDev))
		 {
			strcpy(pszResult,g_szResult);
			return 0;
		 }
		 else{
			 strcpy(pszResult,g_szResult);
			return -1;
		 }
		
	}
	//获取下载程序所在路径
	CFunCommon::GetAbsolutePath (szRunPath) ;
	strcpy(g_szAppPath,szRunPath);
	//CFunCommon::DlSprintf("szRunPath:%s\n",szRunPath);

	//CFunCommon::DlSprintf("Downloader:%s\n",g_szAppPath);
	snprintf(szUnnzp,sizeof(szUnnzp),"%s/unnzp",g_szAppPath);
	snprintf(szUnzip,sizeof(szUnzip),"%s/unzip",g_szAppPath);
	//取被下载文件所在路径
	memset(szTmp,0,sizeof(szTmp));
	strcpy(szTmp,pszDownFile);
	if('/' == szTmp[0] )
	{
		strcpy(szFile,pszDownFile);
		
	}
	else{
		snprintf(szFile,sizeof(szFile),"%s/%s",szRunPath,szTmp);
	}
	//下载文件后缀
	pszExt = strrchr(szFile,'.');

	if(NULL == pszExt)
	{
		CFunCommon::DlSprintf("Failed:The file extension is not supported.\n");
		return -1;
	}

	if (strcasecmp(pszExt,".ARD") == 0 )
	{
		nDownType = sType_ANDROID_FIRM;
	} 
	else if (strcasecmp(pszExt,".NLC") == 0 )
	{
		nDownType = sType_NLC;
	} 
	else if (strcasecmp(pszExt,".NLP") == 0)
	{
		if (DOWN_APP == nType ) 
				nDownType = sType_MPOS_APP;	
		if (DOWN_FIRM == nType)	
			nDownType = sType_MPOS_FIRM;

	} 
	
	else if (strcasecmp(pszExt,".NLD") == 0)
	{
		nDownType = sType_App;
	}
	else if (strcasecmp(pszExt,".OTA") == 0  || strcasecmp(pszExt,".ZIP") == 0  || strcasecmp(pszExt,".APK") == 0 
		|| strcasecmp(pszExt,".SIG") == 0  || strcasecmp(pszExt,".PEM_NEW") == 0  || strcasecmp(pszExt,".CRT") == 0 )
	
	{
		nDownType = sType_ANDROID_APP;
	}
	else if (strcasecmp(pszExt,".LIST") == 0)
	{
		 
			if(ini.iniFileLoad(szFile) == -1)
			{
				CFunCommon::DlSprintf("Failed to open %s\n",szFile);
				strcpy(pszResult,g_szResult);
				return -1;
			}
			//先判断固件
			nDownType = ini.iniGetInt("BootHex","DownType",0);
			//应用类型
			if(0 == nDownType )
				nDownType = ini.iniGetInt("Set","Type",0);
			ini.iniFileFree();
			bList = true;
	}
	else
	{
		CFunCommon::DlSprintf("%s",Usage);
		strcpy(pszResult,g_szResult);
		return -1;
	}
	
	if (sType_ANDROID_FIRM == nDownType || sType_NLC == nDownType || sType_MPOS_FIRM == nDownType )
	{
		if(1 != nType)
		{
			CFunCommon::DlSprintf("%s\n",Usage);
			strcpy(pszResult,g_szResult);
			return -1;
		}
	}
	if (sType_ANDROID_APP == nDownType || sType_NLD == nDownType || sType_MPOS_APP == nDownType )
	{
		if( 0 != nType)
		{
			CFunCommon::DlSprintf("%s\n",Usage);
			strcpy(pszResult,g_szResult);
			return -1;
		}
	}
	//system("export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:./");
	//CFunCommon::DlSprintf("nDownType:%d,DownloaderFile:%s\n",nDownType,szFile);
	//读错误信息ini
	snprintf(szEngIni,sizeof(szEngIni),"%s/Language/Multi_eng.ini",g_szAppPath);
	if (ini.iniFileLoad(szEngIni) == -1)
	{
		CFunCommon::DlSprintf("Failed to open %s\n",szEngIni);
		return -1;
	}
	for(int i=0; i<MSGSUM; i++)
	{
		sprintf(szTmp,"%d",i);
		ini.iniGetString("g_szMessage",szTmp,g_szMessage[i],256,"Error");
	}
	ini.iniFileFree();

	//mkdir unzip,unnzp
	snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnzip);
	CFunCommon::ExecSystem(szCmd);
	snprintf(szCmd,sizeof(szCmd),"mkdir %s",szUnzip);
	CFunCommon::ExecSystem(szCmd);

	snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnnzp);
	CFunCommon::ExecSystem(szCmd);
	snprintf(szCmd,sizeof(szCmd),"mkdir %s",szUnnzp);
	CFunCommon::ExecSystem(szCmd);
		
	
	//ard 下载
	if(sType_ANDROID_FIRM  == nDownType )
	{
		snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnzip);
		CArdDownload ardlist;
	
		pid_t pid;
 		if(pipe(fd) < 0)                     //新建管道        
    	{  
   	  	  	CFunCommon::ExecSystem(szCmd);
          	CFunCommon::DlSprintf("Failed:pipe error!\n");  
			strcpy(pszResult,g_szResult);
			return -1;
   		 }  
		pid=fork();
		if(pid<0)
		{
			CFunCommon::DlSprintf("Failed:error in fork!");
			strcpy(pszResult,g_szResult);
			return -1;
		}
		
		else if(pid==0)
		{
			//CFunCommon::DlSprintf("child process ID is %d/n",getpid());
			CArdDownload arddown;
			chdir(g_szAppPath);
			//重启bootloader
			if(arddown.DownloadProcess((const char*)szFile)==2)
			{
				close(fd[0]);
				usleep(200);  
            	write(fd[1], w_buf, 20);
			}
		
		}
		else
		{
			//CFunCommon::DlSprintf(" parent process ID is %d/n",getpid());
			signal(SIGCHLD, handle);  
    		int ret;  
        	while(1)  
    		{  
           	 	sleep(1);  
           	 	ret = kill(pid, 0);  
            	if (ret < 0)  
            	{  
            	   // CFunCommon::DlSprintf("child had dead\n");  
					break;
            	}  
           	 	else  
            	{  
            	    //CFunCommon::DlSprintf("child is alive\n");  
            	}  
    		}  
			close(fd[1]);
			usleep(200);  
       		read(fd[0], r_buf, 20);
			if(strcmp(r_buf,"hello word!") ==0)
			{
				chdir(g_szAppPath);
				CArdDownload arddown1;
				if(arddown1.WaitForConnect() !=0)
				{
					CFunCommon::DlSprintf("Failed:Please confirm the connection to the Newland device and download again\n");
					CFunCommon::ExecSystem(szCmd);
					strcpy(pszResult,g_szResult);
					return -1;
				}
				nRet = arddown1.DownloadProcess((const char*)szFile);
				strcpy(pszResult,g_szResult);
				return nRet;
			}
			
		}
		CFunCommon::ExecSystem(szCmd);
		strcpy(pszResult,g_szResult);
		return 0;
	}
	//NLC 下载
	if(sType_NLC == nDownType )
	{
		snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnzip);
		CNlcDownload nlcdown;
		if(0 != nlcdown.ParseFileList(szFile,bList) )
		{
			CFunCommon::ExecSystem(szCmd);
			strcpy(pszResult,g_szResult);
			return -1;
		}	
		chdir(g_szAppPath);
		//重启bootloader
		if(2 == (nRet = nlcdown.DownloadProcess(szFile,szDev)))
		{
				chdir(g_szAppPath);
				if(nlcdown.WaitForConnect(szDev) != 0 )
				{
					CFunCommon::DlSprintf("Failed:Please confirm the connection to the Newland device and download again\n");
					CFunCommon::ExecSystem(szCmd);
					strcpy(pszResult,g_szResult);
					return -1;
				}
				nRet = nlcdown.DownloadProcess(szFile,szDev);
			
				CFunCommon::ExecSystem(szCmd);
				strcpy(pszResult,g_szResult);
				return nRet;	
		}		
		
		else
		{
			CFunCommon::ExecSystem(szCmd);
			strcpy(pszResult,g_szResult);
			return nRet;	
		}		
	}
	//下载NLP:包含 参数为固件NLP,参数为应用NLP,参数为list，配置的固件下，参数为list配置的应用下载
	if(sType_MPOS_APP == nDownType  || sType_MPOS_FIRM == nDownType )
	{
		CNLPDownload nlp;
		if(MakeDownFileString(szFile,bList,nDownType) < 0)
		{
			strcpy(pszResult,g_szResult);
			return -1;
		}
		
		//CFunCommon::DlSprintf("g_strFilelist = %s\n",g_strFilelist.c_str());
		nRet = nlp.DownloadProcess(g_strFilelist.c_str(),szDev,bClear,nDownType);
		strcpy(pszResult,g_szResult);
		return nRet;
	}
		
	//下载NLD,1、参数为NLD，2 list配置NLD下载，3list配置的多应用多参数NLD下载
	if(sType_NLD == nDownType ||sType_App == nDownType || sType_App_MultiPara == nDownType )
	{

		CNLDDownload nld;
		if(MakeDownFileString(szFile,bList,nDownType) <0 )
		{
			strcpy(pszResult,g_szResult);
			return -1;
		}
		nld.SetClearFlag(bClear);
		nRet = nld.DownloadProcess(g_strFilelist.c_str(),szDev);
		snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnnzp);
		CFunCommon::ExecSystem(szCmd);
		strcpy(pszResult,g_szResult);
		return nRet;
	}

	//下载androidPOS应用：包含单个可识别类型的android应用，list加载的android应用
	if(sType_ANDROID_APP == nDownType)
	{
		//android 应用下载与nlp scrop平台下载协议相同，只是发送帧大小不同 根据握手返回确认，有4k，16k；scrop的只有4k
		CNLPDownload nlp;
		snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnnzp);
		if(!nlp.ParseAndroidFileList((const char*)szFile,bList))
		{
			CFunCommon::ExecSystem(szCmd);
			strcpy(pszResult,g_szResult);
			return -1;
		}
		nRet = nlp.DownloadProcess(g_strFilelist.c_str(),szDev,bClear,nDownType);
	
		CFunCommon::ExecSystem(szCmd);
		strcpy(pszResult,g_szResult);
		return nRet;
	}
	strcpy(pszResult,g_szResult);
	return 0;
}




int main(int argc,char *argv[])
{   
	
	CIniRW ini;
	bool bClear = false;		//是否清空应用
	bool bList = false;			//是否通过list配置	

	char szCmd[512] = {0};
	char szUnnzp[MAX_PATH] = {0};		//unnzp 存放ota包解压文件
	char szUnzip[MAX_PATH] = {0};		//unzip NLC,ARD解包路径 
	char szRunPath[MAX_PATH] = {0};		//运行dl的路径


	int fd[2];  
	char r_buf[100];                              
    char w_buf[20]= "hello word!";  

	int nPos = 0;
	int nCountSep = 0;				//路径中'/'数量
	
	char szDlRunCmd[256] = {0};	//downloader自身运行 运行参数
	char szFile[MAX_PATH] = {0};		//被下载文件全路径
	char szDev[128] = {0};		//设备号
	char szTmp[256] = {0};
	char szEngIni[MAX_PATH] = {0};	//Multi_eng.ini
	char* pszExt = NULL;		//被下载文件后缀 
	int nDownType = 0;			//下载类型
	string strTmp;
	int nType = 0; //用于判断参数是否使用正确，0,应用下载，1固件下载,2获取应用列表,3删除linux应用,4,删除android应用
	
	
	if(2 != argc && 5 != argc && 6 != argc && 3 != argc )
	{
		CFunCommon::DlSprintf("%s\n",Usage);
		return -1;
	}
	if( (strcasecmp(argv[1],"-a") != 0 ) && (strcasecmp(argv[1],"-v") != 0 ) && (strcasecmp(argv[1],"-f") != 0 ) && (strcasecmp(argv[1],"-l") != 0 )  && (strcasecmp(argv[1],"del_linux") != 0 )) 
	{
		CFunCommon::DlSprintf("%s\n",Usage);
		return -1;
	}
	if( strcasecmp(argv[1],"-a") == 0 )
	{
		nType = 0;
	}
	if( strcasecmp(argv[1],"-f") == 0 )
	{
		nType = 1;
	}
	if( strcasecmp(argv[1],"-l") == 0 )
	{
		nType = 2;
	}
	if( strcasecmp(argv[1],"del_linux") == 0 )
	{
		nType = 3;
	}
	if( strcasecmp(argv[1],"del_android") == 0 )
	{
		nType = 4;
	}
	if(strcasecmp(argv[1],"-v") == 0 )
	{
		CFunCommon::DlSprintf("%s\n",DOWNLOSD_VER);
		return -1;
	}
	if (argc > 3)
	{
		if (strcasecmp(argv[3],"-d") != 0)
		{
			CFunCommon::DlSprintf("%s\n",Usage);
			return -1;
		}
		strcpy(szDev,argv[4]);
	}
	
	
	
	if (6 == argc)
	{
		if (strcasecmp(argv[5],"-c") != 0 || strcasecmp(argv[2],"-a") != 0)
		{
			CFunCommon::DlSprintf("%s\n",Usage);
			return -1;
		}
		bClear = true;	
	}
		
	//获取应用列表	
	if (2 == nType)
	{
		if (strcasecmp(argv[2],"android") == 0 )
		{
			nDownType = sType_GET_APP;
			CNLPDownload nlp;
			nlp.GetAppList(szDev);
			return 0;
		}
		else if(strcasecmp(argv[2],"linux") == 0 )
		{
			CNLDDownload nld;
			nld.GetAppList(szDev);
			return 0;
		}
		else{
			CFunCommon::DlSprintf("%s\n",Usage);
			return -1;
		}
	}
	//删除linux应用
	if (3 == nType)
	{
		CNLDDownload nld;
		nld.DelAppList(argv[2],szDev);
		return 0;
	}
	//获取下载程序所在路径
	strcpy(szDlRunCmd, argv[0]);
	strcpy(szTmp, argv[0]);
	CFunCommon::GetAbsolutePath (szRunPath) ;
	//CFunCommon::DlSprintf("szRunPath:%s\n",szRunPath);
	nCountSep = CFunCommon::ReplaceAnsi(szTmp,'/','\\');
	if(1 == nCountSep )
	{
		//在dl所在当前路径运行dl
		if(strncmp(szDlRunCmd,"./",2) == 0  )
		{
			strcpy(g_szAppPath,szRunPath);
		}
		else if(strncmp(szDlRunCmd,"../",3) == 0)
		{
			snprintf(g_szAppPath,sizeof(g_szAppPath),"%s%s",szRunPath,szDlRunCmd);
		}
		else{
			CFunCommon::DlSprintf("Failed:Unrecognized path.\n");
			return -1;
		}
	}
	
	if(nCountSep > 1)
	{
		//绝对路径,
		if('/' == szDlRunCmd[0]) 
		{
			strcpy(g_szAppPath,szDlRunCmd);
		//	memset(g_szAppPath,0,sizeof(g_szAppPath));
			//strcpy(g_szAppPath, dirname(szDlRunCmd));	
		}
		//相对路径
		else 
		{
			strcpy(g_szAppPath,szRunPath);
			strcat(g_szAppPath,"/");
			strcat(g_szAppPath,szDlRunCmd);
		}
		
	}
	
	//CFunCommon::DlSprintf("Downloader:%s\n",g_szAppPath);
	snprintf(szUnnzp,sizeof(szUnnzp),"%s/unnzp",g_szAppPath);
	snprintf(szUnzip,sizeof(szUnzip),"%s/unzip",g_szAppPath);
	//取被下载文件所在路径
	memset(szTmp,0,sizeof(szTmp));
	strcpy(szTmp,argv[2]);
	if('/' == szTmp[0] )
	{
		strcpy(szFile,argv[2]);
		
	}
	else{
		snprintf(szFile,sizeof(szFile),"%s/%s",szRunPath,szTmp);
	}
	//下载文件后缀
	pszExt = strrchr(szFile,'.');
	if(NULL == pszExt)
	{
		CFunCommon::DlSprintf("Failed:The file extension is not supported.\n");
		return -1;
	}

	if (strcasecmp(pszExt,".ARD") == 0 )
	{
		nDownType = sType_ANDROID_FIRM;
	} 
	else if (strcasecmp(pszExt,".NLC") == 0 )
	{
		nDownType = sType_NLC;
	} 
	else if (strcasecmp(pszExt,".NLP") == 0)
	{
		if (strcasecmp(argv[1],"-a") == 0)
				nDownType = sType_MPOS_APP;	
		if (strcasecmp(argv[1],"-f") == 0)	
			nDownType = sType_MPOS_FIRM;

	} 
	else if (strcasecmp(pszExt,".LIST") == 0)
	{
			ini.iniFileLoad(szFile);
			//先判断固件
			nDownType = ini.iniGetInt("BootHex","DownType",0);
			//应用类型
			if(0 == nDownType )
				nDownType = ini.iniGetInt("Set","Type",0);
			ini.iniFileFree();
			bList = true;
	}
	else if (strcasecmp(pszExt,".NLD") == 0)
	{
		nDownType = sType_App;
	}
	else if (strcasecmp(pszExt,".OTA") == 0  || strcasecmp(pszExt,".ZIP") == 0  || strcasecmp(pszExt,".APK") == 0 
		|| strcasecmp(pszExt,".SIG") == 0  || strcasecmp(pszExt,".PEM_NEW") == 0  || strcasecmp(pszExt,".CRT") == 0 )
	
	{
		nDownType = sType_ANDROID_APP;
	}
	else
	{
		CFunCommon::DlSprintf("%s",Usage);
		return -1;
	}
	
	if (sType_ANDROID_FIRM == nDownType || sType_NLC == nDownType || sType_MPOS_FIRM == nDownType )
	{
		if(1 != nType)
		{
			CFunCommon::DlSprintf("Failed:%s\n",Usage);
			return -1;
		}
	}
	if (sType_ANDROID_APP == nDownType || sType_NLD == nDownType || sType_MPOS_APP == nDownType )
	{
		if( 0 != nType)
		{
			CFunCommon::DlSprintf("Failed:%s\n",Usage);
			return -1;
		}
	}

	//system("export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:./");

	//CFunCommon::DlSprintf("DownloaderFile:%s\n",szFile);
	//读错误信息ini
	snprintf(szEngIni,sizeof(szEngIni),"%s/Language/Multi_eng.ini",g_szAppPath);
	
	ini.iniFileLoad(szEngIni);
	for(int i=0; i<MSGSUM; i++)
	{
		sprintf(szTmp,"%d",i);
		ini.iniGetString("g_szMessage",szTmp,g_szMessage[i],256,"Error");
	}
	ini.iniFileFree();
	
	//mkdir unzip,unnzp
	snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnzip);
	CFunCommon::ExecSystem(szCmd);
	snprintf(szCmd,sizeof(szCmd),"mkdir %s",szUnzip);
	CFunCommon::ExecSystem(szCmd);

	snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnnzp);
	CFunCommon::ExecSystem(szCmd);
	snprintf(szCmd,sizeof(szCmd),"mkdir %s",szUnnzp);
	CFunCommon::ExecSystem(szCmd);
		
	//ard 下载
	if(sType_ANDROID_FIRM  == nDownType )
	{
		snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnzip);
		CArdDownload ardlist;
	
		pid_t pid;
 		if(pipe(fd) < 0)                     //新建管道        
    	{  
   	  	  	CFunCommon::ExecSystem(szCmd);
          	CFunCommon::DlSprintf("Failed:pipe error!\n");  
        	return -1;
   		 }  
		pid=fork();
		if(pid<0)
			CFunCommon::DlSprintf("Failed:error in fork!");
		else if(pid==0)
		{
			//CFunCommon::DlSprintf("child process ID is %d/n",getpid());
			CArdDownload arddown;
			chdir(g_szAppPath);
			//重启bootloader
			if(arddown.DownloadProcess((const char*)szFile)==2)
			{
				close(fd[0]);
				usleep(200);  
            	write(fd[1], w_buf, 20);
			}
		
		}
		else
		{
			//CFunCommon::DlSprintf(" parent process ID is %d/n",getpid());
			signal(SIGCHLD, handle);  
    		int ret;  
        	while(1)  
    		{  
           	 	sleep(1);  
           	 	ret = kill(pid, 0);  
            	if (ret < 0)  
            	{  
            	   // CFunCommon::DlSprintf("child had dead\n");  
					break;
            	}  
           	 	else  
            	{  
            	    //CFunCommon::DlSprintf("child is alive\n");  
            	}  
    		}  
			close(fd[1]);
			usleep(200);  
       		read(fd[0], r_buf, 20);
			if(strcmp(r_buf,"hello word!") ==0)
			{
				chdir(g_szAppPath);
				CArdDownload arddown1;
				if(arddown1.WaitForConnect() !=0)
				{
					CFunCommon::DlSprintf("Failed:Please confirm the connection to the Newland device and download again\n");
					CFunCommon::ExecSystem(szCmd);
					return -1;
				}
				arddown1.DownloadProcess((const char*)szFile);
			
			}
			
		}
		CFunCommon::ExecSystem(szCmd);
		return 0;
	}


	//NLC 下载
	if(sType_NLC == nDownType )
	{
		snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnzip);
		CNlcDownload nlcdown;
		if(0 != nlcdown.ParseFileList(szFile,bList) )
		{
			CFunCommon::ExecSystem(szCmd);
			return -1;
		}	
		pid_t pid;
 		if(pipe(fd) < 0)                     			//新建管道        
    	{  
   	  	  	CFunCommon::ExecSystem(szCmd);
        	CFunCommon::DlSprintf("Failed to pipe.!\n");  
        	exit(1);  
   		}  
		pid = fork();
		if(pid < 0)
			CFunCommon::DlSprintf("Failed to fork!");
		//子进程完成当前次下载	
		else if(0 == pid)
		{
			//CFunCommon::DlSprintf("child process ID is %d\n",getpid());
			chdir(g_szAppPath);
			//重启bootloader
			if(2 == nlcdown.DownloadProcess(szFile,szDev))
			{
				close(fd[0]);
				usleep(200);  
            	write(fd[1], w_buf, 20);
			}		
		}
		else
		{
			//CFunCommon::DlSprintf(" parent process ID is %d\n",getpid());
			signal(SIGCHLD, handle);  
    		int ret;  
        	while(1)  
    		{  
           	 	sleep(1);  
           	 	ret = kill(pid, 0);  
            	if (ret < 0)  
            	{  
            	   // CFunCommon::DlSprintf("child had dead\n");  
					break;
            	}  
           		else  
            	{  
            	    //CFunCommon::DlSprintf("child is alive\n");  
            	}  
    		}  
			close(fd[1]);
			usleep(200);  
       		read(fd[0], r_buf, 20);
			if(strcmp(r_buf,"hello word!") == 0)
			{
				chdir(g_szAppPath);
				if(nlcdown.WaitForConnect(szDev) != 0 )
				{
					CFunCommon::DlSprintf("Failed:Please confirm the connection to the Newland device and download again\n");
					CFunCommon::ExecSystem(szCmd);
					return -1;
				}
				nlcdown.DownloadProcess(szFile,szDev);
			
				CFunCommon::ExecSystem(szCmd);
			
				return 0;	
			}	
			CFunCommon::ExecSystem(szCmd);
			return 0;	
		}	
		
		
	}
	//下载NLP:包含 参数为固件NLP,参数为应用NLP,参数为list，配置的固件下，参数为list配置的应用下载
	if(sType_MPOS_APP == nDownType  || sType_MPOS_FIRM == nDownType )
	{
		CNLPDownload nlp;
		if(MakeDownFileString(szFile,bList,nDownType) < 0)
		{
			return -1;
		}
		
		//CFunCommon::DlSprintf("g_strFilelist = %s\n",g_strFilelist.c_str());
		nlp.DownloadProcess(g_strFilelist.c_str(),szDev,bClear,nDownType);
		return 0;
	}
		
	//下载NLD,1、参数为NLD，2 list配置NLD下载，3list配置的多应用多参数NLD下载
	if(sType_NLD == nDownType ||sType_App == nDownType || sType_App_MultiPara == nDownType )
	{
		CNLDDownload nld;
		if(MakeDownFileString(szFile,bList,nDownType) <0 )
		{
			return -1;
		}
		nld.SetClearFlag(bClear);
		nld.DownloadProcess(g_strFilelist.c_str(),szDev);
		snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnnzp);
		CFunCommon::ExecSystem(szCmd);
		return 0;
	}

	//下载androidPOS应用：包含单个可识别类型的android应用，list加载的android应用
	if(sType_ANDROID_APP == nDownType)
	{
		//android 应用下载与nlp scrop平台下载协议相同，只是发送帧大小不同 根据握手返回确认，有4k，16k；scrop的只有4k
		CNLPDownload nlp;
		if(!nlp.ParseAndroidFileList((const char*)szFile,bList))
		{
			snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnnzp);
			CFunCommon::ExecSystem(szCmd);
			return -1;
		}
		nlp.DownloadProcess(g_strFilelist.c_str(),szDev,bClear,nDownType);
		snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szUnnzp);
		CFunCommon::ExecSystem(szCmd);
		return 0;
	}
	
	return 0;
}
