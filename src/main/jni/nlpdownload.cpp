#include "function.h"
#include "inirw.h"
//#include "libusb.h"
#include "common.h"
#include "arq.h"
#include "nlpdownload.h"
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <errno.h>
#include <dlfcn.h>
#include <sys/time.h>
#include<dirent.h>


extern char g_szAppPath[1024] ; 
//#define GETNLPDEVICE  "cu.usbmodemXXXX"
//libusb_device_handle *device = NULL;
#if 0
//mac 下libusb只有64位，而下载程序只能32bit(原来PC端打包工具等都是win32程序，若mac下载程序64bit编译，则不兼容，会有各种错误，入lzss解压后不匹配等)
#define PRODUCT_ID 0x9120
#define VERDOR_ID  0x05c6
static int LIBUSB_CALL usb_arrived_callback(struct libusb_context *ctx, struct libusb_device *dev,    libusb_hotplug_event event, void *userdata)
{   
	struct libusb_device_handle *handle;   
	struct libusb_device_descriptor desc;    
	unsigned char buf[512];   
	int rc;   
	libusb_get_device_descriptor(dev, &desc);   
	CFunCommon::DlSprintf("Add usb device: \n");   
	CFunCommon::DlSprintf("\tCLASS(0x%x) SUBCLASS(0x%x) PROTOCOL(0x%x)\n",    
	desc.bDeviceClass, desc.bDeviceSubClass, desc.bDeviceProtocol);    
	CFunCommon::DlSprintf("\tVENDOR(0x%x) PRODUCT(0x%x)\n", desc.idVendor, desc.idProduct);    
	rc = libusb_open(dev, &handle);    
	if (LIBUSB_SUCCESS != rc) {        
		CFunCommon::DlSprintf("Could not open USB device\n");       
		return 0;    
	}    
	memset(buf, 0, sizeof(buf));   
	rc = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, buf, sizeof(buf));    
	if (rc < 0) {        
		CFunCommon::DlSprintf("Get Manufacturer failed\n");    
	} else {       
		CFunCommon::DlSprintf("\tManufacturer: %s\n", buf);    
	}    
	memset(buf, 0, sizeof(buf));    
	rc = libusb_get_string_descriptor_ascii(handle, desc.iProduct, buf, sizeof(buf));    
	if (rc < 0) {        
		CFunCommon::DlSprintf("Get Product failed\n");    
	} else {       
		CFunCommon::DlSprintf("\tProduct: %s\n", buf);    
	}    memset(buf, 0, sizeof(buf));   
	rc = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, buf, sizeof(buf));    
	if (rc < 0) {        
		CFunCommon::DlSprintf("Get SerialNumber failed\n");    
	} else {        
		CFunCommon::DlSprintf("\tSerialNumber: %s\n", buf);   
	}    
		libusb_close(handle);  
		return 0;
}

static int LIBUSB_CALL usb_left_callback(struct libusb_context *ctx, struct libusb_device *dev,libusb_hotplug_event event, void *userdata)
{    
	struct libusb_device_descriptor desc;    
	libusb_get_device_descriptor(dev, &desc);   
	CFunCommon::DlSprintf("Remove usb device: CLASS(0x%x) SUBCLASS(0x%x) iSerialNumber(0x%x)\n",       
	desc.bDeviceClass, desc.bDeviceSubClass, desc.iSerialNumber);    
	return 0;
}




static int  print_devs(libusb_device **devs)
{
	libusb_device *dev;
	int i = 0, j = 0;
	uint8_t path[8]; 

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			return -1;
		}
		if(desc.idVendor != VERDOR_ID ||desc.idProduct!= PRODUCT_ID )
			continue;
		CFunCommon::DlSprintf("%04x:%04x (bus %d, device %d)",desc.idVendor, desc.idProduct,	libusb_get_bus_number(dev), libusb_get_device_address(dev));
		r = libusb_get_port_numbers(dev, path, sizeof(path));
		if (r > 0) {
			CFunCommon::DlSprintf(" path: %d", path[0]);
			for (j = 1; j < r; j++)
				CFunCommon::DlSprintf(".%d", path[j]);
		}
		CFunCommon::DlSprintf("\n");
		return 0;
	}
	return -1;
}



static int usbinit()
{
	libusb_hotplug_callback_handle usb_arrived_handle;    
	libusb_hotplug_callback_handle usb_left_handle;    
	libusb_context *ctx;    
	int rc;    
	//libusb_init(&ctx);   
	CFunCommon::DlSprintf("libusb_init");
	rc = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,LIBUSB_HOTPLUG_NO_FLAGS, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,LIBUSB_HOTPLUG_MATCH_ANY, usb_arrived_callback, NULL, &usb_arrived_handle);   
	if (LIBUSB_SUCCESS != rc) {        
		CFunCommon::DlSprintf("Error to register usb arrived callback\n");        
		goto failure;    
	}    
	rc = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,LIBUSB_HOTPLUG_NO_FLAGS, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY, usb_left_callback, NULL, &usb_left_handle);    
	if (LIBUSB_SUCCESS != rc) {        
		CFunCommon::DlSprintf("Error to register usb left callback\n");       
		goto failure;    
	} 
	while (1) 
	{        
		libusb_handle_events_completed(ctx, NULL);        
		usleep(1000);   
	}   
	libusb_hotplug_deregister_callback(ctx, usb_arrived_handle);    
	libusb_hotplug_deregister_callback(ctx, usb_left_handle);    
	libusb_exit(ctx);    
	return 0;
failure:    
		libusb_exit(ctx);    
		return EXIT_FAILURE;
}



// Uses POSIX functions to send and receive data from a Maestro.
// NOTE: You must change the 'const char * device' line below.




// Gets the position of a Maestro channel.
// See the "Serial Servo Commands" section of the user's guide.
static int maestroGetPosition(int fd, unsigned char channel)
{
  unsigned char command[] = {0x90, channel};
  if(write(fd, command, sizeof(command)) == -1)
  {
    CFunCommon::DlSprintf("error writing");
    return -1;
  }

  unsigned char response[2];
  if(read(fd,response,2) != 2)
  {
    perror("error reading");
    return -1;
  }

  return response[0] + 256*response[1];
}

// Sets the target of a Maestro channel.
// See the "Serial Servo Commands" section of the user's guide.
// The units of 'target' are quarter-microseconds.
int maestroSetTarget(int fd, unsigned char channel, unsigned short target)
{
  unsigned char command[] = {0x84, channel, target & 0x7F, target >> 7 & 0x7F};
  if (write(fd, command, sizeof(command)) == -1)
  {
    perror("error writing");
    return -1;
  }
  return 0;
}


#endif



/**
 * @brief	默认构造函数
*/
CNLPDownload::CNLPDownload()
{
	m_nFileCount = 0;
	memset(&m_nFileType,0,sizeof(m_nFileType));		
	memset(&m_szDownFullPath,0,sizeof(m_szDownFullPath));						
}

/**
 * @brief	默认析构函数
*/
CNLPDownload::~CNLPDownload()
{

}



/**

 * @brief 目录unnzp下指定zip包
 * @param[in]  strFolderPath    unnzp目录
 * @param[in] strFile  搜索的的文件名称
 * @return 
 * @li true           成功
 * @li false        失败
*/
bool CNLPDownload::FindZIPFile(string strFolderPath,string strFile)
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
			CFunCommon::GetAbsolutePathFileName(ptr->d_name,szFile);

			if (strFile == szFile)
			{
				m_strDownZipTmp = strFolderPath + "/" + ptr->d_name;
				//CFunCommon::DlSprintf("m_strDownZipTmp=%s\n",m_strDownZipTmp.c_str());
				bSucc = true;
				return true;
			}
	 		//strcpy(szFileName[nSubFileCount++],ptr->d_name);	
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
			 if(FindZIPFile(base,strFile))
			 {	
				 bSucc = true;
			 }
            
        }
    }
     closedir(dir);
	 strFile = strRet;
     return bSucc;
}


/**
 * @brief	解压ota或zip包
 * @param[in] pszZipFile 待解压的文件
 * @return	void
*/
void CNLPDownload::UnpackZipFile(const char *pszZipFile)
{
	string strTmpPath,strTmpFileName,strTmp;

	int nPos = -1;
	char szCmd[512] = {0};
	OTA_ZIP_INFO otainfo;		//ota包的信息
   // CFunCommon::DlSprintf("pszZipFile=%s\n\n",pszZipFile);
	strTmp=pszZipFile;
	//初始化zip文件的属性结构体
	otainfo.csVersionHigh="";
	otainfo.csVersionLow="";
	otainfo.csZipFileName="";
	otainfo.nDirCount=0;
	otainfo.nZipType=-1;

	//查找对应的类型
	if (strTmp.find("FW.FULL",0)!=string::npos)
		otainfo.nZipType=FW_FULL;
	else if (strTmp.find("OTA.ALL",0)!= string::npos)
		otainfo.nZipType=OTA_ALL;
	else if (strTmp.find("OTA",0)!= string::npos)
		otainfo.nZipType=OTA;
	else if (strTmp.find("USR.LOGO",0)!= string::npos)
		otainfo.nZipType=USR_LOGO;
	else if (strTmp.find("USR.DATA",0)!= string::npos)
		otainfo.nZipType=USR_DATA;
	if((nPos=strTmp.rfind('/')) != string::npos)
		strTmpFileName = strTmp.substr(nPos+1);	
	else
		strTmpFileName = strTmp;
	otainfo.csZipFileName=strTmpFileName;
	//CFunCommon::DlSprintf("strTmpFileName=%s\n",strTmpFileName.c_str());
	switch (otainfo.nZipType)  //根据类别解析对应的版本
	{
	case FW_FULL:
		nPos = strTmpFileName.find("FW.FULL");
		otainfo.csMachine = strTmpFileName.substr(0,nPos-1);
		strTmp = strTmpFileName.substr(nPos);
		nPos = strTmp.find("_");
		strTmp = strTmp.substr(nPos+1);
		nPos = strTmp.find("_");
		otainfo.csVersionLow=strTmp.substr(0,nPos);
		strTmp=otainfo.csVersionLow;
		strTmp=strTmp.substr(1,strTmp.length());
		otainfo.verinfo_L.nMainVer=atoi(strTmp.substr(0,1).c_str());
		
		
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nMinorVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nPackVer=atoi(strTmp.substr(0,2).c_str());

		break;

	case OTA_ALL:

		nPos = strTmpFileName.find("OTA.ALL");
		otainfo.csMachine = strTmpFileName.substr(0,nPos-1);
		
		strTmp = strTmpFileName.substr(nPos);
		nPos = strTmp.find("_");
		strTmp = strTmp.substr(nPos+1);
		nPos = strTmp.find("_");
		otainfo.csVersionLow=strTmp.substr(0,nPos);
		strTmp=otainfo.csVersionLow;
		strTmp=strTmp.substr(1,strTmp.length());
		otainfo.verinfo_L.nMainVer=atoi(strTmp.substr(0,1).c_str());
		
		
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nMinorVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nPackVer=atoi(strTmp.substr(0,2).c_str());
	//	CFunCommon::DlSprintf("otainfo.csMachine =%s,otainfo.csVersionLow=%s,%d:%d:%d\n",otainfo.csMachine.c_str(),otainfo.csVersionLow.c_str(),otainfo.verinfo_L.nMainVer, otainfo.verinfo_L.nMinorVer,otainfo.verinfo_L.nPackVer);
		break;

	case USR_DATA:
		
		nPos = strTmpFileName.find("USR.DATA");
		otainfo.csMachine = strTmpFileName.substr(0,nPos-1);
		strTmp = strTmpFileName.substr(nPos);
		nPos = strTmp.find("_");
		strTmp = strTmp.substr(nPos+1);
		nPos = strTmp.find("_");
		otainfo.csVersionLow=strTmp.substr(0,nPos);
		strTmp=otainfo.csVersionLow;
		strTmp=strTmp.substr(1,strTmp.length());
		otainfo.verinfo_L.nMainVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nMinorVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nPackVer=atoi(strTmp.substr(0,2).c_str());

		break;

	case USR_LOGO:

		nPos = strTmpFileName.find("USR.LOGO");
		otainfo.csMachine = strTmpFileName.substr(0,nPos-1);
		strTmp = strTmpFileName.substr(nPos);
		nPos = strTmp.find("_");
		strTmp = strTmp.substr(nPos+1);
		nPos = strTmp.find("_");
		otainfo.csVersionLow=strTmp.substr(0,nPos);
		strTmp=otainfo.csVersionLow;
		strTmp=strTmp.substr(1,strTmp.length());
		otainfo.verinfo_L.nMainVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nMinorVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nPackVer=atoi(strTmp.substr(0,2).c_str());

		break;

	case OTA:

		nPos = strTmpFileName.find("OTA");
		otainfo.csMachine = strTmpFileName.substr(0,nPos-1);
		strTmp = strTmpFileName.substr(nPos);
		nPos = strTmp.find("_");
		strTmp = strTmp.substr(nPos+1);
		nPos = strTmp.find("_");
		strTmp=strTmp.substr(0,nPos);

		nPos = strTmp.find("...");
		
		otainfo.csVersionHigh=strTmp.substr(nPos+3);
		otainfo.csVersionLow=strTmp.substr(0,nPos);
		
		//CFunCommon::DlSprintf("otainfo.csMachine =%s,otainfo.csVersionLow=%s,otainfo.csVersionHigh=%s\n",otainfo.csMachine.c_str(),otainfo.csVersionLow.c_str(),otainfo.csVersionHigh.c_str());

		strTmp=otainfo.csVersionLow;
		strTmp=strTmp.substr(1,strTmp.length());
		otainfo.verinfo_L.nMainVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nMinorVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_L.nPackVer=atoi(strTmp.substr(0,2).c_str());

		//CFunCommon::DlSprintf("%d:%d:%d\n",otainfo.verinfo_L.nMainVer,otainfo.verinfo_L.nMinorVer,otainfo.verinfo_L.nPackVer);
		strTmp=otainfo.csVersionHigh;
		strTmp=strTmp.substr(1,strTmp.length());
		otainfo.verinfo_H.nMainVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_H.nMinorVer=atoi(strTmp.substr(0,1).c_str());
		strTmp=strTmp.substr(2,strTmp.length());
		otainfo.verinfo_H.nPackVer=atoi(strTmp.substr(0,2).c_str());
		//CFunCommon::DlSprintf("%d:%d:%d\n",otainfo.verinfo_H.nMainVer,otainfo.verinfo_H.nMinorVer,otainfo.verinfo_H.nPackVer);
		break;
	default:
		break;
	}

	strTmpPath = g_szAppPath;
	strTmpPath += "/unnzp/";

	snprintf(szCmd,sizeof(szCmd),"cp -r %s %s",pszZipFile,strTmpPath.c_str());
	CFunCommon::ExecSystem(szCmd);
	if(OTA_ALL == otainfo.nZipType )
	{
		string strOta = strTmpPath +strTmpFileName;
		snprintf(szCmd,sizeof(szCmd),"unzip -q -d %s %s",strTmpPath.c_str(),strOta.c_str());
		//CFunCommon::DlSprintf("szCmd = %s\n",szCmd);
		CFunCommon::ExecSystem(szCmd);
		remove(strOta.c_str());
	}
	m_vec_otainfo.push_back(otainfo); 
	return ;
}



/**
 * @brief 	解析传入的android应用文件
 * @param[in] pszFileName    	传入文件名
 * @return
 * @li  true        	成功
 * @li  false        	失败
*/
bool CNLPDownload::ParseAndroidFileList(const char* pszFileName,bool bList)
{

	int nCount = 0;

	if(NULL == pszFileName)
	{
		CFunCommon::DlSprintf("Failed: no file to download\n");
		return false;
	}

	if(!bList)
	{
		//OTA类型为1，下载需要再次解析
		m_nFileCount = 1;
		//CFunCommon::DlSprintf("sFileName = %s\n",sFileName);
		if(strstr(pszFileName,".OTA") != NULL || strstr(pszFileName,".ota") != NULL||strstr(pszFileName,".ZIP") != NULL || strstr(pszFileName,".zip") != NULL)
		{
			m_nFileType[0] = 1;
			UnpackZipFile(pszFileName);
		}
		else
		{
			m_nFileType[0] = 8;
		}
		strcpy(m_szDownFullPath[0],pszFileName);
		return true;
	}
	else
	{
		FILE *fp = NULL;
		char szBuff[256] = {0};
		bool bFind = false;
		int nPos = 0;
		string strConver,strLeft,strRight,strRelativePub,strTmp;
		strTmp = pszFileName;
		nPos = strTmp.rfind("/");
		if(nPos != -1)
		{
			strRelativePub = strTmp.substr(0,nPos+1);
		//	CFunCommon::DlSprintf("strRelativePub = %s,nPos = %d\n",strRelativePub.c_str(),nPos);
		}
	
		if (NULL == (fp = fopen(pszFileName, "rb")) ) 
		{
			CFunCommon::DlSprintf("Failed to open file %s.\n",pszFileName);
			return false;
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
				if(!strRelativePub.empty())
					strLeft = strRelativePub + strLeft;
				//ota
				if(strLeft.find(".ota")!= string::npos || strLeft.find(".OTA")!= string::npos ||strLeft.find(".zip")!= string::npos||strLeft.find(".ZIP")!= string::npos)
				{
					//CFunCommon::DlSprintf("strLeft.c_str() = %s\n",strLeft.c_str());
					m_nFileType[nCount] = 1;
					UnpackZipFile((char *)strLeft.c_str());
					strcpy(m_szDownFullPath[nCount],strLeft.c_str());
					m_nFileCount++ ;
					nCount++;
				}
				else 
				{
					m_nFileType[nCount] = 8;
					strcpy(m_szDownFullPath[nCount],strLeft.c_str());
					m_nFileCount++ ;
					nCount++;
				}
					
			}
	
		}
		fclose(fp);
		return true;
	}
	
}


/**
 * @brief 	生成android平台应用下载列表
 * @param[out] strVersion    设备返回的版本信息
 * @return void
*/
void CNLPDownload::MakeAndroidDownFile(string strVersion)
{
	string strFile,strPosVer,strTemp,strMacType,strFirmID,strFindPath;
	OTA_ZIP_INFO otainfo;		//ota包的信息
	char szBuff[MAX_PATH + 100] = {0};
	strFindPath = g_szAppPath;
	strFindPath += "/unnzp";
	int nRet = 0,nPosMainVer = -1,nPosMinorVer = - 1,nPosPackVer = -1;
	
	for (int i=0; i<m_nFileCount; i++)
	{
		m_strDownZip="";
		//CFunCommon::DlSprintf("m_nFileType[i] = %d,m_nFileCount-%d\n",m_nFileType[i],m_nFileCount);
		if (1 == m_nFileType[i])
		{
			//解压的路径中选取文件
			strTemp = strVersion;
			strMacType =strTemp.substr(0,strTemp.find("_"));
			//CFunCommon::DlSprintf("strMacType = %s\n",strMacType.c_str());
			strTemp = strTemp.substr(strTemp.find("_") +1 );

			strFirmID=strTemp.substr(0,strTemp.find("_"));
		//	CFunCommon::DlSprintf("strFirmID = %s\n",strFirmID.c_str());
			strTemp = strTemp.substr(strTemp.find("_")+1);

	
			strTemp=strTemp.substr(0,strTemp.find("_"));
			strPosVer=strTemp;
			//CFunCommon::DlSprintf("strPosVer = %s\n",strPosVer.c_str());

			strTemp = strTemp.substr(1);
			nPosMainVer=atoi(strTemp.substr(0,1).c_str());
			//CFunCommon::DlSprintf("nPosMainVer = %d\n",nPosMainVer);
			strTemp = strTemp.substr(2);
			nPosMinorVer=atoi(strTemp.substr(0,1).c_str());
			//CFunCommon::DlSprintf("nPosMinorVer = %d\n",nPosMinorVer);
			strTemp = strTemp.substr(2);
			nPosPackVer=atoi(strTemp.c_str());
			//	CFunCommon::DlSprintf("nPosPackVer = %d\n",nPosPackVer);
			vector<OTA_ZIP_INFO>::iterator iter;
			for (iter=m_vec_otainfo.begin(); iter!=m_vec_otainfo.end(); iter++)
			{
				string itemName =m_szDownFullPath[i];
				otainfo = *iter;	
				if (otainfo.csZipFileName == itemName)
				{
					break;
				}

			}
			//	CFunCommon::DlSprintf("otainfo.csMachine = %s\n",otainfo.csMachine.c_str());
			if (otainfo.csMachine.find(strMacType.c_str())<0)
			{
				nRet = -1; //机型匹配错误				
				m_strDownZip = "";
				continue;
			}

			if (otainfo.csMachine.find(strFirmID.c_str())<0)
			{
				nRet = -2; //硬件识别码匹配错误
				m_strDownZip = "";
				continue;
			}
			//	CFunCommon::DlSprintf("otainfo.nZipType = %d\n",otainfo.nZipType);
			//xx.xx.xx-xx.xx.xx_.OTA
			if (OTA == otainfo.nZipType )//OTA包需要判断版本是否介于当前的范围
			{
				//CFunCommon::DlSprintf("otainfo.verinfo_L.nMainVer=%d,nPosMainVer=%d\n",otainfo.verinfo_L.nMainVer,nPosMainVer);
				if (otainfo.verinfo_L.nMainVer>nPosMainVer || otainfo.verinfo_H.nMainVer<nPosMainVer)
				{
					nRet= -3; //pos的主版本号不在OTA包的范围内
					m_strDownZip ="";
					continue;
				}
				//CFunCommon::DlSprintf("otainfo.verinfo_L.nMinorVer=%d,nPosMinorVer=%d\n",otainfo.verinfo_L.nMinorVer,nPosMinorVer);
				if (otainfo.verinfo_L.nMinorVer>nPosMinorVer || otainfo.verinfo_H.nMinorVer<nPosMinorVer)
				{
					nRet = -4; //pos的次版本号不在OTA包的范围内
					m_strDownZip = "";
					continue;
				}

				if (otainfo.verinfo_L.nMinorVer == nPosMinorVer)  //主、次版本都相同
				{
					if (otainfo.verinfo_L.nPackVer>nPosPackVer)
					{
						nRet = -5;	//Pos的版本不在OTA包范围内
						m_strDownZip = "";
						continue;
					}
				}
				//主次版本相同，两个ota包即没意义，用一个就可以
				if (otainfo.verinfo_H.nMinorVer==nPosMinorVer)  //主、次版本都相同
				{
					if (otainfo.verinfo_H.nPackVer<=nPosPackVer)
					{
						nRet = -5;	//Pos的版本不在OTA包范围内
						m_strDownZip = "";
						continue;
					}
					//	CFunCommon::DlSprintf("otainfo.csVersionHigh = %s\n",otainfo.csVersionHigh.c_str());
					strFile=strMacType+"_"+strFirmID+"_"+strPosVer+"..."+otainfo.csVersionHigh+".zip";
				//	CFunCommon::DlSprintf("strFile = %s\n",strFile.c_str());
					strTemp = strFile;
					m_strDownZipTmp = "";
					if(!FindZIPFile(strFindPath,strTemp))
					{
						nRet = -6; //查找文件失败
						m_strDownZip = "";
						continue;
					}
					else
					{
						m_strDownZip += m_strDownZipTmp;
						m_strDownZip += "*8|";
					}
				}
				else
				{
					if (0 == otainfo.verinfo_L.nPackVer)
					{
						strFile = "";
						sprintf(szBuff,"%s_%s_OTA_V%d.%d.%02d...V%d.%d.00",
							strMacType.c_str(),strFirmID.c_str(),nPosMainVer,nPosMinorVer,nPosPackVer,nPosMainVer,nPosMinorVer+1);
						//strTemp.Format("%s_%s_OTA_V%d.%d.%02d...V%d.%d.00",
							//strMacType,strFirmID,nPosMainVer,nPosMinorVer,nPosPackVer,nPosMainVer,nPosMinorVer+1);
						strTemp = szBuff;
						strFile += strTemp;
						strFile += ".zip";

						strTemp = strFile;
						if(!FindZIPFile(strFindPath,strTemp))
						{
							nRet = -6; //查找文件失败
							m_strDownZip = "";
							continue;
						}
						else
						{
							m_strDownZip += m_strDownZipTmp;
							m_strDownZip += "*8|";
						}
					}						
					if ((nPosMinorVer+1)!=otainfo.verinfo_H.nMinorVer)  //如果pos的次版本增加1无法和配置文件中的次版本相等，还需要一个升级包
					{
						strFile="";
						sprintf(szBuff,"%s_%s_OTA_V%d.%d.00...V%d.%d.00",
							strMacType.c_str(),strFirmID.c_str(),nPosMainVer,nPosMinorVer+1,nPosMainVer,otainfo.verinfo_H.nMinorVer);
						strTemp = szBuff;

						strFile += strTemp;
						strFile += ".zip";

						strTemp = strFile;
						if(!FindZIPFile(strFindPath,strTemp))
						{
							nRet = -6; //查找文件失败
							m_strDownZip = "";
							continue;
						}
						else
						{
							m_strDownZip += m_strDownZipTmp;
							m_strDownZip += "*8|";
						}
					}

					if (0 != otainfo.verinfo_H.nPackVer)
					{
						strFile = "";
						
						sprintf(szBuff,"%s_%s_OTA_V%d.%d.00...V%d.%d.%02d",
							strMacType.c_str(),strFirmID.c_str(),nPosMainVer,otainfo.verinfo_H.nMinorVer,nPosMainVer,otainfo.verinfo_H.nMinorVer,otainfo.verinfo_H.nPackVer);
						strTemp = szBuff;


						strFile += strTemp;
						strFile += ".zip";

						strTemp = strFile;
						if(!FindZIPFile(strFindPath,strTemp))
						{
							//AfxMessageBox(strFile);
							nRet = -6; //查找文件失败
							m_strDownZip = "";
							continue;
						}
						else
						{
							m_strDownZip += m_strDownZipTmp;
							m_strDownZip += "*8|";
						}							
					}
				}
			}
			//xx.xx.xx_.OTA
			else  //，只要判断包版本和pos版本的关系
			{
			//	CFunCommon::DlSprintf("otainfo.verinfo_L.nMainVer=%d,nPosMainVer=%d\n",otainfo.verinfo_L.nMainVer,nPosMainVer);
				if (otainfo.verinfo_L.nMainVer<nPosMainVer)
				{
					nRet = -3; //OTA包的主版本比pos的主版本低
					m_strDownZip = "";
					continue;
				}
				//	CFunCommon::DlSprintf("otainfo.verinfo_L.nMinorVer=%d,nPosMinorVer=%d\n",otainfo.verinfo_L.nMinorVer,nPosMinorVer);
				if (otainfo.verinfo_L.nMinorVer<nPosMinorVer)
				{
					nRet = -4; //OTA包的次版本比pos的次版本低
					m_strDownZip = "";
					continue;
				}

				if (otainfo.verinfo_L.nMinorVer == nPosMinorVer)  //主、次版本都相同
				{
				//	CFunCommon::DlSprintf("otainfo.verinfo_L.nPackVer=%d,nPosPackVer=%d\n",otainfo.verinfo_L.nPackVer,nPosPackVer);
					if (otainfo.verinfo_L.nPackVer <= nPosPackVer)
					{
						nRet= -5;	//OTA包的补丁版本比pos的补丁版本低，或者是相等
						m_strDownZip="";
						continue;
					}

					strFile=strMacType+"_"+strFirmID+"_OTA_"+strPosVer+"..."+otainfo.csVersionLow+".zip";

						
					strTemp = strFile;
				//	CFunCommon::DlSprintf("strFindPath=%s,strTemp=%s\n",strFindPath.c_str(),strTemp.c_str());	
					if(!FindZIPFile(strFindPath,strTemp))
					{
						nRet = -6; //查找文件失败
						m_strDownZip = "";
						continue;
					}
					else
					{
						m_strDownZip += m_strDownZipTmp;
						m_strDownZip += "*8|";
					}
				}
				else
				{
					if (0 == otainfo.verinfo_L.nPackVer && (nPosMinorVer+1)==otainfo.verinfo_L.nMinorVer)
					{

						strFile = "";
				
						sprintf(szBuff,"%s_%s_OTA_V%d.%d.%02d...V%d.%d.00",
							strMacType.c_str(),strFirmID.c_str(),nPosMainVer,nPosMinorVer,nPosPackVer,nPosMainVer,nPosMinorVer+1);
						strTemp = szBuff;


						strFile += strTemp;
						strFile += ".zip";
						strTemp = strFile;
						if(!FindZIPFile(strFindPath,strTemp))
						{
							nRet = -6; 
							m_strDownZip = "";
							continue;
						}
						else
						{
							m_strDownZip += m_strDownZipTmp;
							m_strDownZip += "*8|";
						}
					}											
					if (((nPosMinorVer+1) != otainfo.verinfo_L.nMinorVer) && 0 == otainfo.verinfo_L.nPackVer)  //如果pos的次版本增加1无法和配置文件中的次版本相等，还需要一个升级包
					{
						strFile="";

						sprintf(szBuff,"%s_%s_OTA_V%d.%d.00...V%d.%d.00",
							strMacType.c_str(),strFirmID.c_str(),nPosMainVer,nPosMinorVer+1,nPosMainVer,otainfo.verinfo_L.nMinorVer);
						strTemp = szBuff;


						strFile += strTemp;
						strFile += ".zip";
						strTemp = strFile;
						if(!FindZIPFile(strFindPath,strTemp))
						{
							nRet = -6; 
							m_strDownZip = "";
							continue;
						}
						else
						{
							m_strDownZip += m_strDownZipTmp;
							m_strDownZip += "*8|";
						}
					}

					if (otainfo.verinfo_L.nPackVer!=0)
					{
						strFile="";

						sprintf(szBuff,"%s_%s_OTA_V%d.%d.00...V%d.%d.%02d",
							strMacType.c_str(),strFirmID.c_str(),nPosMainVer,otainfo.verinfo_L.nMinorVer,nPosMainVer,otainfo.verinfo_L.nMinorVer,otainfo.verinfo_L.nPackVer);
						strTemp = szBuff;

						strFile += strTemp;
						strFile += ".zip";

						strTemp = strFile;
						if(!FindZIPFile(strFindPath,strTemp))
						{
							//AfxMessageBox(strFile);
							nRet = -6; //查找文件失败
							m_strDownZip = "";
							continue;
						}
						else
						{
							m_strDownZip += m_strDownZipTmp;
							m_strDownZip += "*8|";
						}							
					}
				}				
			}

		

			if (m_strDownZip != "")
				m_strDownList += m_strDownZip;
		}
		
		else 
		{					

			m_strDownList += m_szDownFullPath[i];;
			m_strDownList += "*8|";
			

		}
				
	}

}


/**
 * @brief 	解析并打印android list
 * @param[out] strFileList    返回的应用列表信息
 * @return void
*/
void CNLPDownload::ParseAndroidList(string strFileList)
{
	string strApplist;
	string strTmp,strTmpName,strTmpVer,strTmpPacket;

	int i = 0,nPos = 0,nSubPos = 0;
	int nTag = 0; //是否最后一个数据

	strApplist = strFileList;
	
	while (1)
	{
		nPos = strApplist.find('|');
		if (-1 == nPos )
		{
			if (1 == nTag)
			{
				return;
			}
			nTag = 1;
		}	
		strTmp = strApplist.substr(0,nPos);
		if(strTmp.find('*') != -1)
		{		
			nSubPos = strTmp.find('*');
			// app name
			strTmpName = strTmp.substr(0,nSubPos);
			strTmp = strTmp.substr(nSubPos+1);
			//ver
			nSubPos=strTmp.find('*');
			strTmpVer=strTmp.substr(0,nSubPos);
			strTmp = strTmp.substr(nSubPos+1);
			//pack name
			nSubPos=strTmp.find('*');
			strTmp = strTmp.substr(nSubPos+1);
			//高端平台最后一个字段数据改为包名
			if (strTmp.length() >1)
			{
				strTmpPacket = strTmp;
			}
			CFunCommon::DlSprintf("%d:%s %s %s\n",i+1,strTmpName.c_str(),strTmpVer.c_str(),strTmpPacket.c_str());
			
		}
		else if (strTmp.find('=') != -1)
		{
			nSubPos = strTmp.find('=');
			// app name
			strTmpName = strTmp.substr(0,nSubPos);
			//value
			strTmp = strTmp.substr(nSubPos+1);
			CFunCommon::DlSprintf("%d:%s %s \n",i+1,strTmpName.c_str(),strTmp.c_str());
		}
		strApplist = strApplist.substr(nPos+1);
		i++;
	}
	return;
}


/**
 * @brief 	获取android app列表
 * @param[in] pszDev    usb-serail号
 * @return 
 * @li 0 成功
 * @li <0 失败
*/
int CNLPDownload::GetAppList(const char* pszDev)
{
	char szDevicePath[20] = {0};

	int nRet = 0;
	string strAppList;
	snprintf(szDevicePath,sizeof(szDevicePath),"/dev/%s",pszDev);
	CARQ arq(0,3);
	if(!arq.OpenPort(szDevicePath))
		return -1;
	if(-1 == (nRet=arq.Android_Shakehand()) )
	{
		CFunCommon::DlSprintf("Failed to sharkhand.");
		return -1;
	}
	arq.InitBuffer(nRet);
	if(!arq.Android_GetAppInfo(strAppList) )
	{
			CFunCommon::DlSprintf("Failed to Get App Info .\n");
			return -1;
	}
	//	CFunCommon::DlSprintf("%s\n",strAppList.c_str());
	ParseAndroidList(strAppList);
	CFunCommon::DlSprintf("%s\n",DOWN_SUCC);
	return 0;	
}

/**
 * @brief 	下载
 * @param[in] pszFileName    下载文件列表
 * @param[in] pszDev    		设备号
 * @param[in] bClear    		是否清空
 * @param[in] nType    		下载类型  固件/应用 下载
 * @return
 * @li  0        	成功
 * @li  -1        	失败
*/
int CNLPDownload::DownloadProcess(const char* pszFileList,const char* pszDev,bool bClear,int nType)
{
	char szDevicePath[20] = {0};
	char szVersion[1024] = {0};
	int nMposType = 0;
	int nRet = 0;
	snprintf(szDevicePath,sizeof(szDevicePath),"/dev/%s",pszDev);
	CARQ arq(bClear,3);
	if(!arq.OpenPort(szDevicePath))
		return -1;
	if (sType_ANDROID_APP == nType)
	{
		if(-1 == (nRet=arq.Android_Shakehand()) )
		{
			CFunCommon::DlSprintf("Failed to sharkhand.");
			return -1;
		}
		arq.InitBuffer(nRet);
	
		if (!arq.Android_GetVersion(szVersion))
		{
			CFunCommon::DlSprintf("Failed to GetVersion.");
			return -1;
		}
	//	CFunCommon::DlSprintf("POS Version: %s\n",szVersion);
		MakeAndroidDownFile(szVersion);
	//	struct  timeval  start;
  	//	struct  timeval  end;
  	//	gettimeofday(&start,NULL);
		if(arq.Android_DownLoad_APP(m_strDownList.c_str()) != 0)
		{
			CFunCommon::DlSprintf("Failed to download .\n");
			return -1;
		}
	//	gettimeofday(&end,NULL);	
	//	CFunCommon::DlSprintf("[ %us ]\n",end.tv_sec-start.tv_sec);
		CFunCommon::DlSprintf("%s\n",DOWN_SUCC);
		return 0;
	}
	
	arq.InitBuffer(SENDBUFFER_4K);
	
	if(sType_MPOS_APP == nType)
	{
		nMposType = arq.GetMposPlat(1);
		if (nMposType < 0)
		{
			//CFunCommon::DlSprintf("get boot error.\n");
			return -1;
		}
		if (mpos_plat_scrop == nMposType)
		{
			if(arq.Scrop_DownLoad_APP(pszFileList) != 0)
			{
				CFunCommon::DlSprintf("Failed to download .\n");
				return -1;
			}
		}
		if (mpos_plat_cruxplus == nMposType)
		{
			if(arq.CruxPlus_DownLoad_APP(pszFileList) != 0)
			{
				CFunCommon::DlSprintf("Failed to download.\n");
				return -1;
			}
		}
		CFunCommon::DlSprintf("%s\n",DOWN_SUCC);
		return 0;
		
	}
		
	if(sType_MPOS_FIRM == nType )	 
	{
		nMposType = arq.GetMposPlat(2);
		if (nMposType < 0)
		{
			//CFunCommon::DlSprintf("get boot error.\n");
			return -1;
		}
		//CFunCommon::DlSprintf("pszFileList=%s\n",pszFileList);
		if (mpos_plat_scrop == nMposType)
		{
			if(arq.Scrop_DownLoad_Firm(pszFileList) != 0)
			{
				CFunCommon::DlSprintf("Failed to download.\n");
				return -1;
			}
		}
		
		if (mpos_plat_cruxplus == nMposType)
		{
			if(arq.CruxPlus_DownLoad_Firm(pszFileList) != 0)
			{
				CFunCommon::DlSprintf("Failed to download.\n");
				return -1;
			}
		}
		CFunCommon::DlSprintf("%s\n",DOWN_SUCC);
		return 0;
	}
	return -1;
	
}




