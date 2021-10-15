#include "nlcdownload.h"
#include "lzss.h"
#include "inirw.h"
#include "function.h" 
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define cmd_get_length(p)   (*(p + 2) | ((*(p + 3)) << 8))

extern char g_szMessage[MSGSUM][100];
extern char g_szAppPath[MAX_PATH];

static NLC_INFO arrNLC[MAX_ARD_NUM];	/// arrNLC很大，定义全局变量，局部变量或类成员数据会超过系统默认最大限制
extern PFCALLBACK gCallBack ;

/**
 *	@brief 构造函数
*/
CNlcDownload::CNlcDownload()
{
	m_nSlaveCount = 0;
	m_bMaster = 0;
	m_partinfo = NULL;
	m_packetpart = NULL;
	m_packetpartold = NULL;
	m_ptrpart=  &m_ptrpartThread[0];		
	m_beNewBios = true;
}

/**
 * @brief 析构函数
*/
CNlcDownload::~CNlcDownload()
{

}

/**
 * @brief		读取加载单个NLC方式的配置文件newconfig.ini的分组信息
 * @param[in] pszNLC  NLC文件名
 * @param[in] pszFileName 	newconfig.ini全路径
 * @param[in] nIndex    	unzip目录下索引
 * @return
 * @li true		成功
 * @li false	失败
*/
bool CNlcDownload::ReadGroupAttAuto(const char* pszNLC,const char* pszFileName,int nIndex)
{
	int i = 0,j = 0;
	int ndex = 0;
	int nCount = 0; 
	char szIndex[256] = {0};
	char szCmd[512] = {0};
	char szTmp[128] = {0};
	char szVer[128] = {0};
	string strFile,strMark,strSectorName,strFullPath,strPackName;
	CIniRW ini;
	if(-1 == (access(pszFileName,F_OK)))  
	{
		CFunCommon::DlSprintf("Failed:%s doesn't exist!\n",pszFileName);
		return false;
	}

	ini.iniFileLoad(pszFileName);
	ndex = ini.iniGetInt("ATTR","FULLFLAG",0);
	//0为全量包，1为单分区
	if (0 == ndex )
	{
		m_nSlaveCount = 0;
		m_bIsFull = true;
	} 
	else
	{
		m_nSlaveCount = 1;
		m_bIsFull = false;
	}
	if(0 == nIndex)
	{
		nIndex = ndex;
	}	
	arrNLC[nIndex].bIsUsed = true;	
	arrNLC[nIndex].nGroupCount = ini.iniGetInt("GROUPCOUNT","count",-1);
	if (-1 == arrNLC[nIndex].nGroupCount  || 0 == arrNLC[nIndex].nGroupCount)
	{
		arrNLC[nIndex].nFirmPackVer = 0;
		arrNLC[nIndex].nFullPack = 0;
	}
	else
	{	
		arrNLC[nIndex].nFirmPackVer = ini.iniGetInt("ATTR","PACKVER",-1);
		arrNLC[nIndex].nFullPack = ini.iniGetInt("ATTR","FULLFLAG",0);

		if ((arrNLC[nIndex].nGroupCount > 0) && (-1 == arrNLC[nIndex].nFirmPackVer) )
			arrNLC[nIndex].nFirmPackVer = 1;
	}
	
	ini.iniGetString("NEWPLAT","VER",szVer,sizeof(szVer),"");
	if(strcmp(szVer,"6ul") == 0 )
	{
		m_cDownType = sType_6UL;
	}	
	else
	{
		m_cDownType = sType_NLC;
	}
	
	for(i=0; i<arrNLC[nIndex].nGroupCount; i++)
	{
		
		strMark = "GROUP";
		sprintf(szIndex,"%d",i);
		strMark += szIndex;
		memset(szVer, 0x00, sizeof(szVer));
	
		ini.iniGetString(strMark.c_str(),"ver",szVer,sizeof(szVer),"Error");
		strcpy(arrNLC[nIndex].szGroupFileAtt[i].GrAttitude.szGroupVer,szVer);	
		arrNLC[nIndex].szGroupFileAtt[i].GrAttitude.GroupCount=ini.iniGetInt(strMark.c_str(),"count",0);
		arrNLC[nIndex].szGroupFileAtt[i].GrAttitude.iBoot=ini.iniGetInt(strMark.c_str(),"boot",0);
		
		for(j=0;j<arrNLC[nIndex].szGroupFileAtt[i].GrAttitude.GroupCount;j++)
		{
			strFile="FILE";
			snprintf(szIndex,sizeof(szIndex),"%d",j);
			strFile += szIndex;
			memset(szVer, 0x00, sizeof(szVer));
			ini.iniGetString(strMark.c_str(),strFile.c_str(),szVer,sizeof(szVer),"Error");
			strcpy(arrNLC[nIndex].szGroupFileAtt[i].szGroupFile[j],szVer);
		}
	
	}
	memset(arrNLC[nIndex].szBiospara,0,sizeof(arrNLC[nIndex].szBiospara));
	nCount = ini.iniGetInt("FILECOUNT","count",0); 
	arrNLC[nIndex].nBinFileCount = nCount;
	for (i=0; i<nCount; i++)
	{
		memset(szIndex,0,sizeof(szIndex));
		snprintf(szIndex,sizeof(szIndex),"FILE%d",i);
		memset(szVer,0,sizeof(szVer));
		ini.iniGetString(szIndex,"sector",szVer,sizeof(szVer),"Error");
		strcpy(arrNLC[nIndex].szBiospara[i].szSector,szVer);

		memset(szVer,0,sizeof(szVer));
		ini.iniGetString(szIndex,"fullname",szVer,sizeof(szVer),"Error");
		strcpy(arrNLC[nIndex].szBiospara[i].szFullname,szVer);
	
		memset(szVer,0,sizeof(szVer));
		ini.iniGetString(szIndex,"ver",szVer,sizeof(szVer),"Error");
		strcpy(arrNLC[nIndex].szBiospara[i].szVer,szVer);

		arrNLC[nIndex].szBiospara[i].cIsupdate = ini.iniGetInt(szIndex,"isupdate",0); 

	}
	strcpy(arrNLC[nIndex].szFileName , pszNLC);
	ini.iniFileFree();

	char szNewPath[MAX_PATH] = {0};
	char szOldPath[MAX_PATH] = {0};
	snprintf(szOldPath,sizeof(szOldPath),"%s/unzip/nlctmp",g_szAppPath);
	snprintf(szNewPath,sizeof(szNewPath),"%s/unzip/%d",g_szAppPath,nIndex);
	snprintf(szCmd,sizeof(szCmd),"rm -rf %s",szNewPath);
	CFunCommon::ExecSystem(szCmd);
	snprintf(szCmd,sizeof(szCmd),"mv %s %s",szOldPath,szNewPath);
	CFunCommon::ExecSystem(szCmd);
	
	return 0;
}

/**
 * @brief 初始化arrNLC数组
*/
 void CNlcDownload::InitMultiNlcArr()
{	
	for (int i=0;i<MAX_ARD_NUM;i++)
	{
		arrNLC[i].bIsUsed = false;
		memset(arrNLC[i].szFileName,0,sizeof(arrNLC[i].szFileName));
		memset(arrNLC[i].szFilePath,0,sizeof(arrNLC[i].szFilePath));
		arrNLC[i].nFirmPackVer=-1;
		arrNLC[i].nFullPack=-1;
		arrNLC[i].nGroupCount=-1;		
	}
	return;
}




/**
 * @brief	读取解析通过加载list方式的配置文件newconfig.ini的分组信息
 * @param[in] pszFileName 	newconfig.ini全路径
 * @param[in] nIndex    		unzip目录下索引
 * @return
 * @li true		成功
 * @li false	失败
*/
 bool CNlcDownload::ReadGroupAtt(const char* pszFileName,int nIndex)
{
	int i = 0, j = 0;
	int nCount = 0; 
	string strFile,strMark,strMark_1, strSectorName, strFullPath,strPackName;
	char szIndex[256] = {0};
	char szTmp[128] = {0};
	CIniRW ini;

	//CFunCommon::DlSprintf("sFileName = %s",sFileName);
	if(-1 == (access(pszFileName,F_OK)))  
	{
		CFunCommon::DlSprintf("Failed:%s doesn't  exist!\n",pszFileName);
		return false;

	}
	ini.iniFileLoad(pszFileName);
	arrNLC[nIndex].bIsUsed = true;	
	arrNLC[nIndex].nGroupCount = ini.iniGetInt("GROUPCOUNT","count",-1);
	if (-1 == arrNLC[nIndex].nGroupCount  || 0 == arrNLC[nIndex].nGroupCount)
	{
		arrNLC[nIndex].nFirmPackVer = 0;
		arrNLC[nIndex].nFullPack = 0;
	}
	else
	{	
		arrNLC[nIndex].nFirmPackVer = ini.iniGetInt("ATTR","PACKVER",-1);
		arrNLC[nIndex].nFullPack = ini.iniGetInt("ATTR","FULLFLAG",0);

		if ((arrNLC[nIndex].nGroupCount > 0) && (-1 == arrNLC[nIndex].nFirmPackVer))
			arrNLC[nIndex].nFirmPackVer = 1;
	}
	
	for(i=0; i<arrNLC[nIndex].nGroupCount; i++)
	{
		strMark = "GROUP";
		sprintf(szIndex,"%d",i);
		strMark += szIndex;
		memset(szTmp, 0x00, sizeof(szTmp));
	
		ini.iniGetString(strMark.c_str(),"ver",szTmp,sizeof(szTmp),"Error");
		strcpy(arrNLC[nIndex].szGroupFileAtt[i].GrAttitude.szGroupVer,szTmp);	
		arrNLC[nIndex].szGroupFileAtt[i].GrAttitude.GroupCount=ini.iniGetInt(strMark.c_str(),"count",0);
		arrNLC[nIndex].szGroupFileAtt[i].GrAttitude.iBoot=ini.iniGetInt(strMark.c_str(),"boot",0);
		
		for(j=0; j<arrNLC[nIndex].szGroupFileAtt[i].GrAttitude.GroupCount; j++)
		{
			strFile="FILE";
			sprintf(szIndex,"%d",j);
			strFile += szIndex;
			memset(szTmp, 0x00, sizeof(szTmp));
			ini.iniGetString(strMark.c_str(),strFile.c_str(),szTmp,sizeof(szTmp),"Error");
			strcpy(arrNLC[nIndex].szGroupFileAtt[i].szGroupFile[j],szTmp);
		}	
	}
	memset(arrNLC[nIndex].szBiospara,0,sizeof(arrNLC[nIndex].szBiospara));
	nCount = ini.iniGetInt("FILECOUNT","count",0); 
	arrNLC[nIndex].nBinFileCount = nCount;
	for (i = 0; i<nCount; i++)
	{
		memset(szIndex,0,sizeof(szIndex));
		sprintf(szIndex,"FILE%d",i);
		memset(szTmp,0,sizeof(szTmp));
		ini.iniGetString(szIndex,"sector",szTmp,sizeof(szTmp),"Error");
		strcpy(arrNLC[nIndex].szBiospara[i].szSector,szTmp);
	
		memset(szTmp,0,sizeof(szTmp));
		ini.iniGetString(szIndex,"fullname",szTmp,sizeof(szTmp),"Error");
		strcpy(arrNLC[nIndex].szBiospara[i].szFullname,szTmp);

		memset(szTmp,0,sizeof(szTmp));
		ini.iniGetString(szIndex,"ver",szTmp,sizeof(szTmp),"Error");
		strcpy(arrNLC[nIndex].szBiospara[i].szVer,szTmp);
	
		arrNLC[nIndex].szBiospara[i].cIsupdate=ini.iniGetInt(szIndex,"isupdate",0); 
		
	}
	ini.iniFileFree();
	return true;
}

/**
 * @brief 	对固件下载传入参数解析，解压出bin文件，并解析各个NLC包配置信息
 * @param [in]pszFileName  单个NLC/LIST文件绝对路径
 * @param[in] bList  是否list
 * @return
 * @li  0        	成功
 * @li  其它       	失败
*/
int CNlcDownload::ParseFileList(const char* pszFileName,bool bList)
{

	int nPos = 0;
	char szIniFile[MAX_PATH] = {0}; //newconfig.ini

	m_nSlaveCount = 0;
	m_bMaster = false;

	InitMultiNlcArr();	
	
	if(access(pszFileName,F_OK) == -1)  
	{
		CFunCommon::DlSprintf("Failed:%s not exist,please check.\n",pszFileName);
		return -1;
	}
	//单个NLC
	if(!bList)
	{
		//解包NLC
		if(!UnCompressNLCAuto(pszFileName,0))
		{
			return -1;
		}
		//解析ini	
		sprintf(szIniFile,"%s/unzip/nlctmp/newconfig.ini",g_szAppPath);
		ReadGroupAttAuto(pszFileName,szIniFile,0);
		return 0;
	}
	//list
	else
	{
	
		CIniRW ini;
		string strRelativePath = ""; 
		string strRelativePub = "";		//list文件所在路径
		char szFileName[MAX_PATH] = {0};	//list中配置的文件名

		strRelativePath = pszFileName;
		nPos = strRelativePath.rfind('/'); 
		if(nPos != string::npos)
		{
			strRelativePub = strRelativePath.substr(0,nPos+1);
		}
		memset(szFileName,0,sizeof(szFileName));

		ini.iniFileLoad(pszFileName);
		ini.iniGetString("BootHex","Master",szFileName,sizeof(szFileName),"");
		if (strlen(szFileName) > 0)
		{
			m_bMaster = true;
		}
		//master
		if (m_bMaster)
		{		
			//取list中文件所在绝对路径
			if('/' == szFileName[0]) //list配置的文件是绝对路径
			{
				strcpy(arrNLC[0].szFileName,szFileName);
			}
			else{//list配置的文件是相对路径
				snprintf(arrNLC[0].szFileName,sizeof(arrNLC[0].szFileName),"%s%s",strRelativePub.c_str(),szFileName);
			}

			if(access(arrNLC[0].szFileName,F_OK) == -1 )  
			{
				CFunCommon::DlSprintf("Failed to open %s,please check!\n",arrNLC[0].szFileName);
				ini.iniFileFree();
				return -1;
			}
			strncpy(arrNLC[0].szNLCName,arrNLC[0].szFileName,sizeof(arrNLC[0].szNLCName)-1);
			if(!UnCompressListAuto(arrNLC[0].szFileName,0))
			{
				ini.iniFileFree();
				return -1;
			}
				
			snprintf(szIniFile,sizeof(szIniFile),"%s/unzip/0/newconfig.ini",g_szAppPath);
			ReadGroupAtt(szIniFile,0);
			ini.iniFileFree();
			
		}
		
		//slave	
	
		ini.iniFileLoad(pszFileName);
		m_nSlaveCount= ini.iniGetInt("BootHex","SlaveCount",0);	
		//CFunCommon::DlSprintf("m_nSlaveCount=%d\n",m_nSlaveCount);
		for(int i=1; i<=m_nSlaveCount; i++)
		{
			char szTmp[10] = {0};
			snprintf(szTmp,sizeof(szTmp),"Slave%d",i);
			ini.iniFileLoad(pszFileName);
			memset(szFileName,0,sizeof(szFileName));
			ini.iniGetString("BootHex",szTmp,szFileName,sizeof(szFileName),"");
			//取list中文件所在绝对路径
			if('/' == szFileName[0]) //list配置的文件是绝对路径
			{
				strncpy(arrNLC[i].szFileName,szFileName,sizeof(arrNLC[i].szFileName)-1);
			}
			else{//list配置的文件是相对路径
	
				snprintf(arrNLC[i].szFileName,sizeof(arrNLC[i].szFileName),"%s%s",strRelativePub.c_str(),szFileName);
			}

		
			//CFunCommon::DlSprintf("arrNLC[i].szFileName=%s\n",arrNLC[i].szFileName);
			
			if((access(arrNLC[i].szFileName,F_OK)) == -1)  
			{
				CFunCommon::DlSprintf("Failed:%s not exist,please check!\n",arrNLC[i].szFileName);
				ini.iniFileFree();
				return -1;
			}	

			strncpy(arrNLC[i].szNLCName,arrNLC[i].szFileName,sizeof(arrNLC[i].szNLCName)-1);

			if(!UnCompressListAuto(arrNLC[i].szFileName,i))
			{
				ini.iniFileFree();
				return -1;
			}
			char szFile[MAX_PATH+30] = {0};	
			snprintf(szFile,sizeof(szFile),"%s/unzip/%d/newconfig.ini",g_szAppPath,i);
			ReadGroupAtt(szFile,i);
		}
		ini.iniFileFree();
		return 0;
	}
		
}

/**
 * @brief	解压固件NLC包：命令行传入单个NLC的情况
 * @param[in] pszNLC 	NLC包绝对路径
 * @param[in] nIndex 	unzip目录下索引
 * @return
 * @li true		成功
 * @li false	失败
*/
bool CNlcDownload::UnCompressNLCAuto(const char* pszNLC,int nIndex)		
{
	FILE *fp = NULL;
	char szUnZipDir[MAX_PATH] = {0};
	char szFileHead[MAX_PATH] = {0};	
	char szCmd[MAX_PATH+20] = {0};
	char szIni[MAX_PATH] = {0};
	struct  timeval  start;
  	struct  timeval  end;


  	gettimeofday(&start,NULL);
	if (NULL == pszNLC)
		return false;
	
	fp = fopen(pszNLC,"rb");
	if (NULL == fp)
	{
		CFunCommon::DlSprintf("Failed to open %s ,Please Check!",pszNLC);
		return false;
	}
	fclose(fp);
	

	snprintf(szUnZipDir,sizeof(szUnZipDir),	"%s/unzip/nlctmp",g_szAppPath);
	snprintf(szCmd,sizeof(szCmd)," rm -rf %s",szUnZipDir);
	CFunCommon::ExecSystem(szCmd);
	snprintf(szCmd,sizeof(szCmd),"mkdir -p %s",szUnZipDir);
	CFunCommon::ExecSystem(szCmd);

	if(access(szUnZipDir,F_OK) != 0)
	{
		CFunCommon::DlSprintf("Failed to create nlctmp floder:%s.\n",strerror(errno));
		return false;
	}
	
	if (NULL == (fp = fopen(pszNLC, "rb")) ) 
	{
		CFunCommon::DlSprintf("Failed to open file.\n");
		return false;
	}

	fread(szFileHead, sizeof(unsigned char), 4, fp);
	if (memcmp(szFileHead, "\xAB\xCD\xEF\xAF", 4) != 0) 
	{
		CFunCommon::DlSprintf("Failed:File contents are illegal\n");
		fclose(fp);
		return false;
	}	


	if(chdir(szUnZipDir) == -1)
	{
		CFunCommon::DlSprintf("Failed:chdir error:%s.\n",strerror(errno));
		return false;
	}
		
//	CFunCommon::DlSprintf("Parsing %s ......",pszNLC);
	char szStatus[512];
	sprintf(szStatus,"Parsing %s ......",pszNLC);
	(*gCallBack)((char *)szStatus,-1);
	fflush(stdout);
	fread(szFileHead, sizeof(unsigned char), 1, fp);

	//此处64位与win32类型不同
	int  i,len, insize,fileNum=0;
	fileNum = szFileHead[0];
	FILE ** fouts = new PFILE[fileNum];
	unsigned int* oulen = new unsigned int[fileNum];
	memset(fouts, 0x00, fileNum);
	for (i = 0 ; i < fileNum; i ++) 
	{
		fread(szFileHead, sizeof(unsigned char), 1, fp);
		len = szFileHead[0];	
		fread(szFileHead, sizeof(unsigned char), len, fp);
		szFileHead[len] = 0;
		fread(&len, sizeof(unsigned int), 1, fp);
		oulen[i] = len;
		fouts[i] = fopen((char *)szFileHead, "wb");
	}
	len = ftell(fp);
	fseek(fp, 0, SEEK_END);
	insize = ftell(fp) - len - 20;
	fseek(fp, len, SEEK_SET);
	CLzss lzss;
		
	lzss.DeCompress(fp, insize,fouts, oulen, fileNum);	
	for (i = 0 ; i < fileNum; i ++) 
		fclose(fouts[i]);	

	fseek(fp, len+insize, SEEK_SET);
	unsigned char digest[20];
	fread(digest, sizeof(unsigned char), 20, fp);
	fclose(fp);
	delete[] fouts;
	delete[] oulen;
	#if 0
	CFunCommon::DlSprintf("\n");
	for (int n=0;n<20;n++ )
	{
		CFunCommon::DlSprintf("%02x ",digest[n]);
	}
	CFunCommon::DlSprintf("\n");
	unsigned char *aa = lzss.GetDigest();

	for ( int n=0;n<20;n++ )
	{
		CFunCommon::DlSprintf("%02x ",aa[n]);
	}
	CFunCommon::DlSprintf("\n");
	#endif
	if (memcmp(digest, lzss.GetDigest(), 20) != 0) 
	{
		CFunCommon::DlSprintf("Failed:File validation failed\n",0);
		return false;
	}
	snprintf(szIni,sizeof(szIni),"%s/newconfig.ini",szUnZipDir);
	if(access(szIni,F_OK) == 0)
	{
		gettimeofday(&end,NULL);	
	//	CFunCommon::DlSprintf("[ %us ]\n",end.tv_sec-start.tv_sec);
		return true;
	}
	return false;
}

/**
 * @brief	解压固件NLC包：命令行传入list配置的情况
 * @param[in] pszNLC  NLC文件
 * @param[in] nIndex unzip目录下索引
 * @return
 * @li true		成功
 * @li false	失败
*/
bool CNlcDownload::UnCompressListAuto(const char* pszNLC,int nIndex)		
{
	FILE *fp = NULL;
	char szUnZipDir[MAX_PATH] = {0};
	char szCmd[MAX_PATH+20] = {0};
	char szIni[MAX_PATH] = {0};
	struct  timeval  start;
  	struct  timeval  end;
  	gettimeofday(&start,NULL);
	if (NULL == pszNLC )
		return false;
	
	if(NULL == (fp=fopen(pszNLC,"rb")))
	{
		CFunCommon::DlSprintf("Failed to opening:%s ,Please Check!\n",pszNLC);	
		return false;
	}
	fclose(fp);
	
	snprintf(szUnZipDir,sizeof(szUnZipDir),"%s/unzip/%d",g_szAppPath,nIndex);
	snprintf(szCmd,sizeof(szCmd)," rm -rf %s/unzip/%d",g_szAppPath,nIndex);
	CFunCommon::ExecSystem(szCmd);
	snprintf(szCmd,sizeof(szCmd),"mkdir -p %s/unzip/%d",g_szAppPath,nIndex);
	CFunCommon::ExecSystem(szCmd);


	if(access(szUnZipDir,F_OK) != 0)
	{
		CFunCommon::DlSprintf("Failed to creating:%s.\n",szUnZipDir);
		return false;
	}
	char filehead[MAX_PATH];	
	if (NULL == (fp = fopen(pszNLC, "rb"))  ) 
	{
		CFunCommon::DlSprintf("Failed to opining:%s.\n",pszNLC);
		return false;
	}
	fread(filehead, sizeof(unsigned char), 4, fp);
	if (memcmp(filehead, "\xAB\xCD\xEF\xAF", 4) != 0) 
	{
		CFunCommon::DlSprintf("Failed: %s are illegal\n",szUnZipDir);
		fclose(fp);
		return false;
	}	

	if(chdir(szUnZipDir) == -1)
	{
		CFunCommon::DlSprintf("Failed:chdir error:%s.\n",strerror(errno));
		fclose(fp);
		return false;
	}
	//CFunCommon::DlSprintf("Parsing %s ......",pszNLC);
	fflush(stdout);
	fread(filehead, sizeof(unsigned char), 1, fp);
	
	
	int i, len,num, insize,fileNum=0;
	fileNum = filehead[0];
	num = fileNum;
	FILE ** fouts = new PFILE[num];
	unsigned int * oulen = new unsigned int[num];
	memset(fouts, 0x00, num);
	for (i = 0 ; i < num; i ++) 
	{
		fread(filehead, sizeof(unsigned char), 1, fp);
		len = filehead[0];	
		fread(filehead, sizeof(unsigned char), len, fp);
		filehead[len] = 0;
		fread(&len, sizeof(unsigned int), 1, fp);
		oulen[i] = len;
		fouts[i] = fopen((char *)filehead, "wb");
	}

	len = ftell(fp);
	fseek(fp, 0, SEEK_END);
	insize = ftell(fp) - len - 20;
	fseek(fp, len, SEEK_SET);

	CLzss lzss;
	lzss.DeCompress(fp, insize,fouts, oulen, fileNum);	
	for (i = 0 ; i < num; i ++) 
		fclose(fouts[i]);	

	fseek(fp, len+insize, SEEK_SET);
	unsigned char digest[20];
	fread(digest, sizeof(unsigned char), 20, fp);
	fclose(fp);
	delete[] fouts;
	delete[] oulen;
	if (memcmp(digest, lzss.GetDigest(), 20) != 0) 
	{
		CFunCommon::DlSprintf("Failed:File validation failed\n",0);
		return false;
	}
	snprintf(szIni,sizeof(szIni),"%s/newconfig.ini",szUnZipDir);
	if(access(szIni,F_OK) == 0)
	{
		gettimeofday(&end,NULL);	
	//	CFunCommon::DlSprintf("[ %us ]\n",end.tv_sec-start.tv_sec);
		return true;
	}
	else
		return false;

}

/**
 * @brief 生成6ul 下载文件列表
 * @param[in] strPath   解包后的bin文件所在路径 eg:g_szAppPath/unzip/0
 * @param[in] gfAtt     newconfig.ini中分组结构
 * @param[in] nIndex   unzip目录下索引
 * @return
 * @li true            成功
 * @li false          失败
*/
bool CNlcDownload::MakeDownFileString_6ul(string strPath,GROUP_FILE_ATT gfAtt,int nIndex)
{
	int  nCount = 0;
	char szTmp[5] = {0};
	string strSectorName, strFullPath,strDownList;

	nCount = arrNLC[nIndex].nBinFileCount;
	if (0 == nCount)
	{
		CFunCommon::DlSprintf("Failed:There is no file to download, please check on packing.\n");
		return false;
	}
		
	
	for (int k=0; k<gfAtt.GrAttitude.GroupCount; k++)
	{
		
		for (int i = 0; i < nCount; i++)
		{
			strSectorName = arrNLC[nIndex].szBiospara[i].szSector;
			strFullPath = arrNLC[nIndex].szBiospara[i].szFullname;
			//CFunCommon::DlSprintf("strFullPath =%s;\n",strFullPath.c_str());
			if (strFullPath == gfAtt.szGroupFile[k])
			{
				//CFunCommon::DlSprintf("%d;strSectorName =%s;strFullPath=%s\n",i,strSectorName.c_str(),strFullPath.c_str());
				//CFunCommon::DlSprintf("%d:gfAtt.szGroupFile[k]=%s\n",k,gfAtt.szGroupFile[k]);
				strDownList = "";
				strDownList += (strPath+strFullPath + '*');
				strcat(m_szDownFileList,strDownList.c_str());	
				sprintf(szTmp,"%d",k);
				strDownList = "";
				strDownList = szTmp;
				strDownList += "|";
				strcat(m_szDownFileList,strDownList.c_str());			
				break;
			}
		}
	}

	return true;
}

/**
 * @brief 6ul usb-serial通讯接收指定长度数据
 * @param[in] ucCmd    		协议命令
 * @param[out] pszData   	接收的数据
 * @param[in] nLen   		接收长度
 * @param[in] nTimeout   	超时时间
 * @return
 * @li 0            成功
 * @li 其它          失败
*/
int CNlcDownload::RecvOnePacket_6ul(unsigned char ucCmd,unsigned char* pszData,int nLen,int nTimeout)
{
	
	char szData[4096] = {0};
	int nRet = 0;
	int nTimes = nTimeout/10;
	//mac系统发送完文件后，会返回不定长数量的未知数据0，测试有1k多0，然后才返回有效数据
#if 0
	while (nTimes-- > 0)
	{
		if(1 != (nRet=m_objUsbSerial.ReadBuffData(szData, 1, 10)) )
		{
			//CFunCommon::DlSprintf("continue \n");
			if(nRet == -1)
			{
				CFunCommon::DlSprintf("Failed to read :%s\n",strerror(errno));
				return -2;
			}
			continue;
		}
		if(szData[0] == 0x02)
		{
			break;
		}
		//CFunCommon::DlSprintf("szData[0]:%02x ",szData[0]);
	}
#endif

	m_objUsbSerial.ReadBuffData(szData, 1, nTimeout);
    if ( 0x02 != szData[0] ) {
        CFunCommon::DlSprintf("Failed: STX received failed \n");
        return -2;
    }
	if (m_objUsbSerial.ReadBuffData(szData+1, CMD_HEAD_LEN-1, nTimeout) != (CMD_HEAD_LEN-1)){
//	if (m_objUsbSerial.ReadBuffData(szData, CMD_HEAD_LEN, nTimeout) != CMD_HEAD_LEN) {
        CFunCommon::DlSprintf("Failed to waiting a packet header  \n");
        return -1;
    }

    if (szData[1] != ucCmd) {
        CFunCommon::DlSprintf("Failed: CMD received not matched, get the left data. \n");
        return -3;
    }
    int reqlen = cmd_get_length(szData) + 2;
    nLen -= CMD_HEAD_LEN;
    if (reqlen > nLen) reqlen = nLen;
  //  CFunCommon::DlSprintf("reqlen = %d\n",reqlen);
    if (m_objUsbSerial.ReadBuffData((char*)szData + CMD_HEAD_LEN, reqlen, nTimeout) != reqlen) {
        CFunCommon::DlSprintf("Failed to waiting a packet body \n");
        return -4;
    }
    if (0x00 != szData[CMD_HEAD_LEN] ) {
		CFunCommon::DlSprintf("Failed: error status [%02X] received\n", szData[CMD_HEAD_LEN]);

		switch (szData[CMD_HEAD_LEN])
		{
		case 0x01:	
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_UNSUPPORTED\n");
			break;
		case 0x02:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_LENGTH\n");
			break;
		case 0x03:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_CERT\n");
			break;
		case 0x04:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_FIRM\n");
			break;
		case 0x05:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_FIRM_LEN\n");
			break;
		case 0x06:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_FIRM_HASH\n");
			break;
		case 0x07:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_CERT_FIELD\n");
			break;
		case 0x08:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_FLASH\n");
			break;
		case 0x09:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_UNKNOWN_PART\n");
			break;
		case 0x0A:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_CRC16_ERROR\n");
			break;
		case 0x0B:
			CFunCommon::DlSprintf("Failed:CMD_ERR_CODE_DWN_ADDR\n");
			break;
		default:
			break;
		}
        return -5;
    }
	memcpy(pszData,szData,CMD_HEAD_LEN+reqlen);
	m_objUsbSerial.FlushPort();
    return 0;
}

/**
 * @brief 6ul 固件下载
 * @param[in] pszFileList   待下载的文件列表
 * @return
 * @li true            成功
 * @li false          失败
*/
bool CNlcDownload::DownLoadFileList_6ul(const char* pszFileList)
{
	char szTotal[MAX_PATH*100] = {0};		
	char szDownFullPath[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char szDownFileName[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char szTmp[10] = {0};
	int szFileType[50] ={0};		
	int	nTotalFileCount = 0;	
	int	nDownFileLen = 0;			
	int	nLen = 0,i = 0,j = 0,nOffset = 0,nProgress = 0;
	float fProgress = 0.0f;

	memset(szFileType,0,sizeof(szFileType));
	strncpy(szTotal,pszFileList,sizeof(szTotal) -1);
	nDownFileLen = strlen(szTotal);

	//取出各个文件路径与文件名，文件数量
	for (i=0; i<nDownFileLen; i++)  
	{
		if ('*' == szTotal[i])
		{
			szTotal[i] = '\0';
			strncpy(szDownFullPath[nTotalFileCount],szTotal+nOffset,sizeof(szDownFullPath[nTotalFileCount])-1);
			szTotal[i] = '*';		
			for (j=strlen(szDownFullPath[nTotalFileCount])-1; j>0; j--)
			{
				if ('/' == szDownFullPath[nTotalFileCount][j])
				{
					strncpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1, sizeof(szDownFileName[nTotalFileCount])-1);
					break;
				}
			}		
			for (j=i; ; j++)
			{
				if ('|' == szTotal[j])
					break;
			}			
			memset(szTmp, 0, sizeof(szTmp));
			memcpy(szTmp, szTotal+i+1, j-i-1);
			szFileType[nTotalFileCount] = atoi(szTmp);			
			nOffset = j+1;			
			nTotalFileCount++;
		}
	}

	
	for (i=0; i<nTotalFileCount; i++)
	{
		
		U8 uszBuff[4096 + 64] = {0};
		uszBuff[0] = 0x02;
		uszBuff[1] = 0x02;
		uszBuff[2] = CMD_DWN_HEAD_LEN & 0x00FF;
        uszBuff[3] = (CMD_DWN_HEAD_LEN >> 8) & 0x00FF;
	//	CFunCommon::DlSprintf("Downloading %s \n",szDownFileName[i]);

		U32 unSendLen = 0;	
		U32 unHasDownLen = 0;	//已下载
		U32 unFrameSize = 4096;//发送数据帧大小设置为4k发送
		FILE *fp = NULL;
	//	CFunCommon::DlSprintf("Downloading %s \n",szDownFullPath[i]);
		if (NULL == (fp = fopen(szDownFullPath[i], "rb")))
		{
			CFunCommon::DlSprintf("Failed to opening %s. \n",szDownFullPath[i]);
			return false;
		}
		fseek(fp, 0, SEEK_END);
		nLen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fread(&uszBuff[4],1,HEADLENTH,fp);
		uszBuff[HEADLENTH+4+1] = 0;
		uszBuff[HEADLENTH+4+2] = 0;
		m_objUsbSerial.FlushPort();
		if(m_objUsbSerial.SendBuffData((char*) &uszBuff, 4+HEADLENTH+2) != (4+HEADLENTH+2))
		{
			fclose(fp);
			return false;
		}
		memset(uszBuff,0,sizeof(uszBuff));
		
		if (RecvOnePacket_6ul(0x02,uszBuff,sizeof(uszBuff),NLC_TIME_OUT) < 0 )
		{
			fclose(fp);
			return false;
		}
		
		m_objUsbSerial.SetBlock(true);
		nLen -= HEADLENTH;
		while(1)
		{
			unsigned char szTmpBuff[4*1024+64] = {0};
			unSendLen = (nLen-unHasDownLen)>unFrameSize?unFrameSize:(nLen-unHasDownLen);		
			if (0 == unSendLen)
			{
				break;
			}
				
			fseek(fp, unHasDownLen+HEADLENTH, SEEK_SET);
			memset(szTmpBuff, 0, sizeof(szTmpBuff));
			fread(szTmpBuff, unSendLen, 1, fp) ;
			
			if (unSendLen != m_objUsbSerial.SendBuffData((char*)&szTmpBuff, unSendLen))
			{
				CFunCommon::DlSprintf("Failed to send.\n");
				fclose(fp);
				return false;
			}
			
			unHasDownLen += unSendLen;	
			fProgress = 0.1f*unHasDownLen*1000/(nLen);
			nProgress = fProgress; 
		//	CFunCommon::DlSprintf("   --> Downloading %s......   %3d%%\033[1A\r\n",szDownFileName[i],nProgress); 	
			(*gCallBack)(szDownFileName[i],nProgress);								
		}
		fclose(fp);
		memset(uszBuff,0,sizeof(uszBuff));
		m_objUsbSerial.SetBlock(false);

		if (RecvOnePacket_6ul(0x02, uszBuff, sizeof (uszBuff), NLC_TIME_OUT*3) < 0 )
		{
			return false;
		}	
	//	CFunCommon::DlSprintf("        --> Downloading %s......   %d%%\n",szDownFileName[i],100);
		(*gCallBack)(szDownFileName[i],100);	
		usleep(1000*1000);
	}
	
	U8 uszReboot[6];
    uszReboot[0] = 0x02;
    uszReboot[1] = 0x01;
    uszReboot[2] = 0x00;
    uszReboot[3] = 0x00;
    uszReboot[4] = 0x00;
    uszReboot[5] = 0x00;
	
	if (m_objUsbSerial.SendBuffData((char*)&uszReboot, sizeof(uszReboot)) != sizeof(uszReboot))
	{
		CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_SENDERROR]);
		
		return false;
	}
		
	CFunCommon::DlSprintf("%s\n",DOWN_SUCC);
	return true;
}

/**
 * @brief	NLC下载主控流程
 * @param[in] pszFileList 下发文件列表
 * @param[in] pszDev usb-serial设备号
 * @return
 * @li 0    成功
 * @li -1    失败
 * @li 2 	表示下载了boot需要重启设备到固件下载状态继续下载;
*/
int CNlcDownload::DownloadProcess(const char* pszFileList,const char* pszDev)
{

	string strCurFirmPath,csTmp;
	string m_csLog;
	bool bFind = false; //是否有查找到匹配的组
	bool bBoot = false;//对于master表示：是否重启，与this->aboot配合判断aboot是否下载，6ul 底座都没有重启
	bool bNewLogic = false;
	char szBuff[256] = {0};
	char szIn[256] = {0};
	char szOut[256] = {0};
	char szConfigVer[128] = {0};
	char szDevicePath[50] = {0};

	int nVerLen=0;

	int nIndex = -1, i = 0, j = 0;
	strCurFirmPath = g_szAppPath;
	strCurFirmPath += "/unzip/0/";

	if (NULL == pszFileList || NULL ==  pszDev  )
	{
		CFunCommon::DlSprintf("Failed:param vaild.\n");
		return -1;
	}
	
	chdir(g_szAppPath);

	snprintf(szDevicePath,sizeof(szDevicePath),"/dev/%s",pszDev);
	//CFunCommon::DlSprintf("DevicePath: %s\n",szDevicePath);
	if(!m_objUsbSerial.OpenPort(szDevicePath))
	{
		return -1;
	}
	if(sType_6UL == m_cDownType)
	{
		//6ul只有一个固件包，不会有重启过程
		if(!MakeDownFileString_6ul(strCurFirmPath,arrNLC[0].szGroupFileAtt[0],0))
			return -1;
	//	CFunCommon::DlSprintf("m_szDownFileList=%s\n",m_szDownFileList);
		if(!DownLoadFileList_6ul(m_szDownFileList))
			return -1;
		return 0;
	}
	//master
	if (arrNLC[0].bIsUsed)
	{
		//新配置文件以后的版本，旧配置的包，新工具直接不支持报错
		if (0 != arrNLC[0].nFirmPackVer) 
		{			
			nIndex = -1;
			//只有整包的固件才会执行reload动作
			if (0 == arrNLC[0].nFullPack) 
			{
				 //更新pos信息
				if (!RefreshDevice()) 
				{
					CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_BIOS_REFRESH]);
					return -1;
				}
				//CFunCommon::DlSprintf("%s\n",m_szBootVer);
				memset(m_szDownFileList, 0, sizeof(m_szDownFileList));
				for (i=0; i<arrNLC[0].nGroupCount; i++)
				{
					nVerLen = strlen(m_szBootVer);
					memset(szConfigVer,0,sizeof(szConfigVer));
					strcpy(szConfigVer,arrNLC[0].szGroupFileAtt[i].GrAttitude.szGroupVer);	
					//pos上送版本和配置文件中的机型进行比对，找到对应的下载分组group		
					if (strncmp(szConfigVer,m_szBootVer,nVerLen-3) == 0)						
					{						
							bFind = true;
							nIndex = i;
							//判断有boot文件需要先下载boot文件，然后重启
							if (strncmp(szConfigVer+nVerLen-3,m_szBootVer+nVerLen-3,4) != 0)
							{
								bBoot = true;
								break;
							}
							else
							{
								bBoot = false;
								break;
							}
						}													
				}  				
				//未找到机型匹配的组，查找是否有配置为空串的组
				if (!bFind)
				{
					nIndex = -1;
					//如果没有空串这个组，提示报错，有则下载
					for (i=0; i<arrNLC[0].nGroupCount; i++)
					{
						memset(szConfigVer, 0, sizeof(szConfigVer));
						strcpy(szConfigVer,arrNLC[0].szGroupFileAtt[i].GrAttitude.szGroupVer);						
						//空串
						if (strlen(szConfigVer) == 0)
							{
								bFind = true;
								nIndex = i;
								bBoot = false;
								break;
							}			
					}  
					if (-1 == nIndex)
					{		
						CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_MACHINE_BOOT]); //boot不匹配
						return -1;
					}
				}

			   //需要下载boot，但是配置没有找到boot文件
				if ((0 == arrNLC[0].szGroupFileAtt[i].GrAttitude.iBoot) && bBoot)
				{
						CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_BOOTFILE_LOST]);//boot文件缺失			
						return -1;
						
				}
				 //需要下载boot，且有boot文件，则下载boot文件
				else if ((1 == arrNLC[0].szGroupFileAtt[i].GrAttitude.iBoot) && bBoot) 
				{
					if(MakeBootDownFile(arrNLC[0].szGroupFileAtt[nIndex].szGroupFile[0],0) != 0)	
					{							
						return -1;
					}
					//CFunCommon::DlSprintf("Downloading %s \n",arrNLC[0].szFileName);
					if (!DownloadFile(m_szDownFileList))
					{
						return -1;
					}
					if (!ReLoadDefaultPart())	//下载完boot发送重载命令，擦除kernel
					{
						CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_RELOAD_PRAT]);	
						return -1;
					}					
					if (ReBoot())
					{
						return 2;  	
					}				
				}
				 //下载固件
				if (!bBoot)
				{
					if (!ReLoadDefaultPart())		//每次先reload一下，确保pos上送的信息是最新的
					{
						CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_RELOAD_PRAT]);	
						return -1;
					}

					if (!RefreshDevice())  //更新pos信息
					{
						CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_BIOS_REFRESH]);
						return -1;
					}
					MakeBiosDownFile(arrNLC[0].szGroupFileAtt[nIndex],0);

				//	CFunCommon::DlSprintf("Downloading %s \n",arrNLC[0].szFileName);
					if (!DownloadFile(m_szDownFileList))
					{
						return -1;
					}
				}
				
			}
			
			else  // 非整包固件
			{
				if (!RefreshDevice())  //更新pos信息
				{
					CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_BIOS_REFRESH]);
					return -1;
				}
				nIndex = -1;
				bFind = false;
				for (i=0; i<arrNLC[0].nGroupCount; i++)
				{
					nVerLen=strlen(m_szBootVer);
					memset(szConfigVer,0,sizeof(szConfigVer));
					strcpy(szConfigVer,arrNLC[0].szGroupFileAtt[i].GrAttitude.szGroupVer);
					if (strncmp(szConfigVer, m_szBootVer, nVerLen-3) == 0)	 		
					{						
							bFind = true;
							nIndex = i;
							break;
					}
									
				}  

				if (-1 == nIndex)
				{
					bFind = false;
					bBoot = false;
					for (i=0; i<arrNLC[0].nGroupCount; i++)
					{
						memset(szConfigVer,0,sizeof(szConfigVer));
						strcpy(szConfigVer,arrNLC[0].szGroupFileAtt[i].GrAttitude.szGroupVer);			
						if (strlen(szConfigVer)==0)
						{
							bFind = true;
							nIndex = i;
							bBoot = false;
							break;
						}				
					}  
							
					if (-1 == nIndex)
					{				
						CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_NLC]);
						return -1;
						
					}
				}

				MakeBiosDownFile(arrNLC[0].szGroupFileAtt[nIndex],0);
				//CFunCommon::DlSprintf("Downloading %s \n",arrNLC[0].szFileName);
				if (!DownloadFile(m_szDownFileList))
				{
					return -1;
				}				
			}	
			

		}  
				
		else{
			CFunCommon::DlSprintf("Failed:Package format is too old. Please repack.\n");
			return -1;
		}
	}
	
	for (j=1; j<=m_nSlaveCount; j++)  
	{	
	//	CFunCommon::DlSprintf("m_nSlaveCount = %d\n",m_nSlaveCount);
		snprintf(szBuff,sizeof(szBuff),"%s/unzip/%d/",g_szAppPath,j);
		memset(m_szDownFileList,0,sizeof(m_szDownFileList));
		if (arrNLC[j].bIsUsed)
		{
			//newconfig.ini有配置group
			if (0 != arrNLC[j].nFirmPackVer) 
			{
				bFind = false;
				if (!RefreshDevice())  
				{
					CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_BIOS_REFRESH]);						
					return -1;
				}
				nIndex = -1;
				for (i=0; i<arrNLC[j].nGroupCount; i++)
				{
					memset(szConfigVer,0,sizeof(szConfigVer));
					strcpy(szConfigVer,arrNLC[j].szGroupFileAtt[i].GrAttitude.szGroupVer);
					//	CFunCommon::DlSprintf("szConfigVer=%s,m_szBootVer=%s\n",szConfigVer,m_szBootVer);
					if (strncmp(szConfigVer,m_szBootVer,strlen(m_szBootVer)-3) == 0)
					{
							bFind = true;
							nIndex = i;
							break;
					}													
				}  
				if (-1 == nIndex)
				{
					for (i=0;i<arrNLC[j].nGroupCount;i++)
					{
						memset(szConfigVer,0,sizeof(szConfigVer));
						strcpy(szConfigVer, arrNLC[j].szGroupFileAtt[i].GrAttitude.szGroupVer);					
						if (strlen(szConfigVer) == 0)	
						{
								bFind = true;
								nIndex = i;
								bBoot = false;
								break;
						}				
					}  
					if (-1 == nIndex)
					{		
						CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_BIOS_REFRESH]);
						return -1;
					}
				}
				MakeBiosDownFile(arrNLC[j].szGroupFileAtt[nIndex],j);
			//	CFunCommon::DlSprintf("Downloading %s \n",arrNLC[j].szFileName);
	
				if (!DownloadFile(m_szDownFileList))
				{
					CFunCommon::DlSprintf("Failed to Download slave. \n");
					return -1;
				}							

			}  

			//newconfig.ini未配置group
			else
			{
				if (!RefreshDevice())  
				{
					CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_BIOS_REFRESH]);						
					return -1;
				}			
				MakeBiosDownFile_old(szBuff,j);			
			//	CFunCommon::DlSprintf("Downloading %s \n",arrNLC[j].szFileName);
			//	CFunCommon::DlSprintf("m_szDownFileList = %s\n",m_szDownFileList);
				if (!DownloadFile(m_szDownFileList))
				{
					CFunCommon::DlSprintf("Failed to Download slave. \n");
					return -1;
				}	
								
			}				
	
		}
	}
	ReBoot();
	CFunCommon::DlSprintf("%s\n",DOWN_SUCC);
	return 0;
}

/**
 * @brief	等待设备接入，并进入固件下载状态
 * @param[in] pszDev   设备号
 * @return
 * @li 0         成功
 * @li-1         失败
*/
int CNlcDownload::WaitForConnect(const char *pszDev)
{

	char szCmd[512] = {0};
	char szOut[512] = {0};
	int nTimeOut = 30;
	chdir(g_szAppPath);
	while (nTimeOut-- > 0)
	{
		if(!m_objUsbSerial.OpenPort(pszDev))
		{
			usleep(1000*1000);
			continue;
		}
		m_objUsbSerial.ClosePort();
		return 0;
	}
	return -1;
	
}


/**
 * @brief	下载流程说明：
	1、PC-->POS:填充包格式如下：0x02,0x00,nLen1(从命令码开始到文件末尾的长度),	0xE1(下载命令),nLen2(从0x90开始到包的结尾),0x90,分区信息长度,分区内容(56字节),0x91,0x04,文件长度(4字节)
	2、POS-->PC：0x02,0x00,0x03,0x99,0x01,0x00 
	3、PC-->POS：循环发送文件内容，4096*4字节发送,最末一包要附上4字节的CRC校验码
	4、POS-->PC：0x02,0x00,0x03,0x99,0x01,0x00(成功指示标志) 
	0x00:成功 
	0x01:数据tlv解析错误 
	0x02:长度信息获取失败
	0x03:在pos端无法获取到对应分区信息
	0x04:获取数据文件出错
	0x05:crc校验出错
	0x06:签名校验出错
	0x07:写flash错误
	0x08:要修改的分区为只读分区
	0x09:flash坏块数过多
	0x0A:无对应分区信息
	0x11:数据格式错误
 * @param[in] pszBiosFile   下载文件列表
 * @return
 * @li true            成功
 * @li false          失败
*/
bool CNlcDownload::DownloadFile(const char* pszBiosFile)
{
	int i = 0, j = 0,
	nRet = 0, nLen = 0,
	nTotalFileCount = 0,	//下载的文件总数
	nOffset = 0 ;
	unsigned int
		nFileSize = 0,		//文件的长度
		nPartSize = 0,		//分区的空间大小
		nCrc32 = 0,			//CRC的结果存储
		nHaveRead = 0,		//已经读取的文件长度
		nRead = 0,			//当包读取的文件长度
		nCrcLenleft = 4;	//CRC的长度

	char szTotal[4096] = {0};			//用于存储文件列表
	char szDownFullPath[MAX_DOWNFILE_NUM][MAX_PATH] = {0};	//存储下载的列表文件的全路径
	char szDownFileName[MAX_DOWNFILE_NUM][MAX_PATH] = {0};	//存储单纯的文件名
	int	 szPartIndex[MAX_DOWNFILE_NUM] = {0};			//存储分区的信息的下标
	char szTmp[10] = {0};
	unsigned char szPacket[MAX_FILE_PACKET] = {0};
	unsigned char szCrc[4] = {0};
	unsigned char szReceive[8] = {0};
	FILE *fp = NULL;

	memset(szTotal, 0, sizeof(szTotal));
	strcpy(szTotal, pszBiosFile);
	nLen = strlen(szTotal);

	for (i=0;i<nLen;i++)  
	{
		if ('*' == szTotal[i] )
		{
			szTotal[i] = '\0';
			strcpy(szDownFullPath[nTotalFileCount],szTotal+nOffset); 
			szTotal[i] = '*';
			
			for (j=strlen(szDownFullPath[nTotalFileCount])-1; j>0; j--)
			{
				if ('/' == szDownFullPath[nTotalFileCount][j])
				{
					strcpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1);
					break;
				}
			}

			for (j=i;;j++)
			{
				if ('|' == szTotal[j])
					break;
			}

			nOffset = j+1;
			memset(szTmp,0,sizeof(szTmp));
			strncpy(szTmp,szTotal+i+1,j-i-1);
			szPartIndex[nTotalFileCount]=atoi(szTmp); 
			nTotalFileCount++;
		}
	}

	for (i=0; i<nTotalFileCount; i++)		
	{	
		if (NULL == (fp=fopen(szDownFullPath[i],"rb")))
		{
			CFunCommon::DlSprintf("Failed to open %s.\n",szDownFullPath[i]);
			return false;
		}
		else
		{
			fseek(fp,0,SEEK_END);
			nFileSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
		}
		memset(szPacket, 0, sizeof(szPacket));
		szPacket[0] = 0x02;
		szPacket[1] = 0x00;
		szPacket[3] = CMD_DOWNLOAD;
		szPacket[5] = 0x90; 

		if (m_beNewBios)
		{
			nPartSize = m_ptrpart->part[szPartIndex[i]].end_address - m_ptrpart->part[szPartIndex[i]].start_address;			
			szPacket[6] = PARTSIZE;			//分区信息长度			
			memcpy(szPacket + 7, (unsigned char *)(&m_ptrpart->part[szPartIndex[i]]), PARTSIZE);
			szPacket[7 + PARTSIZE] = 0x91;	//文件长度标志
			szPacket[8 + PARTSIZE] = 0x04; 	//文件长度存储区域的长度
			CCodeTools::InttoC4(nFileSize, szPacket + 9 + PARTSIZE);
			szPacket[4] = 8 + PARTSIZE;	
		}
		else
		{		
			nPartSize=m_ptrpart->partold[szPartIndex[i]].end_address - m_ptrpart->partold[szPartIndex[i]].start_address;			
			szPacket[6] = PARTSIZE_OLD;			
			memcpy(szPacket + 7, (unsigned char *)(&m_ptrpart->partold[szPartIndex[i]]), PARTSIZE_OLD);
			szPacket[7 + PARTSIZE_OLD] = 0x91; 	//文件长度标志
			szPacket[8 + PARTSIZE_OLD] = 0x04;  //文件长度存储区域的长度
			CCodeTools::InttoC4(nFileSize, szPacket + 9 + PARTSIZE_OLD);
			szPacket[4] = 8 + PARTSIZE_OLD;	
		}

		if (nFileSize > nPartSize) //文件的大小和分区的大小进行比较
		{
			CFunCommon::DlSprintf("Failed:filesize larger partsize\n");
			fclose(fp);		
			return false;
		}
		

		szPacket[2] = szPacket[4] + 2; //从命令码开始的长度
		nLen = szPacket[2] + 3;	//下载信息包的总长度
		m_objUsbSerial.FlushPort();
		nRet = m_objUsbSerial.SendBuffData((char *)szPacket,nLen);
		if (nRet != nLen)
		{
			CFunCommon::DlSprintf("Failed:send error\n");
			fclose(fp);
			return false;
		}
		nRead = 3;
		nHaveRead = 0;
		memset(szPacket,0,sizeof(szPacket));		
		nRet = m_objUsbSerial.ReadBuffData((char *)szPacket,nRead,NLC_TIME_OUT);
		if (nRet != nRead)
		{
			CFunCommon::DlSprintf("Failed:read %d:%d\n",nRead,nRet);
			fclose(fp);
			return false;
		}
		
		if (0x02 == szPacket[0]) 
		{
			nHaveRead += nRead;
			nRead = CCodeTools::bcd2int(szPacket + 1);
			nRet=m_objUsbSerial.ReadBuffData((char *)szPacket+nHaveRead,nRead,NLC_TIME_OUT);
			if (0 == nRet)
			{		
				fclose(fp);
				return false;
			}
		}

		if (memcmp(szPacket, "\x02\x00\x03\x99\x01\x00", 6) != 0) 
		{
			CFunCommon::DlSprintf("Failed:Illegal package.\n",nRead,nRet);
			fclose(fp);
			return false;
		}
	
		nHaveRead = 0;
		nCrc32 = 0;
		nCrcLenleft = 4;
		float fProgress = 0.0f;
		int nProgress = 0;
		m_objUsbSerial.SetBlock(true);
		while (nHaveRead != nFileSize) 
		{

			memset(szPacket,0,sizeof(szPacket));			
			nRead = MAX_FILE_PACKET;
			
			if (nRead > (nFileSize-nHaveRead))
				nRead = nFileSize-nHaveRead;
			
			fread(szPacket,nRead,1,fp);
			nCrc32 = CCodeTools::crc32_org(nCrc32, szPacket, nRead);			
			nHaveRead+=nRead;
			
			if (nRead < MAX_FILE_PACKET) 
			{
				memcpy(szCrc, (unsigned char *)(&nCrc32), 4);
				nCrcLenleft = MAX_FILE_PACKET - nRead;
				
				if (nCrcLenleft > 4) 						
					nCrcLenleft = 4;
				
				memcpy(szPacket + nRead, szCrc, nCrcLenleft);
				nRead += nCrcLenleft;
				nCrcLenleft = 4 - nCrcLenleft;
			}		
		//	m_objUsbSerial.FlushPort();
			nRet = m_objUsbSerial.SendBuffData((char *)szPacket,nRead);
			if (nRet != nRead)
			{
				CFunCommon::DlSprintf("Failed send error at pos:%d\n",ftell(fp));
				fclose(fp);
				return false;
			}
			//CFunCommon::DlSprintf("nFileSize=%d,nHaveRead=%d\n",nFileSize,nHaveRead);
			fProgress=0.1f*nHaveRead*1000/(nFileSize);
			nProgress=fProgress; 
		//	CFunCommon::DlSprintf("   --> Downloading %s......   %3d%%\033[1A\n",szDownFileName[i],nProgress); 
			(*gCallBack)(szDownFileName[i],nProgress);	
			usleep(1000);					

		}
		
	//	CFunCommon::DlSprintf("        --> Downloading %s......   %d%%\n",szDownFileName[i],100);
		(*gCallBack)(szDownFileName[i],100);	
		if (0 != nCrcLenleft) 
		{ 
			if (4 == nCrcLenleft) 
			{
				memcpy(szCrc, (unsigned char *)(&nCrc32), 4);
			}
			
			memcpy(szPacket, szCrc + 4 - nCrcLenleft, nCrcLenleft);
		//	m_objUsbSerial.FlushPort();
			nRet = m_objUsbSerial.SendBuffData((char *)szPacket,nCrcLenleft);
			if (nRet != nCrcLenleft)
			{
				fclose(fp);
				return false;
			}
		}
		
		//CFunCommon::DlSprintf("        --> Verify and write partitions ......       ");
        fflush(stdout);
		memset(szReceive,0xff,sizeof(szReceive));
		m_objUsbSerial.SetBlock(false);
		nRet = m_objUsbSerial.ReadBuffData((char *)szReceive,6,NLC_TIME_OUT*5);
		if(6 != nRet)
		{
			CFunCommon::DlSprintf("Failed:ReadBuffData Verify and write partitions  error\n");
			fclose(fp);
			return false;
		}
	
		if (memcmp(szReceive, "\x02\x00\x03\x99\x01\x00", 6) != 0) 
		{	
			CFunCommon::DlSprintf("Failed:[%d][%s]:%s ! ", i+1, szDownFileName[i], g_szMessage[DOWN_ERR_BIOS_UPDATE]);
			fclose(fp);
			return false;	
		}
		usleep(100);
		fclose(fp);
	}

	
	return true;
		
}



/**
 * @brief	刷新设备，获取设备boot等信息
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNlcDownload::RefreshDevice(void)	
{
	U8 szPacket[2048] = {0};
	int nLen = 0,nRet = 0;
	int nRead = 0,nHaveRead = 0;
	tlvobject pobj[64];
	int nPos = 0,i = 0;
	char szText[256] = {0};
	char *p = NULL;
	
	szPacket[0] = 0x02;
	szPacket[1] = 0x00;
	szPacket[2] = 0x02;
	szPacket[3] = CMD_REFRESHDEVICE;
	szPacket[4] = 0x00;
	nLen = 5;
	
	szPacket[2] = szPacket[4] + 2;
	nLen = szPacket[2] + 3;
	m_objUsbSerial.FlushPort();
	nRet = m_objUsbSerial.SendBuffData((char *)szPacket,nLen);
	
	if (nRet != nLen)
	{
		CFunCommon::DlSprintf("Failed to RefreshDevice:%s\n",strerror(errno));
		return false;
	}		
	
	nRead = 3;
	nHaveRead = 0;
	nRet = m_objUsbSerial.ReadBuffData((char *)szPacket + nHaveRead,nRead,NLC_TIME_OUT);
	if (nRet != nRead)
	{
		CFunCommon::DlSprintf("Failed:read timeout:%s\n",strerror(errno));
		return false;
	}
	
	if (0x02 == szPacket[0]) 
	{
		nHaveRead += nRead;
		nRead = CCodeTools::bcd2int(szPacket + 1);
		nRet = 0;
		nRet = m_objUsbSerial.ReadBuffData((char *)szPacket + nHaveRead,nRead,NLC_TIME_OUT);
		if (nRet != nRead)
		{
			CFunCommon::DlSprintf("Failed:read timeout:%s\n",strerror(errno));
			return false;
		}
		
	}

	if (CCodeTools::emvtlv_decode((U8 *)szPacket+3, nRead, pobj, 64, STRING_TLVOBJ) < 0) 
	{
		CFunCommon::DlSprintf("Failed:%s\n",g_szMessage[DOWN_ERR_BIOS_ANALYZE_TLV]);

		return false;
	}
	
	if ((nPos = CCodeTools::emvtlv_get(0, 0x80, pobj, SEARCH_ONLY_SON))) 
	{
		memset(szText,0,sizeof(szText));
		memset(m_szBootVer,0,sizeof(m_szBootVer));
		strcpy(szText, (char *)pobj[nPos].pvalue);
		strcpy(m_szBootVer, (char *)pobj[nPos].pvalue);
		int n = CFunCommon::GetIndexOf(szText,"*");
		if (-1 != n)
		{
			memset(m_szBootVer,0,sizeof(m_szBootVer));
			memcpy(m_szBootVer,szText,n+4);
		}

		
	}
	
	if ((nPos = CCodeTools::emvtlv_get(0, 0x81, pobj, SEARCH_ONLY_SON))) 
	{}	
	if ((nPos = CCodeTools::emvtlv_get(0, 0x82, pobj, SEARCH_ONLY_SON))) 
	{
		memset(szText,0,sizeof(szText));
		strcpy(szText, (char *)pobj[nPos].pvalue);		
		
		if (NULL != (p=strstr(szText,"730")) )
		{
			//macType = 0;		
		}
		else if (NULL != (p=strstr(szText,"710")) )
		{
			//macType = 1;		
		}
		else if (NULL != (p=strstr(szText,"8510")))
		{
			//macType = 2;		
		}
	}

	if ((nPos = CCodeTools::emvtlv_get(0, 0x83, pobj, SEARCH_ONLY_SON))) 
	{
		memset(szText,0,sizeof(szText));
		strcpy(szText, (char *)pobj[nPos].pvalue);	
		
		//if (NULL != (p=strstr(szText,"64MB")))
		//	is64MB = 0;
		//else
		//	is64MB = 1;
	}
	
	if ((nPos = CCodeTools::emvtlv_get(0, 0x84, pobj, SEARCH_ONLY_SON))) 
	{
		memset(szText,0,sizeof(szText));
		strcpy(szText, (char *)pobj[nPos].pvalue);
	}
	
	if ((nPos = CCodeTools::emvtlv_get(0, 0x85, pobj, SEARCH_ONLY_SON))) 
	{
		memset(szText,0,sizeof(szText));
		strcpy(szText, (char *)pobj[nPos].pvalue);
	}
	
	if ((nPos = CCodeTools::emvtlv_get(0, 0x86, pobj, SEARCH_ONLY_SON))) 
	{
		//CFunCommon::DlSprintf("pobj[nPos].valuelen %d :sizeof (part_t) %d",pobj[nPos].valuelen,sizeof (part_t));
		if (pobj[nPos].valuelen % sizeof (part_t))	
		{
			
			m_beNewBios = false;
		}
		
		if (m_beNewBios)
		{
			
			int items = pobj[nPos].valuelen / sizeof (part_t);
		
			m_ptrpart->total = items;
			
			part_t * ppart;
			
			if (m_partinfo) 
			{
				free (m_partinfo);
				m_partinfo = NULL;
			}
			m_partinfo = (char*)malloc(pobj[nPos].valuelen); 
			memcpy(m_partinfo, pobj[nPos].pvalue, pobj[nPos].valuelen);
			ppart = (part_t *)(m_partinfo);
			
			for (i = 0; i < items; i ++, ppart ++) 
			{
				strcpy(m_ptrpart->part[i].name, ppart->name);		// 0413
				strncpy(m_ptrpart->part[i].version, ppart->version, MAX_VERSION_LEN - 1);	// 0413
				
				if (strcmp(ppart->name, "boot") == 0)
				{

				}
				//be315 = Is315Dev(m_ptrpart->part[i].version);
				
				m_ptrpart->part[i].start_address = ppart->start_address;		// 0413
				m_ptrpart->part[i].end_address = ppart->end_address;
				m_ptrpart->part[i].flag = ppart->flag;
				m_ptrpart->part[i].writelen = ppart->writelen;			
			}			
			
		}
		else
		{
			
			int items = pobj[nPos].valuelen / sizeof (part_t_old);
			m_ptrpart->total = items;
			part_t_old * ppart;
			
			if (m_partinfo) 
			{
				free (m_partinfo);
				m_partinfo = NULL;
			}
			
			m_partinfo = (char*)malloc(pobj[nPos].valuelen);
			memcpy(m_partinfo, pobj[nPos].pvalue, pobj[nPos].valuelen);
			
			ppart = (part_t_old *)(m_partinfo);
			
			for (i = 0; i < items; i ++, ppart ++) 
			{
				strcpy(m_ptrpart->partold[i].name, ppart->name);		
				
				m_ptrpart->partold[i].start_address = ppart->start_address;
				m_ptrpart->partold[i].end_address = ppart->end_address;
				m_ptrpart->partold[i].flag = ppart->flag;
				m_ptrpart->partold[i].writelen = ppart->writelen;						
			}
		}
	}
	
	if ((nPos = CCodeTools::emvtlv_get(0, 0x87, pobj, SEARCH_ONLY_SON))) 
	{
		
	}	
	
	return true;
} 

/**
 * @brief	重启设备到开机状态
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNlcDownload::ReStart(void)		
{
	U8 szPacket[10] = {0};
	int nLen;
	szPacket[0] = 0x02;
	szPacket[1] = 0x00;
	szPacket[2] = 0x02;
	szPacket[3] = CMD_LINUXRUN;
	szPacket[4] = 0x00;
	nLen = 5;
	if (m_objUsbSerial.SendBuffData((char *)szPacket,nLen) != nLen)
	{
		CFunCommon::DlSprintf("Failed to start system:%s\n",strerror(errno));
		return false;
	}	
	
	return true;
}


/**
 * @brief	下载boot后，重启到固件下载
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNlcDownload::ReBoot(void)	
{
	unsigned char szPacket[10]={0};
	int nLen;
	szPacket[0] = 0x02;
	szPacket[1] = 0x00;
	szPacket[2] = 0x02;
	szPacket[3] = CMD_REBOOT;
	szPacket[4] = 0x00;
	nLen = 5;
	if (m_objUsbSerial.SendBuffData((char *)szPacket,nLen) != nLen)
	{
		CFunCommon::DlSprintf("Failed to reboot :%s\n",strerror(errno));
		return false ;
	}	
	
	return true;	
}

/**
 * @brief	重载缺省分区
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNlcDownload::ReLoadDefaultPart(void)	
{
	U8 szPacket[10] = {0};
	int nLen = 0,nRet = 0,nRead = 0,nHaveRead = 0;
	
	szPacket[0] = 0x02;
	szPacket[1] = 0x00;
	szPacket[2] = 0x02;
	szPacket[3] = CMD_RELOAD;
	szPacket[4] = 0x00;
	nLen = 5;

	nRet = m_objUsbSerial.SendBuffData((char *)szPacket,nLen);
	
	if (nRet != nLen)
	{
		CFunCommon::DlSprintf("Failed to reload part :%s\n",strerror(errno));
		return false;
	}	

	nRead = 3;
	nHaveRead = 0;
	nRet = m_objUsbSerial.ReadBuffData((char *)szPacket + nHaveRead,nRead,NLC_TIME_OUT);

	if (nRet != nRead)
	{
		return false;
	}
	
	if (0x02 == szPacket[0] ) 
	{
		nHaveRead += nRead;
		nRead = CCodeTools::bcd2int(szPacket + 1);
		nRet = m_objUsbSerial.ReadBuffData((char *)szPacket + nHaveRead,nRead,NLC_TIME_OUT);
		if (nRet != nRead)
		{
			return false;
		}
		if (memcmp(szPacket, "\x02\x00\x03\x99\x01\x00", 6) != 0) 
		{
			return false;
		}
	}	
	return true;
}

/**
 * @brief 生成 bios下载文件列表,旧版本配置的打包格式，newconfig.ini中没有配置group分组，目前非完整包，入logo都还是使用旧配置
 * @param[in] pszFPath   文件路径
 * @param[in] nIndex   unzip下目录索引
 * @return
 * @li 0            成功
 * @li 其它          失败
*/
int CNlcDownload::MakeBiosDownFile_old(char* pszFPath,int nIndex)
{
	int nCount;
	string strSectorName, strFullName;
	char szTmp[MAX_PATH+20] = {0};
	nCount = arrNLC[nIndex].nBinFileCount;
	if (0 == nCount )
		return -2;

	for (int i = 0; i < nCount; i++)
	{
		strSectorName = arrNLC[nIndex].szBiospara[i].szSector;
		strFullName =arrNLC[nIndex].szBiospara[i].szFullname;
		for (int j = 0; j < m_ptrpart->total; j++)
		{
			string strPartName = m_ptrpart->part[j].name;
			if ((strSectorName == strPartName && !(m_ptrpart->part[j].flag & PART_RDONLY)))
			{
					memset(szTmp,0,sizeof(szTmp));
					snprintf(szTmp,sizeof(szTmp),"%s%s*%d|",pszFPath,strFullName.c_str(),j);
					strcat(m_szDownFileList,szTmp);
					break;
			}				
		}
	}
	return 0;
}

/**
 * @brief 生成 bios下载文件列表
 * @param[in] gfAtt   分组结构信息
 * @param[in] nIndex   unzip下目录索引
 * @return
 * @li 0            成功
 * @li 其它          失败
*/
int CNlcDownload::MakeBiosDownFile(GROUP_FILE_ATT gfAtt,int nIndex)
{
	int nCount = 0;
	char szTmp[MAX_PATH+20] = {0};
	char szSectorName[128] = {0};
	char szPartName[128]= {0}; 
	char szFullPath[MAX_PATH] = {0}; 
	char szFilePath[MAX_PATH]= {0}; 

	nCount = arrNLC[nIndex].nBinFileCount;
	
	//CFunCommon::DlSprintf("count=%d,nIndex=%d\n",count,nIndex);
	if (0 == nCount )
	{
		return -1;
	}
	//CFunCommon::DlSprintf("gfAtt.GrAttitude.GroupCount=%d\n",gfAtt.GrAttitude.GroupCount);
	for (int k=0; k<gfAtt.GrAttitude.GroupCount; k++)
	{
		for (int i = 0; i < nCount; i++)
		{
			memset(szFullPath,0,sizeof(szFullPath));
			memset(szSectorName,0,sizeof(szSectorName));				
			strcpy(szSectorName,arrNLC[nIndex].szBiospara[i].szSector);
			strcpy(szFullPath,arrNLC[nIndex].szBiospara[i].szFullname);
		//	CFunCommon::DlSprintf("szSectorName=%s,szFullPath=%s\n",szSectorName,szFullPath);
			if (strcmp(szFullPath,gfAtt.szGroupFile[k]) == 0)
				break;
		}

		for (int j = 0; j < m_ptrpart->total; j++)
		{
			//CFunCommon::DlSprintf("m_ptrpart->total=%d\n",m_ptrpart->total);
			memset(szPartName,0,sizeof(szPartName));
			strcpy(szPartName , m_ptrpart->part[j].name);
			snprintf(szFilePath,sizeof(szFilePath),"%s/unzip/%d/",g_szAppPath,nIndex);
			//CFunCommon::DlSprintf("szSectorName=%s,szPartName=%s,m_ptrpart->part[j].flag=%d\n",szSectorName,szPartName,m_ptrpart->part[j].flag);		
			if ((strcmp(szSectorName,szPartName) == 0) && !(m_ptrpart->part[j].flag & PART_RDONLY))
			{
				memset(szTmp,0,sizeof(szTmp));
				snprintf(szTmp,sizeof(szTmp),"%s%s*%d|",szFilePath,szFullPath,j);
				
				strcat(m_szDownFileList,szTmp);
			//	CFunCommon::DlSprintf("m_szDownFileList = %s\n",m_szDownFileList);
				break;
			}
		}
		//CFunCommon::DlSprintf("%s%s*%d|",arrNLC[nIndex].szFilePath,szFullPath,4);
	}

	return 0;
}

/**
 * @brief 生成 boot下载文件列表
 * @param[in] pszBootName    boot文件名
 * @param[in] nIndex   	 unzip下目录索引
 * @return
 * @li 0            成功
 * @li 其它          失败
*/
int CNlcDownload::MakeBootDownFile(const char *pszBootName,int nIndex)
{
	int i = 0;
	char szSector[128] = {0};
	char szPartName[128] = {0};
//	CFunCommon::DlSprintf("BootName=%s,nIndex=%d\n",pszBootName,nIndex);
	if (0 == arrNLC[nIndex].nBinFileCount ) 
		return -1;
	
	for (i = 0; i < arrNLC[nIndex].nBinFileCount; i++)
	{
 		if (strcmp(pszBootName,arrNLC[nIndex].szBiospara[i].szFullname)==0)
		{
			memset(szSector,0,sizeof(szSector));
			strcpy(szSector,arrNLC[nIndex].szBiospara[i].szSector);
			break;
		}
	}

	for (i = 0; i < m_ptrpart->total; i++)
	{
		strcpy(szPartName,m_ptrpart->part[i].name);

		if ((strcmp(szPartName,szSector) == 0) && !(m_ptrpart->part[i].flag & PART_RDONLY))
		{
			memset(m_szDownFileList,0,sizeof(m_szDownFileList));
			snprintf(m_szDownFileList,sizeof(m_szDownFileList),"%s%s*%d|",arrNLC[nIndex].szFilePath,pszBootName,i);
			break;
		}				
	}
	
	return 0;
}


