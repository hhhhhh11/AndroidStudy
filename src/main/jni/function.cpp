#include"function.h"
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<ctype.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include<dlfcn.h>
#include<sys/wait.h>
#include<time.h>
//#include<iconv.h>  
#include <iostream>
#include <errno.h>
#include<stdarg.h>

/**
 * @brief 获取应用程序所在绝对路径
 * @param[out] pszAbsolutePath    返回路径
 * @return
 * @li true           成功
 * @li false          失败
*/
bool CFunCommon::GetAbsolutePath (char* pszAbsolutePath) 
{ 
	FILE *fstream = NULL;    
	char szBuff[1024] = {0} ;  
	memset(szBuff,0,sizeof(szBuff));  
	if (NULL == pszAbsolutePath)
	{
		CFunCommon::DlSprintf("Failed to access.\n",strerror(errno));	 
		return false;
	}
	
	if(NULL == (fstream=popen("pwd","r")))	 
	{	
		CFunCommon::DlSprintf("Failed:execute command failed: %s",strerror(errno));	 
		return false;	  
	}	
	if(fgets(szBuff, sizeof(szBuff), fstream) != NULL) {
	
			memcpy(pszAbsolutePath,szBuff,strlen(szBuff)-1);
	}
	pclose(fstream);  
	return true;	


//linux 获取路径方法在mac并没有用，需要用的xcode或oc可以获得，本项目暂时通过系统命令获取
#if 0
	char buf[ MAXBUFSIZE ]; 
	int count; 
	char *p = NULL;
	if(NULL == strAbsolutePath)
		return -1;
	count = readlink( "/proc/self/exe", buf, MAXBUFSIZE ); 
	if ( count < 0 || count >= MAXBUFSIZE ) 
	{ 
		CFunCommon::DlSprintf( "Failed\n" ); 
		return -1; 
	} 
	buf[ count ] = '\0'; 
	memset(strAbsolutePath,0,sizeof(strAbsolutePath));
	p=buf;
	p=strrchr(buf,'/');
	if(p!=NULL)
		*p='\0';
	strcpy(strAbsolutePath,buf);
	return 0; 
	#endif

} 


/**
 * @brief 拷贝文件
 * @param[in] pszSrc    源文件
 * @param[in] pszDst   目标文件
 * @return
 * @li true           成功
 * @li false          失败
*/
bool CFunCommon::CopyFile(const char* pszSrc,const char* pszDst)
{
	char szBuff[1024] = {0};
	int nLen = 0;
	FILE *pfIn = NULL,*pfOut = NULL;

	if (NULL == pszSrc || NULL == pszDst)
	{
		CFunCommon::DlSprintf("Failed to assert.\n");
		return false;
	}
	
	if(NULL == (pfIn=fopen(pszSrc,"r")))
	{
		CFunCommon::DlSprintf("Failed to opening %s.\n",pszSrc);
		return false;
	}
	if(NULL ==  (pfOut=fopen(pszDst,"w")))
	{
		CFunCommon::DlSprintf("Failed to opening %s.\n",pszDst);
		return false;
	}

	while((nLen=fread(szBuff,1,1024,pfIn)) > 0 ){
		fwrite(szBuff,1,nLen,pfOut);
	}
	fclose(pfOut);
	fclose(pfIn);
}




/**
 * @brief 运行system命令
 * @param[in] pszCmd    要运行的命令
 * @return
 * @li true           成功
 * @li false          失败
*/
bool CFunCommon::ExecSystem(const char* pszCmd)
{

	int nRet = 0;
	if(NULL == pszCmd)
	{
		CFunCommon::DlSprintf("Failed to assert.\n");
		return false;
	}
	nRet = system(pszCmd);
	if(-1 == nRet)
	{
		CFunCommon::DlSprintf("Failed to running %s\n",pszCmd);
		return -1;
		
	}
	if (WIFEXITED(nRet))  
    {  
		if (0 != WEXITSTATUS(nRet) )  
		{
			//CFunCommon::DlSprintf("Failed to running %s\n",pszCmd);
			CFunCommon::DlSprintf("Failed to run \"%s\" exit code: %d\n",pszCmd, WEXITSTATUS(nRet));
			return false;
		}		 
    }  
	return true;
}



/**
 *@brief 递归删除目录下所有子目录与文件 
 *@param[in] pszdir    要删除的路径
 *@return
 *@li true           成功
 *@li false          失败
*/
bool CFunCommon::DeleteDir(const char* pszdir)
{
    char cur_dir[] = ".";
    char up_dir[] = "..";
    char dir_name[128];
    DIR *dirp;
    struct dirent *dp;
    struct stat dir_stat;

    // 参数传递进来的目录不存在，直接返回
    if ( 0 != access(pszdir, F_OK) ) {
        return true;
    }

    // 获取目录属性失败，返回错误
    if ( 0 > stat(pszdir, &dir_stat) ) {
        perror("Failed to get directory stat");
        return false;
    }
	// 普通文件直接删除
    if ( S_ISREG(dir_stat.st_mode) ) {  
        remove(pszdir);
    }
	// 目录文件，递归删除目录中内容 
	else if ( S_ISDIR(dir_stat.st_mode) ) 
	{   
        dirp = opendir(pszdir);
        while ( (dp=readdir(dirp)) != NULL ) {
            // 忽略 . 和 ..
            if ( (0 == strcmp(cur_dir, dp->d_name)) || (0 == strcmp(up_dir, dp->d_name)) ) 
			{
                continue;
            }

            sprintf(dir_name, "%s/%s", pszdir, dp->d_name);
            DeleteDir(dir_name);   // 递归调用
        }
        closedir(dirp);

        rmdir(pszdir);     // 删除空目录
    } else {
        perror("unknow file type!");    
    }
    return true;
}


/**
 *@brief 删除string中指定字符串
 *@param[in][out] strSrc    源string串
 *@param[in] strMark   要删除的串
 *@return void
*/
void CFunCommon::DeleteIndexString(string& strSrc,  const string strMark)  
{  
    size_t nSize = strMark.size();  
    while(1)  
    {  
        size_t nPos = strSrc.find(strMark);   
        if(nPos == string::npos)  
        {  
            return;  
        }  
        strSrc.erase(nPos, nSize);  
    }  
}  


 /**
 *@brief  字符替换
 *@param[in] pszSrc    传入的字符串
 *@param[in] oldChar    被替换的字符
 *@param[in] newChar  替换后的字符
 *@return         替换的字符数
*/
int  CFunCommon::ReplaceAnsi(char* pszSrc,char cOld,char cNew)
{

	int nCount = 0;
	int nLen = 0;
	if( NULL == pszSrc)
	{
		CFunCommon::DlSprintf("Failed to rplace.\n");
		return -1;
	}
	nLen = strlen(pszSrc);
	char *pszTmp = 	new char[nLen+1];
	if (NULL == pszTmp)
	{
		CFunCommon::DlSprintf("Failed to new.\n");
		return -1;
	}
	memset(pszTmp,0,sizeof(char)*(nLen+1));
	strcpy(pszTmp,pszSrc);
	for(int  i=0; i<strlen(pszTmp); i++)
	{
		if(cOld == pszTmp[i])
		{
			pszTmp[i]=cNew;
			nCount++;
		}
		
	}
	strcpy(pszSrc,pszTmp);
	delete pszTmp;
	pszTmp = NULL;
	return nCount;
}


 /**
 *@brief  查找字符串中子串位置，实现类似C++ string::indexof功能
 *@param[in] pszSrc    传入的字符串
 *@param[in] pszMark    查找的子串符
 *@param[in] newChar  替换后的字符
 *@return         子串的索引位置
*/
int CFunCommon::GetIndexOf( char* pszSrc, char* pszMark )  
{  
    int i=0;  
	if (NULL == pszSrc || NULL == pszMark)
	{
		return -1;
	}
    char *p = strstr(pszSrc,pszMark);  
    if(NULL == p)  
	{
		 return -1;  
	}
    
	while(pszSrc != p)  
	{  
		pszSrc++;  
		i++;  
	}  
    
    return i;  
} 


/**
 *@brief 删除字符串空格
 *@param[in]  lpStr    传入需要去除空格字符串，返回删除空格字符串
 *@return
 *@li 0            删除成功
 *@li -1          删除失败
*/
 int CFunCommon::Trim(char* pszStr)
{
    if(NULL == pszStr)
        return -1;
	int nLen = strlen(pszStr);
	char* pTmp = new char[nLen+1];
	if (NULL == pTmp)
	{
		CFunCommon::DlSprintf("Failed to new.\n");
		return -1;
	}
	memset(pTmp,0,sizeof(char)*(nLen+1));
    char* ptr = pszStr;
    int i = 0;
    while(*ptr) 
	{
        if(!isspace(*ptr)){
            pTmp[i] = *ptr;
            i++;
        }
        ptr++;
    }
	nLen = strlen(pTmp);
   
    strcpy(pszStr,pTmp);
	pszStr[nLen] = '\0';
	if (NULL != pTmp)
	{
		delete pTmp;
		pTmp = NULL;
	}
	

    return 0;
}


/**
 *@brief 从绝对路径取出文件名称
 *@param[in] pszPath    传入的绝对路径
 *@param[out] szFileName  传出的文件名称
 *@return
 *@li 0            成功
 *@li -1          失败
*/
int CFunCommon::GetAbsolutePathFileName(char* pszPath,char* pszFileName)
{
	if (NULL == pszPath || NULL == pszFileName)
	{
		return -1;
	}
	char *p=strrchr(pszPath,'/');
	if (NULL == p)
	{
		strcpy(pszFileName,pszPath);
		return 0;
	}
	else
	{
		strcpy(pszFileName,p+1);
		return 0;
	}
}


/**
 *@brief 目录下搜索文件
 *@param[in]  strFolderPath    搜索的目录名
 *@param[in] strFile  搜索的的文件名称
 *@param[out] strFile  返回搜索到文件的绝对路径
 *@return 
 *@li true           成功
 *@li false        失败

*/
bool CFunCommon::GetFileNameFromFolder(string strFolderPath,string& strFile)
{
     DIR *dir;
	 bool bSucc = false;
     struct dirent *ptr;
	 string strRet;
     char base[1024] = {0};
	 char szFile[1024] = {0};
	 if("" == strFolderPath || "" == strFile)
	 	return false;
     if ((dir=opendir(strFolderPath.c_str())) == NULL)
     {
         CFunCommon::DlSprintf("Failed to Open dir %s",strFolderPath.c_str());
         return false;
     }
     while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name,"..") == 0)    ///current dir OR parrent dir
             continue;
        else if(ptr->d_type == 8)    ///file
        {
			// ptr->d_name 为绝对路径，取出文件名
			memset(szFile,0,sizeof(szFile));
			GetAbsolutePathFileName(ptr->d_name,szFile);
		 //	CFunCommon::DlSprintf("strFile=%s,szFile:%s\n",strFile.c_str(),szFile);
			if (strFile == szFile)
			{
				strFile = strFolderPath + "//" + ptr->d_name;
				strRet = ptr->d_name;
				bSucc = true;
				return true;
			}
	  	}
        else if(ptr->d_type == 10)    ///link file
           //  CFunCommon::DlSprintf("d_name:%s/%s\n",strFolderPath.c_str(),ptr->d_name);
           	;
        else if(ptr->d_type == 4)    ///dir
        {
             memset(base,'\0',sizeof(base));
             strcpy(base,strFolderPath.c_str());
             strcat(base,"/");
             strcat(base,ptr->d_name);
			 if(GetFileNameFromFolder(base,strFile))
			 {
				 strRet = strFile;
				 bSucc = true;
			 }
            
        }
    }
     closedir(dir);
	 strFile = strRet;
     return bSucc;
}



/**
 * @brief  字符替换
 * @param pszSrc    传入的字符串
 * @param cOld    被替换的字符
 * @param cNew  替换后的字符
 * @return 替换的字符数量
*/
int  CFunCommon::ReplaceAll(char* pszSrc,char cOld,char cNew)
{

	int nCount = 0,nLen = 0;
	if(NULL == pszSrc)
		return -1;
	nLen = strlen(pszSrc);
	char *pszTmp = new char(nLen+1);

	strcpy(pszTmp,pszSrc);
	for(int i=0; i<strlen(pszTmp); i++)
	{
		if(cOld == pszTmp[i])
		{
			pszTmp[i] = cNew;
			nCount++;
		}
	}
	strcpy(pszSrc,pszTmp);
	delete pszTmp;
	pszTmp = NULL;
	return nCount;
}


/**
 * @brief  自定义printf，原来命令行工具，信息直接打印到stdout，改成so后，改为写入文件，再读出，所有原来的标准输出打印printf改为调用DlSprintf
 *   //qt 工程调用的，标准输出改为仍然调用DlSprintf接口，信息直接追加到全局变量g_szResult
*/
extern char g_szResult[1024];

int CFunCommon::DlSprintf(const char * format, ...)
{
	char szBuff[4096] = {0};
    va_list ap;
    int ret = -1;
    va_start(ap, format);
	ret = vsprintf(szBuff, format, ap);
    va_end(ap);


    FILE* fp = NULL;
	int nFileSize = 0;
	strcat(g_szResult,szBuff);
	#if 0
    fp = fopen(g_szLogFile, "a");
	if (fp != NULL) 
	{	
		fwrite(szBuff,strlen(szBuff),1,fp);
		fclose(fp);
	}
	else{
	}
	#endif
    return ret;
}


#if 0
/**

 *@brief ����Ŀ¼��ȡ��Ŀ¼�������ļ�

 *@param strFolderPath    ����ľ���·��
 
 *@return void

*/
void GetFileNameFromFolder(char *strFolderPath)
{
     DIR *dir;
      struct dirent *ptr;
     char base[1000];
     if ((dir=opendir(strFolderPath)) == NULL)
     {
         perror("Open dir error...");
         exit(1);
     }
     while ((ptr=readdir(dir)) != NULL)
    {
         if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
             continue;
         else if(ptr->d_type == 8)    ///file
         {
		// CFunCommon::DlSprintf("d_name:%s/%s\n",strFolderPath,ptr->d_name);
	 	 strcpy(szFileName[nSubFileCount++],ptr->d_name);	
	  }
         else if(ptr->d_type == 10)    ///link file
           //  CFunCommon::DlSprintf("d_name:%s/%s\n",strFolderPath,ptr->d_name);
           	;
         else if(ptr->d_type == 4)    ///dir
         {
             memset(base,'\0',sizeof(base));
             strcpy(base,strFolderPath);
             strcat(base,"/");
             strcat(base,ptr->d_name);
             GetFileNameFromFolder(base);
         }
    }
     closedir(dir);
     return ;
}


/**

 *@brief ����Ļ�ϴ�ӡ��Ϣ

 *@param strMessage    ����ӡ������

*/
void LanguageShow(char *strMessage)
{
	char Buff[1024];
	memset(Buff,0,sizeof(Buff));
	strcpy(Buff,strMessage);
	CFunCommon::DlSprintf("%s",Buff);
}

/**

 *@brief ��ȡ�ļ�����

 *@param strPath    ����ȡ���ļ�����

 *@param strContent   ���� ��ȡ���ļ�����
 
 *@return

 *@li >0            ��ȡ�ɹ�

 *@li -1          ��ȡʧ��

*/
#if 0
int ReadFileContent(char *strPath, char *strContent)
{
	
	struct stat stat_buf;
	char cCon[1024*25];
	int nFilelen=0,fd;
	memset(cCon,0,sizeof(cCon));
	if (strContent==NULL)
		return -1;
	Trim(strPath);
	 if (stat(strPath, &stat_buf) < 0)
	        return -1;
	nFilelen = (int)stat_buf.st_size;
	fd = open(strPath, O_RDONLY);
	if(nFilelen > sizeof(cCon));
		nFilelen = sizeof(cCon);
    	nFilelen = read(fd, cCon, nFilelen);
	close(fd);
	memcpy(strContent,cCon,sizeof(cCon));
	return nFilelen; 
}
#endif
#if 0
int GetfilePath(char * strFileName)
{
	char szContenBuff[1024*25]={0};
	char szCopyBuff[1024*25]={0};
	char szBuff[1024*25]={0};
	char strMessage[256] ={0};
	char *p = NULL;
	int i=0,nlen;
	num=0;
	if(0 == strlen(strUptPath))
		return -1;
	//CFunCommon::DlSprintf("strAbsolutePath=%s\n",strAbsolutePath);
	//CFunCommon::DlSprintf("strUptPath=%s\n",strUptPath);
	chdir(strUptPath); 
	if((access(strFileName,F_OK))==-1)  
	{
		sprintf(strMessage,"\n  %s doesn't  exist!\n",strFileName);
		LanguageShow(strMessage);
		return FAIL;
	}
	nlen = ReadFileContent(strFileName,szContenBuff);
	if (-1== nlen)
	{
		sprintf(strMessage,"\n  Read %s fail!\n",strFileName);
		LanguageShow(strMessage);
		return FAIL;
	}
	//LanguageShow(szContenBuff);
	while (1)
	{
		p=strstr(szContenBuff,"\n");
		if (p==NULL)
		{
			
			i=0;
			int nLen = strlen(szContenBuff);
			if (nLen==0)
			{
				break;
			}
			while (1)
			{
				if (szContenBuff[i]==' ')
				{
					i++;
				}
				else
				{
					memset(szBuff,0,sizeof(szBuff));
					memcpy(szBuff,szContenBuff+i,strlen(szContenBuff)-i);
					memset(szContenBuff,0,sizeof(szContenBuff));
					strcpy(szContenBuff,szBuff);
					break;
				}
			}
			//CFunCommon::DlSprintf("szContenBuff=%s\r\n",szContenBuff);
			if (-1==SaveArray(szContenBuff))
			{
				p = NULL;
				return -1;
			}
			break;
		}
		if ((p-szContenBuff)==0)
		{
			memset(szCopyBuff,0,sizeof(szCopyBuff));
			strcpy(szCopyBuff,p+1);
			memset(szContenBuff,0,sizeof(szContenBuff));
			strcpy(szContenBuff,szCopyBuff);
			continue;
		}
		memset(szCopyBuff,0,sizeof(szCopyBuff));
		memcpy(szCopyBuff,szContenBuff,p-szContenBuff);
		memset(szBuff,0,sizeof(szBuff));
		i=0;
		while (1)
		{
			if (szCopyBuff[i]==' ')
			{
				i++;
			}
			else
			{
				memcpy(szBuff,szCopyBuff+i,strlen(szCopyBuff)-i);
				memset(szCopyBuff,0,sizeof(szCopyBuff));
				strcpy(szCopyBuff,szBuff);
				break;
			}
		}
		//CFunCommon::DlSprintf("szCopyBuff=%s\r\n",szCopyBuff);
		if (-1==SaveArray(szCopyBuff))
		{
			p = NULL;
			return -1;
		}
		memset(szCopyBuff,0,sizeof(szCopyBuff));
		strcpy(szCopyBuff,p+1);
		memset(szContenBuff,0,sizeof(szContenBuff));
		strcpy(szContenBuff,szCopyBuff);
	}
	p = NULL;
	chdir(strAbsolutePath);
	return 0;
}

#endif

/**

 *@brief дcontrol�ļ�

 *@param strBuff   Ҫд������
 
 *@return

 *@li 0    �ɹ�

 *@li -1    ʧ��
*/
 int WriteFileContent(char *csFilePath, char *csContent)
{
	int fd;
	if(csContent==NULL)
		return -1;
	if((fd = open(csFilePath, O_RDWR|O_CREAT|O_TRUNC, 0666))<0)
	{
		LanguageShow("create file control fail\n");
		return -1;
	}
	write(fd, csContent, strlen(csContent));
	close(fd);
	return 0;
	
}


int GetFileSize(char *szFilePath,char *szSize)
{
	struct stat stat_buf;
	char strsize[20]={0};
	int nFilelen=0;
	 if (stat(szFilePath, &stat_buf) < 0)
	 {	
	 	CFunCommon::DlSprintf("File %s cannot be accessed, please check",szFilePath);
	 	exit(0);
	 }
	nFilelen = stat_buf.st_size;
	sprintf(strsize,"%d",nFilelen);
	strcpy(szSize,strsize);
	return 0;
}



int GetFileReleaseDate(char *szFilePath, char *szDateBuff)
{
	struct stat statbuf; 
	struct tm *tblock; 
	if (szDateBuff == NULL )
	{
		return -1;
	}

	if (0 != stat(szFilePath, &statbuf))
	{
		return -1;
	}
	else
	{
		tblock=localtime(&statbuf.st_atime);
		sprintf(szDateBuff,"%d/%d/%d",tblock->tm_mday,tblock->tm_mon+1,tblock->tm_year+1900);
	}

	return 0;
}

int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)  
{  
#if 0

        iconv_t cd;  
	
        int rc;  
        char **pin = &inbuf;  
        char **pout = &outbuf;  
  		size_t sinlen,soutlen;
		sinlen = inlen;
		soutlen = outlen;
	
        cd = iconv_open(to_charset,from_charset);  
				
        if (cd==0)  
                return -1;  
        memset(outbuf,0,outlen);  
        if (iconv(cd,pin,&sinlen,pout,&soutlen) == -1)  
                return -1;  
        iconv_close(cd);  
	#endif
        return 0;  
}  
  
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)  
{  
        return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);  
}  
  
int g2u(char *inbuf,int inlen,char *outbuf,int outlen)  
{  
        return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);  
} 





/*��ȡsrc�ַ�����,���±�Ϊstart��ʼ��end-1(endǰ��)���ַ���������dest��(�±��0��ʼ)*/  
void substring(char *dest,char *src,int start,int end)  
{  
    int i=start;  
    if(start>strlen(src))return;  
    if(end>strlen(src))  
        end=strlen(src);  
    while(i<end)  
    {     
        dest[i-start]=src[i];  
        i++;  
    }  
    dest[i-start]='\0';  
    return;  
} 


int IsAbsolutePath(char *strPath)
{
	//����·��
	if(strPath[0] == '/')
	{
		return 0;
	}
	else
		return -1;
	
}


	
#endif
