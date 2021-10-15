#ifndef _PUBDEF_H_
#define _PUBDEF_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;

#define FALSE (0)
#define TRUE (1)

typedef unsigned int   u32_t;
typedef signed long s32_t;

typedef unsigned short u16_t;
typedef signed short s16_t;

typedef unsigned char u8_t;
typedef char s8_t;

typedef s8_t S8;  //8λ�з�������   1�ֽ�
typedef u8_t U8;

typedef s16_t S16; //16λ�з�������   2�ֽ�
typedef u16_t U16;

typedef s32_t S32; //32λ�з�������   4�ֽ�
typedef u32_t U32;

typedef long BOOL;

#define kbytes	1024




#define	sType_App			(1)	//表示应用下载 中端平台NLD 非list配置
#define	sType_NLC			(2)	//表示POSB平台固件下载
#define	sType_NLD			(3)//中端平台应用下载,list配置是

#define sType_GET_APP		(4)	//获取pos应用信息
#define sType_MPOS_FIRM		(5)	//表示Mpos固件下载
#define sType_MPOS_APP		(6)	//表示Mpos应用下载
#define sType_ANDROID_APP	(7)	//android应用下载
#define sType_ANDROID_FIRM	(8)	//Android固件下载		固件后缀  ARD

#define sType_ANDROID_MultiPara (9) //通过list加载的android应用下载
#define sType_App_MultiPara (10)	//多应用多参数类型，list配置时

#define sType_APN_MMC		(11)	//获取apn编辑的mmc，mnc信息
#define sType_6UL		(12)	//6ul






#define MSGSUM 103						//消息条数	


enum iMsgCode 
{	
	DOWN_MSG_SUCCESS = 0,				// 0-->下载成功
	DOWN_ERR_PORT_OPEN,					// 串口未打开
	DOWN_ERR_TIMEOUT,					// 等待超时
	DOWN_ERR_OVERTRY,					// 发送超过3次数据
	DOWN_ERR_MORE_MAINPROG,				// 一次只能发送一个主控程序
	DOWN_ERR_VER,						// 下载程序版本过低
	DOWN_ERR_SPACE,						// 判断当前列表中的所有多应用程序的空间是否使用超标
	DOWN_ERR_NOFILE,					// 没有文件需要下载
	DOWN_ERR_REFRESH,					// 删除前需重新刷新
	DOWN_ERR_FILETYPE,					// 错误文件类型：无效的多应用程序
	DOWN_ERR_FILE_UNKNOWN,				// 10-->文件类型无法识别
	DOWN_ERR_APP_VER,					// 应用文件版本过低
	DOWN_ERR_FILE_PARA,					// 文件传参失败
	DOWN_ERR_FILE_OPEN,					// 打开文件失败
	DOWN_ERR_FILE_LENGTH,				// 文件长度异常
	DOWN_ERR_FILE_READ,					// 文件读失败
	DOWN_ERR_FILE_WRITE,				// 文件写失败
	DOWN_ERR_FILE_CHECK,				// 文件校验失败
	DOWN_ERR_FILE_UNPACK,			    // 文件解包失败
	DOWN_ERR_FILE_PACK,					// 文件打包失败
	DOWN_ERR_FRM_LENGTH,				// 20-->帧长度超标
	DOWN_ERR_PROT_NOACK,				// POS无应答
	DOWN_ERR_PROT_WRONGACK,				// 错误应答
	DOWN_REFRESH_OK,					// 刷新成功	
	DOWN_DEL_OK,						// 删除成功
	DOWN_ERR_OPEN_PORT,					// 串口打开失败
	DOWN_MSG_CLEAR,						// 清空消息
	DOWN_MSG_PORT_DETECT,				// 本端口已经探测成功
	DOWN_ERR_PARA_SAVE,					// 参数文件保存失败
	DOWN_ERR_APP_DIR,					// 请检查下载应用目录！
	DOWN_MSG_DOWNING = 30,				// 正在下载
	DOWN_MSG_WAITING_POS,				// 等待POS接入
	DOWN_ERR_COPY_FILE,					// 拷贝临时文件错误！
	DOWN_ERR_FILE,						// 文件内容非法
	DOWN_MSG_EXIT_DOWNTOOL,				// 是否退出下载程序？
	DOWN_ERR_PACK_MONITOR_THREAD,		// 启动报文监控线程失败
	DOWN_ERR_MACHINE_BOOT = 39,			// 请确认机器型号与下载的BOOT是否匹配！
	DOWN_MSG_BIOS_VER_LAST = 40,		// POS版本已经是最新的！
	DOWN_ERR_BIOS_REFRESH = 41,			// 固件刷新失败！
	DOWN_MSG_BIOS_SUCCESS = 42,			// 42=固件升级成功！重启机器！
	DOWN_ERR_BIOS_UPDATE = 43,			// 升级固件失败
	DOWN_ERR_NLC_UPPACKE = 46,			// NLC解包失败
	DOWN_ERR_BIOS_ANALYZE_TLV = 47,		// 解析TLV失败
	DOWN_MSG_ERASE_OK = 48,				// 擦除成功
	DOWN_ERR_ERASE = 49,				// 擦除失败
	DOWN_ERR_PACKET = 50,				// 错误的报文，请检查重试
	DOWN_MSG_MODIFY_OK = 51,			// 修改成功
	DOWN_ERR_MODIFY = 52,				// 修改失败
	DOWN_SUCCESS_BIOS = 53,				// 下载成功！总共下载文件：
	DOWN_ERR_BIOS_BAD_BLOCKS = 54,		// 坏块过多
	DOWN_ERR_BIOS_FILE = 55,			// 无效的bios文件
	DOWN_MSG_RELOAD_PART = 56,			// 重载分区成功
	DOWN_ERR_RELOAD_PRAT = 57,			// 重载缺省分区失败
	DOWN_ERR_DEL_DIR = 58,				// 删除目录失败
	DOWN_ERR_MKDIR = 59,				// 创建目录失败
	DOWN_ERR_NLC = 60,					// NLC升级包版本有误
	DOWN_ERR_BIOS_DATATYPE,				// 数据格式错误
	DOWN_MSG_REFRESHING,				// 正在刷新
	DOWN_ERR_POSABIOS_NOT_BOOTROM,		// 不允许下载BOOTROM
	DOWN_ERR_POSABIOS_SECTORNUM,		// 起始块号超大
	DOWN_ERR_PACKAGE_TOOL,				// 没有打包工具
		
	DOWN_ERR_BOOTFILE_LOST,				//boot文件缺失
	DOWN_ERR_BOOT_REFRESH,				//boot文件下载版本刷新失败
	DOWN_ERR_PARA_TLV,					//0x01:数据tlv解析错误
	DOWM_ERR_GET_LENGTH,				//0x02:长度信息获取失败
	DOWM_ERR_GET_PATITION,				//0x03:在pos端无法获取到对应分区信息
	DOWM_ERR_GET_DATA,					//0x04:获取数据文件出错
	DOWM_ERR_CRC,						//0x05:crc校验出错
	DOWM_ERR_SIGNATURE,					//0x06:签名校验出错
	DOWM_ERR_WRITEFLASH,				//0x07:写flash错误
	DOWM_ERR_PATITION_READONLY,			//0x08:要修改的分区为只读分区
	DOWM_ERR_BADBLOCK,					//0x09:flash坏块数过多
	DOWM_ERR_NO_PATITIONINFO,			//0x0A:无对应分区信息
	DOWM_ERR_DATA,						//0x11:数据格式错误
	DOWN_ERR_COMM,						//0xff:通讯出错.

	DOWN_ERR_SENDERROR=80,				//数据发送失败
	DOWN_ERR_RECVERROR=81,				//数据接收失败
	DOWN_ERR_PROCESSERROR=82,			//数据处理失败
	DOWN_ERR_CHECKING=83,				//数据校验中

	//低端错误
	DOWN_ERR_UNSUPPORTED_COMMAND=87,	//不支持的命令
	DOWN_ERR_SIGNATURE_DECODE = 88,		//签名解密失败
	DOWN_ERR_FLASE_ERASING=89,			//flash擦除失败
	DOWN_ERR_FLASH_WRITE=90,			//flash写失败
	DOWN_ERR_FLASH_READ=91,				//flash读失败
	DOWN_ERR_FIRMWARE_TYPE=92,			//固件类型检查错误
	DOWN_ERR_HASH_VALUE=93,				//hash值错误
	DOWN_ERR_FLASH=94,					//flash错误
	DOWN_ERR_FILE_CREATE=95,			//文件创建错误
	DOWN_ERR_FILE_NOTFOUND=96,			//未找到匹配品号
};

typedef struct 
{
	char			cLogo[4];					/* 4字节固定标识 NLAA		*/
	char			cVerBuf[4];					/* 4字节版本号(目前固定0001)*/
	unsigned char	cLength[2];					/* 2字节头文件长度 hex格式		*/
	char			cMasterName[128];			/* 主控模块名              */ 
	unsigned char	cMasterPos;					/* 主控模块位置，在列表框的顺序 */
	char			cMaintainBuf[99];			/* 99字节保留信息             */
	unsigned char	cRecordSum;					/* 1字节文件记录个数      */
}TMSHEADERINFO;		/* TMS??????? */


/************************************************************************/
typedef struct  
{
	char			cPlateform;					/*1字节---平台类型    '0'表示uCos，'1'表示linux*/
	char			cMachineType[12];			/*12字节---机器类型   "8510"、"GP730"  如果值为全0，表示无机具类型*/
	char			cFileType;					/*文件类型    'A':应用 'M':主控 'L':库文件 'P':参数 'O':其它文件 （08-31新增）*/
	char			cMoudle[128];				/* 128字节----模块名		*/
	char			cVer[30];					/* 30字节----版本号			*/
	char			cFileName[128];				/* 128字节---文件名			*/
	char			cLen[4];					/* 4字节-----文件长度		*/
	char			cRecordBegin[4];			/* 4字节-----文件记录的起始位置 */
	char			cMaintainBuf[100];			/* 100字节保留信息              */
}TMSFILE;			/* 文件记录格式 */



typedef struct
{
	int GroupCount;  //用于存放分组的文件数目
	char szGroupVer[128];//分组的版本属性
	int iBoot;		//是否有boot文件   0：无，1：有boot文件
}GR_CCOUNT_VER;  //分组的文件数目和版本结构体

typedef struct  
{
	GR_CCOUNT_VER GrAttitude;		//分组结构体
	char szGroupFile[20][128];	//分组的文件名数组
}GROUP_FILE_ATT;


typedef struct
{
	char  szFullname[128];//分区bin文件
	char  szSector[128];//分区名称
	char  szVer[128];//分区版本
	char cIsupdate;	//是否强制更新
}TYPE_BIOS_PARA;



	
		


typedef struct  
{
	char szFileName[512];	//NLC文件名称
	char szNLCName[512];	//szFileName保存nlc后，又用于保存ini文件，新增加szNLCName，一直用于保存nlc，打印日志使用
	char szFilePath[512];	//NLC解压的路径
	char szMachineS[512];
	BOOL bIsUsed;					//是否启用    默认为FALSE;
	int  nFirmPackVer;		//0:GP包  1:GROUP类型的包
	int	 nFullPack;				//是否是整包固件  0:整包  1:非整包
	int nGroupCount;			//分组数量
	int nBinFileCount;
	GROUP_FILE_ATT szGroupFileAtt[1000];	//文件分组信息存放	
	TYPE_BIOS_PARA szBiospara[100];			// 存放文件列表参数
}NLC_INFO;

typedef struct _ModelInfo{
	int count;
	char info[50][50];
}ModelInfo;

 
#define VID_PID_LEN		8


#define GETLOCALEID		"0730:dcba"
#define GETNLDDEVICE	"GetNldDevice"
#define GETNLPDEVICE	"Sco cu.usbmodualXXXX"


#define FW_FULL		(0)		//全量包
#define OTA_ALL		(1)		//OTA全量包
#define OTA			(2)		//OTA差量包
#define USR_LOGO	(3)		//用户logo区
#define USR_DATA	(4)		//用户data区

typedef struct  
{
	int nMainVer;
	int nMinorVer;
	int nPackVer;
}OTA_VER_INFO;

typedef struct  
{
	string csZipFileName;	//zip文件名
	string csMachine;		//机型类别
	string csVersionLow;	//除OTA包外，其他的全量包、user包此处存储的是当前zip包的版本，OTA包存放版本1
	string csVersionHigh;	//如果是OTA包，存放版本2，其他包此版本不用
	int nZipType;			//zip文件包的类型
	int nDirCount;			//zip解压后的文件夹个数
	OTA_VER_INFO verinfo_L;
	OTA_VER_INFO verinfo_H;
	//OTA_FILE_INFO szOTA[50];	//整包的信息
}OTA_ZIP_INFO;


#endif