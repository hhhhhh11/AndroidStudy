/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * Author: ym
 * Create: 2021/04/19
 * Describe:打包NLD，从windows移植而来，只做不同平台功能代码实现变更， nld.h,nld.cpp
 * 不像其它类重新设计实现
 ******************************************************************************/
#ifndef NLD_H_
#define NLD_H_

#include "codetools.h"
#include "errno.h"
#include "pubdef.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string>

using namespace std;



#define TDES_INPUT_SIZE         8         /*不能修改*/
#define UNIT_BUF_SIZE           2048      /*为8的整数倍，必须与加密时的块大小一致*/

#define HEADINFO_SIZE_OFF 		(0)		 /*头信息长度域偏移*/
#define HEADINFO_SIZE_LEN 		(4)		 /*头信息长度域长度*/


#define NLD_FILE       POS_PATH"/tmp/a.nld"
#define TAR_FILE       POS_PATH"/tmp/a.tar"
#define PUBKEY_FILE    POS_PATH"/appfs/etc/pubkey"

#define EXPIRED_DATE_OFF    0
#define EXPIRED_DATE_SIZE   sizeof(unsigned int)
#define MOD_LEN_OFF         (EXPIRED_DATE_OFF+EXPIRED_DATE_SIZE)
#define MOD_LEN_SIZE        sizeof(unsigned int)
#define MOD_STR_OFF         (MOD_LEN_OFF+MOD_LEN_SIZE)
#define EXP_LEN_SIZE        sizeof(unsigned int)

#define MAX_APPNAME_LEN	 	32
#define MAX_VERSION_LEN		16
#define MAX_TIME_LEN	 	32

#define	ENCRYPT		0		/*加密*/
#define DISCRYPT    1		/*解密*/

//#include "Makenld.h"
#define TAR_GZ_FILENAME				"TmpDlToTar.tar.gz"
#define TAR_FILENAME                 "TmpDlToTar.tar"
#define NLD_FILENAME                 "a.tmpnld"



#define PRIKEY_MOD_FILENAME                 "pri_mod"
#define PRIKEY_EXP_FILENAME                 "pri_exp"

/*NLD头部结构定义*/

/*多应用定义*/
typedef struct tagV2USRPRGINFO {
    char 			cAppName[MAX_APPNAME_LEN + 1];
    char			cVerBuf[MAX_VERSION_LEN + 1];											/*用户程序版本信息					*/
    int				cSeriNo;													/*编号,V2版保留,但意义不同	*/
    char			tBuildTime[MAX_TIME_LEN + 1];												/* 文件编译时间							*/
    int				bIsMaster;
    unsigned long	sReverse[3];				/*保留域*/
} V2USRPRGINFO;		

//只有部分结构体是1字节对齐，其余采用默认的4字节对齐
#pragma pack(push)
#pragma pack(1)

typedef struct tagHEADERINFO 
{
	unsigned int 		uiHeadSize;				/*头部信息的长度*/
	char				cMachineType[10];		/*机器型号*/
	char				cBIOS[18];				/*BIOS版本号，例如"NL-GP730 V1.00C" */
	char				cType;					/*文件下载类型，有应用'U'、参数'P'、固件'F'和公钥'K'*/
	unsigned int 		uiKeyExpiredDate;		/*公钥的有效期*/
	unsigned int		uiTarFileLen;			/* NLD包中Tar或证书文件的长度*/
	unsigned int		uiSignatureLen;			/* NLD包中签名信息的长度*/
	V2USRPRGINFO		UserPrgInfo;			/*多应用信息结构*/
	char				cExtended[32];			/*保留域*/
}NLD_HEADERINFO;		/*头部信息结构*/

#pragma pack(pop)

//公钥文件中保存的公钥数据结构

typedef struct {
	unsigned int expired_date;						/*公钥有效期*/
	unsigned int  mod_str_len;						/*公钥文件中模的字符个数*/
	unsigned char modulus_str[MAX_RSA_MODULUS_LEN]; /*在此是字符串，并非BCD码，2个字符对于一个BCD码*/
	unsigned int  exp_str_len;
	unsigned char exponent_str[MAX_RSA_MODULUS_LEN];/* public exponent */
} STRUCT_PUBKEY;

typedef union 
{
	unsigned int i;
	unsigned char c[4];
} DATA1;

int CreateParaNld(const char *pcMainNLD,const char *pcPackFile,  int nPara,int nIndex);
int GetHeadInfo(const char *pInFilename,NLD_HEADERINFO *pHeaderInfo);
int CreateParaFile(const NLD_HEADERINFO *pHeaderInfo,const char *pcOutName);
int SavePrikeyModulus_From_Array(R_RSA_PUBLIC_KEY *pPrivateKey);
int SavePrivkeyExponent_From_Array(R_RSA_PUBLIC_KEY *pPrivateKey);
int ComputeDigest ( const char *pFilename, unsigned char *pDigest );
int FileEncrypt(const char *pInFilename, const char *pOutFilename);
void DeleFile(const char *szFileName);

#endif