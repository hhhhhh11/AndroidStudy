/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	nlcdownload.h
 * @brief	Download Communication class for *.NLC
 * @version	1.0
 * @author: ym
 * @date	2021/04/11
 ******************************************************************************/
#ifndef _NLCDOWNLOAD_H_
#define _NLCDOWNLOAD_H_
#include "pubdef.h"
#include "usbserial.h"	
#include "codetools.h"
#include "common.h"
#include "pubdef.h"
#include <ctype.h>
#include <sys/stat.h>
#include <string>
#include <ctype.h>
#include <sys/stat.h>
using namespace std;


#define MAX_PART_NUM			20		// BIOS分区数
#define MAX_LINUX_CMDLINE		128
#define MAX_PARAM_NAME			24
#define MAX_VERSION_LEN			16	
#define MAX_NLC_COUNT 			50  	//最大的NLC包数目
#define CMD_REFRESHDEVICE		0xE0	//刷新设备信息
#define CMD_DOWNLOAD			0xE1	//下载分区
#define CMD_ERASE				0xE2	//擦除分区
#define CMD_LINUXRUN			0xE3	//启动linux
#define CMD_REBOOT				0xE4	//重启系统
#define CMD_UPDATAPARAM			0xE5	//更新启动参数
#define CMD_RELOAD				0xE6	//重载分区

	


#define PART_RDONLY     0x80
#define PARTSIZE  sizeof(part_t)
#define PARTSIZE_OLD  sizeof(part_t_old)
#define MAX_FILE_PACKET 4096*4
#define NLC_TIME_OUT 	10000


typedef struct 
{
	char name[MAX_PARAM_NAME];				//分区名称
	char version[MAX_VERSION_LEN];			//分区版本
	unsigned int  flag;					//分区标志
	unsigned int start_address;			//分区起始地址
	unsigned int end_address;				//分区结束地址
	unsigned int writelen; 				//是否可写标志
}part_t;
	
typedef struct 
{
	char name[MAX_PARAM_NAME];				//分区名称
	int  flag;								//分区标志
	unsigned int start_address;			//分区起始地址
	unsigned int end_address;				//分区结束地址
	unsigned int writelen; 				//是否可写标志
}part_t_old;
	
typedef struct 
{
	part_t part[MAX_PART_NUM];				// 分区信息
	part_t_old partold[MAX_PART_NUM];		// 旧版分区信息
	int total;								// 分区个数
}part_table;



		
		



/***************6ul define begin******************/

#define RSAKEY_LEFT 56
#define RSAKEY_RIGHT MAX_RSA_MODULUS_LEN - RSAKEY_LEFT
#define SIG_LEN         MAX_RSA_MODULUS_LEN
#define CMD_HEAD_LEN        4
#define CMD_DWN_HEAD_LEN    (CRUXCERT_LEN + SIG_LEN)

typedef unsigned char uint8_t;
typedef struct {
    uint8_t signature[MAX_RSA_MODULUS_LEN];
    uint8_t rsakey_right[RSAKEY_RIGHT];
} rsa_pk_cert_t;
#define HEADLENTH 712
#define CRUXCERT_LEN    sizeof(rsa_pk_cert_t)

/***************6ul define end******************/


/**
 * 中端平台固件下载类，实现NLC包与list文件解析，固件文件解压，解压后image或bin文件下载，满足所有中端平台设备与6ul底座固件下载
*/
class CNlcDownload{


public:
	CNlcDownload();
	~CNlcDownload();

	int ParseFileList(const char* pszFileName,bool bList);
	bool UnCompressNLCAuto(const char* pszNLC,int nIndex);		
	bool UnCompressListAuto(const char* pszNLC,int nIndex) ;	
	int DownloadProcess(const char* pszFileList,const char* pszDev);
	void InitMultiNlcArr();
	bool ReadGroupAttAuto(const char* pszNLC,const char* pszFileName,int nIndex);	
	bool ReadGroupAtt(const char* pszFileName,int nIndex);

	int MakeBootDownFile(const char* pszBootName,int nIndex);			
	int MakeBiosDownFile(GROUP_FILE_ATT gfAtt,int nIndex);	
	int MakeBiosDownFile_old(char* pszFPath,int nIndex);			
	bool ReLoadDefaultPart(void);			
	bool DownloadFile(const char* pszBiosFile);		
	bool RefreshDevice(void);					
	bool ReBoot(void);						
	bool ReStart(void);						
	int WaitForConnect(const char* pszDev);		



	//6ul
	bool MakeDownFileString_6ul(string strPath,GROUP_FILE_ATT gfAtt,int nIndex);
	bool DownLoadFileList_6ul(const char* pszFileList);
	int RecvOnePacket_6ul(unsigned char ucCmd,unsigned char* pszData,int nLen,int nTimeout);
private:

	
	int m_nSlaveCount;						///<使用list方式加载时，配置的slave包数量
	char m_cDownType;						///<sType_6UL:蓝牙底座;sType_NLC：中端设备
	bool m_bMaster;							///<是否配置为master
	bool m_bIsFull;							///<true:全量包;false:单分区包
	bool m_beNewBios;						///<是否新bios，现在使用的都属于新的
	char m_szDownFileList[MAX_PATH*30];		///<解包后，从newconfig.ini文件取出的下载文件列表，通过|分割
	char m_szBootVer[128];					///<POS返回的boot版本信息
	CUsbSerial m_objUsbSerial;				///<usb-serail通讯对象
	
	
	

	char* m_partinfo;
	part_t * m_packetpart;
	part_t_old * m_packetpartold;
	part_table *m_ptrpart;			
	part_table m_ptrpartThread[8];				
	
	
};

	
	


#endif




