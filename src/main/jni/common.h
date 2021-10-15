#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>

#define MAX_ARD_NUM 10
#define MAX_DOWNFILE_NUM		200 //最大下载文件数量 add 2021/01/26
#define MAX_PATH          512      //max path length


#define DOWN_SUCC "Download Successful."


using namespace std;
typedef unsigned short UINT16;




enum iCmdCruxPlus {
	cmd_cruxplus_boot,
	cmd_cruxplus_reset,
	cmd_cruxplus_dwnnlp,
	cmd_cruxplus_dwnfile,
	cmd_cruxplus_timesync,
	cmd_cruxplus_clearapp,
	cmd_cruxplus_exitdwn,
	cmd_cruxplus_shutdown,
	cmd_cruxplus_dwnsha256,
};


enum icmdCruxScrop{
	cmd_scrop_boot,
	cmd_scrop_clearapp = 0x09
};

enum iMPosPlat{
    mpos_plat_scrop,
    mpos_plat_cruxplus
};

typedef struct _ST_DOWNFILE_ATT
{
	string csFileName;	//下载文件名，不包含路径
	string csFileType;	//文件类型：主程序、程序、参数文件、文件、单应用程序、子应用程序、应用库、主控程序、BIOS程序、HEX文件
	string csFilePath;	//文件在磁盘上的路径
	string csMachineType;//机具类型:
	char    cLoadType;	//0:单一方式加载   1:NZP文件加载   2:配置文件加载
	char    cParaPos;	//参数文件或者需要打包文件对应主程序所在位置
}ST_DOWNFILE_ATT;



 typedef int ( *PFCALLBACK)(char* pszFile,int nProgress);
#endif