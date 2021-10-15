#define JAVA_CLASS_NAME 	"com/newland/download/ndk/NdkApi"
#define LOG_TAG "DownloadTool_Jni"

#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>
#include <dlfcn.h>
#include "jni.h"
#include "ndk.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <malloc.h>

typedef unsigned char uchar;
typedef unsigned short ushort;

typedef unsigned int uint;

//extern int NDK_MagOpen();
//extern int NDK_MagReset();
//extern int NDK_MagClose();
//extern int NDK_MagReadNormal(char * pszTk1, char * pszTk2, char * pszTk3,
//		int * pnErrorCode);
//extern int NDK_MagSwiped(uchar *psSwiped);
//extern int NDK_LedStatus(EM_LED emStatus);
//extern int NDK_SecGetPin  ( uchar  ucKeyIdx, uchar *  pszExpPinLenIn, const uchar *  pszDataIn,
//		  uchar *  psPinBlockOut, uchar  ucMode,uint  nTimeOutMs);
//extern int NDK_SecGetPinResult( uchar *psPinBlock,int *nStatus);
//extern int NDK_SecLoadKey(ST_SEC_KEY_INFO * pstKeyInfoIn,
//		ST_SEC_KCV_INFO * pstKcvInfoIn);
//extern int NDK_KbGetCode(uint unTime,int *pnCode);
//extern int NDK_SysBeep(void);
//extern int NDK_SysGetPosInfo(EM_SYS_HWINFO emFlag,uint *punLen,char *psBuf);
//extern int NDk_SysGetK21Version(char *psBuf);
int (*NDK_KbFlush)(void);
int(*NDk_SysGetK21Version)(char *psBuf);
int(*NDK_SysGetPosInfo)(EM_SYS_HWINFO emFlag,uint *punLen,char *psBuf);
int(*NDK_FsExist)(const char *pszName);
int(*NDK_FsOpen)(const char *pszName, const char *pszMode);
int(*NDK_FsClose)(int nHandle);
int(*NDK_FsRead)(int nHandle, char * psBuffer, uint unLength);
int(*NDK_IccDetect)(int *pnSta);
int(*NDK_IccPowerUp )(EM_ICTYPE emIcType, uchar *psAtrBuf,int *pnAtrLen);
int(*NDK_Iccrw)(EM_ICTYPE emIcType, int nSendLen,  uchar *psSendBuf, int *pnRecvLen,  uchar *psRecvBuf);

int (*NDK_RfidInit)(uchar *psStatus);
int (*NDK_RfidPiccType)(uchar ucPiccType);
int (*NDK_RfidPiccActivate)(uchar *psPiccType, int *pnDataLen,  uchar *psDataBuf);
int (*NDK_RfidPiccDeactivate)(uchar ucDelayMs);
int (*NDK_M1Request)(uchar ucReqCode, int *pnDataLen, uchar *psDataBuf);

int (*NDK_PrnInit)(uint unPrnDirSwitch);
int (*NDK_PrnSetFont)(EM_PRN_HZ_FONT emHZFont,EM_PRN_ZM_FONT emZMFont);
int (*NDK_PrnGetStatus)(EM_PRN_STATUS *pemStatus);
int (*NDK_PrnSetForm)(uint unBorder,uint unColumn, uint unRow);
int (*NDK_PrnStr)(const char *pszBuf);
int (*NDK_PrnStart)(void);
int (*NDK_Script_Print)(char* prndata,int indata_len);
int (*NDK_SecVppTpInit)(uchar *num_btn, uchar *func_key, uchar *out_seq);
int (* NDK_SecSetKeyOwner)(char *pszName);
int (*NDK_RfidPiccDetect)(uchar *psPiccType);
int (*NDK_SecGetKcv)(uchar ucKeyType, uchar ucKeyIdx, ST_SEC_KCV_INFO *pstKcvInfoOut);
int (*NDK_SecGetDukptKsn)(uchar ucGroupIdx, uchar * psKsnOut);
int (*NDK_KbGetCode)(uint unTime, int *pnCode);
int (*NDK_SysBeep)(void);
int (*NDK_SecGetPinResult)(uchar *psPinBlock, int *nStatus);
int (*NDK_FsSeek)(int nHandle, ulong ulDistance, uint unPosition );
int (*NDK_MagOpen)(void);
int (*NDK_MagClose)(void);
int (*NDK_MagReset)(void);
int (*NDK_MagSwiped)(uchar * psSwiped);
int (*NDK_MagReadNormal)(char *pszTk1, char *pszTk2, char *pszTk3, int *pnErrorCode);
int (*NDK_LedStatus)(EM_LED emStatus);
int (*NDK_SecGetPin)(uchar ucKeyIdx, uchar *pszExpPinLenIn,const uchar * pszDataIn, uchar *psPinBlockOut, uchar ucMode, uint nTimeOutMs);
int (*NDK_SecLoadKey)(ST_SEC_KEY_INFO * pstKeyInfoIn, ST_SEC_KCV_INFO * pstKcvInfoIn);
int (*NDK_FsFileSize)(const char *pszName,uint *punSize);
int (*NDK_IccPowerDown)(EM_ICTYPE emIcType);
int (*NDK_Getlibver)(char *pszVer);
char* (*NDK_szGetBuildingTime)();
char* (*NDK_szGetBuildingDate)();
int (*NDK_PrnFeedByPixel)(uint pixel);
int (*NDK_M1Anti)(int *pnDataLen, uchar *psDataBuf);
int (*NDK_PortOpen)(EM_PORT_NUM emPort, const char *pszAttr);
int (*NDK_PortClose)(EM_PORT_NUM emPort);
int (*NDK_PortRead)(EM_PORT_NUM emPort, uint unLen, char *pszOutBuf,int nTimeoutMs, int *pnReadLen);
int (*NDK_PortWrite)(EM_PORT_NUM emPort, uint unLen,const char *pszInbuf);
int (*NDK_PortClrBuf)(EM_PORT_NUM emPort);
int (*NDK_PortReadLen)(EM_PORT_NUM emPort,int *pnReadLen);


const char* (*NDK_SDK_EP_GetVersion)(void);
const char* (*NDK_SDK_PP_GetVersion)(void);
const char* (*NDK_SDK_PW_GetVersion)(void);
const char* (*NDK_SDK_Qpboc_GetVersion)(void);
int (*NDK_RfidVersion)(uchar *pszVersion);
int (*_NDK_SecGetMposKeyOwner)(char *pszOwner, char* flag);
int (*_NDK_SecGetServKeyOwner)(char *pszOwner, char* flag);
int (*NDK_IccSetPowerUpMode)(int pnMode, int pnVoltage);
int (*NDK_RfidPiccDetect_Atq)(uchar *psPicctype, int *pnAtqlen, uchar* psAtqbuf);
int (*NDK_MifareActive)(uchar ucReqCode,uchar *psUID, uchar *pnUIDLen, uchar *psSak);
const char *(*NAPI_EMVL2GetVersion)(void);

const char *(*NAPI_CLL2_EntryPointGetVersion)(void);

const char *(* NAPI_CLL2_PaypassGetVersion)(void);

const char *(* NAPI_CLL2_PaywaveGetVersion)(void);

const char *(* NAPI_CLL2_QpbocGetVersion)(void);

const char *(* NAPI_CLL2_ExpresspayGetVersion)(void);

const char *(* NAPI_CLL2_DiscoverpayGetVersion)(void);

const char *(* NAPI_CLL2_PureGetVersion)(void);

const char *(* NAPI_CLL2_JCBGetVersion)(void);

const char *(* NAPI_CLL2_InteracGetVersion)(void);

const char *(* NAPI_CLL2_RupayGetVersion)(void);

const char *(* NAPI_CLL2_MIRGetVersion)(void);
const char *(* NAPI_CLL2_MultibancoGetVersion)(void);

int (*NAPI_EMVL2GetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_EntryPointGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_PaypassGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_PaywaveGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_ExpresspayGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_DiscoverpayGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_InteracGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_JCBGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_PureGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_RupayGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_QpbocGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_MIRGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NAPI_MultibancoGetKernelChecksum)(unsigned char *checksum, unsigned int size);
int (*NDK_SetCheckinCardEventFlag)(int flag);
int (*NDK_SecKeyDelete)(uchar ucKeyIdx,uchar ucKeyType);

int (*NAPI_SecGetKeyInfo)(EM_SEC_KEY_INFO_ID InfoID, uchar ucKeyID, EM_SEC_CRYPTO_KEY_TYPE KeyType, EM_SEC_KEY_USAGE KeyUsage,
						  uchar *pAD, uint unADSize, uchar *psOutInfo, int *pnOutInfoLen);
#define _DEBUG 1
#ifdef _DEBUG
//#define LOGI(...) __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
//#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

#define TAG "downloadTool-jni"// 这个是自定义的LOG的标识

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

#define TEST_ver 1

#else
#define LOGI(...) 	;
#endif

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

#define unchar unsinged uchar
#define INPUT_CHECK	1
#define JNI_INPUT_ERR		(-101)
#define ERRMSG_SIZE		(20)

#define JNI_OUTPUTBUF_MAX	(150)
#define	JNI_METHOD_NOT_FOUND	(64)

#define KEY_DATA_FILE "/appfs/ak_main.data"
#define KEY_IDX_FILE "/appfs/ak_main.idx"
#define MKSK_KEY_LEN			24
int hasEmv2 = 1;
typedef struct PHOENIX_KEY_UNIT_S {
	unsigned int id;
	unsigned int arch;
	unsigned int type;
	unsigned int len;
	unsigned char value[0];
} PHOENIX_KEY_UNIT;

#define   _MASK(__n,__s)  	   (((1<<(__s))-1)<<(__n))
#define 	KEY_TYPE_MASK      _MASK(0, 6)
#define 	KEY_ALG_MASK	   _MASK(6, 2)

typedef struct{
	int keyid;
	int keyoffset;
}KEY_IDX_REC;

JNIEnv* getJNIEnv();

int jniRegisterNativeMethods(JNIEnv* env, const char* className,
		const JNINativeMethod* gMethods, int numMethods);

static JavaVM *sVm;

//#define TAG LOG_TAG

#define DLSYM(lib, foo) {	\
	foo =dlsym( lib , #foo);		\
	if(NULL == foo ){		\
	dlError = (char *)dlerror();					\
		foo = NDK_Null;	\
		rc -= 1;	\
		__android_log_print(ANDROID_LOG_INFO, TAG,"dlsym fail:  %s . "#foo"=%x ,ret will be %x\n", dlError,(unsigned int)foo,(unsigned int)NDK_Null);	\
		} \
	};\

int NDK_Null()	{
		return -10001;
	}

void  *functionLib;         /*  Handle to shared lib file   */
char *dlError;

enum KEY_PARSE_ERROR{
	KEY_DATA_FILE_NOT_EXIST = -2,
	KEY_DATA_FILE_SIZE_ERROR = -3,
	KEY_DATA_FILE_OPEN_FAILED = -4,
	KEY_DATA_ARCH_ERROR = -5,
	KEY_DATA_LEN_ERROR = -6,
	KEY_DATA_FILE_READ_FAILED = -7,
	KEY_DATA_TYPE_ERROR = -8,
	KEY_INDEX_NOT_MATCH = -9, //密钥索引不匹配
	KEY_OFFSET_NOT_MATCH = -10, //密钥偏移量不匹配

	KEY_IDX_FILE_NOT_EXIST = -12,
	KEY_IDX_FILE_SIZE_ERROR = -13,
	KEY_IDX_FILE_OPEN_FAILED = -14,
	KEY_IDX_ID_ERROR = -15,
	KEY_IDX_FILE_READ_FAILED = -16,
	KEY_IDX_FILE_IDX_REPEAT = -17,
};

static int parse_key_data(unsigned char *data_buf, int buf_len)
{
	PHOENIX_KEY_UNIT *p_stKey;
	int len=0;
	unsigned char *p;
	int j;

	while(len<(buf_len)){
		p_stKey = (PHOENIX_KEY_UNIT *)(data_buf+len);
		p = (unsigned char *)p_stKey + sizeof(PHOENIX_KEY_UNIT);
		if(p_stKey->arch != KEY_ARCH_CUSTOM){
			LOGD("key info:id=%04x arch=%d type=%d len=%d data=", p_stKey->id, p_stKey->arch, p_stKey->type, p_stKey->len);
			LOGD("key arch error!\n");
			return KEY_DATA_ARCH_ERROR;
		}

		//校验密钥长度是否合法
		if(p_stKey->len > (buf_len - sizeof(PHOENIX_KEY_UNIT) - 8 -len)){
			LOGD("key info:id=%04x arch=%d type=%d len=%d data=", p_stKey->id, p_stKey->arch, p_stKey->type, p_stKey->len);
			LOGD("key len error!\n");
			return KEY_DATA_LEN_ERROR;
		}

		//打印密钥密文
		LOGD("key info:id=%04x arch=%d type=%d len=%d data=", p_stKey->id, p_stKey->arch, p_stKey->type, p_stKey->len);
		for (j=0; j< p_stKey->len; j++){
			printf("%02x", p[j]);
		}
		//打印mac值
		LOGD(" mac=");
		p =  (unsigned char *)p_stKey + sizeof(PHOENIX_KEY_UNIT) + MKSK_KEY_LEN;
		for (j=0; j<8; j++){
			printf("%02x", p[j]);
		}
		LOGD("\r\n");
		len += sizeof(PHOENIX_KEY_UNIT) + 32;
	}


	return 0;
}

static int parse_key_data_by_index(unsigned char *data_buf, int buf_len, KEY_IDX_REC *index)
{
	PHOENIX_KEY_UNIT *p_stKey;
	int len= index->keyoffset;
	unsigned char *p;
	int j,i=0;

	if(len >= buf_len){
		return KEY_OFFSET_NOT_MATCH;
	}

	p_stKey = (PHOENIX_KEY_UNIT *)(data_buf+len);
	p = (unsigned char *)p_stKey + sizeof(PHOENIX_KEY_UNIT);

	//检查id是否匹配
	if(p_stKey->id != index->keyid){
		LOGD("key index:id=%04x key data:id=%04x", p_stKey->id, index->keyid);
		LOGD("key index error!\n");
		return KEY_INDEX_NOT_MATCH;
	}

	//检查arch
	if(p_stKey->arch != KEY_ARCH_CUSTOM){
		LOGD("key arch=%d", p_stKey->arch);
		LOGD("key arch error!\n");
		return KEY_DATA_ARCH_ERROR;
	}

	//检查type
	p_stKey->type &= KEY_TYPE_MASK;
	if(p_stKey->type < SEC_KEY_TYPE_TLK ||
			(p_stKey->type > SEC_KEY_TYPE_MAX &&
					p_stKey->type != SEC_KEY_TYPE_DUKPT &&
					p_stKey->type != SEC_KEY_TYPE_RSA )){
		LOGD("key type:type=%d", p_stKey->type);
		LOGD("key type error!\n");
		return KEY_DATA_TYPE_ERROR;
	}

	//检查len
	if(p_stKey->type == SEC_KEY_TYPE_DUKPT) {
		if(p_stKey->len != 368){
			LOGD("DUKPT key len=%d", p_stKey->len);
			LOGD("DUKPT key len error!\n");
			return KEY_DATA_LEN_ERROR;
		}
	}else{
		if(p_stKey->len != 8 &&
				p_stKey->len != 16 &&
				p_stKey->len != 24 ){
			LOGD("key len=%d", p_stKey->len);
			LOGD("key len error!\n");
			return KEY_DATA_LEN_ERROR;
		}
	}

	LOGD("key info:id=%04x arch=%d type=%d len=%d", p_stKey->id, p_stKey->arch, p_stKey->type, p_stKey->len);

	return 0;
}

static int parse_key_idx(unsigned char *data_buf, int buf_len)
{
	KEY_IDX_REC *p_stKey;
	int len=0;
	unsigned char *p;
	int j;
	int ex_keyid =  -1;

	while(len<(buf_len)) {
		p_stKey = (KEY_IDX_REC *)(data_buf+len);
		p = (unsigned char *)p_stKey + sizeof(KEY_IDX_REC);
		LOGD("keyid = [%04x], offset = %[%04x]\n", p_stKey->keyid, p_stKey ->keyoffset);
		if(ex_keyid == p_stKey->keyid){
			LOGD("key id error!\n");
			return KEY_IDX_ID_ERROR;
		}
		ex_keyid = p_stKey->keyid;
		len += sizeof(KEY_IDX_REC);
	}
	return 0;
}

//解析索引文件，如果成功则返回记录的条数
static int parse_key_idx_return_index_total(unsigned char *data_buf, int buf_len, int* index_total)
{
	KEY_IDX_REC *p_stKey;
	int len=0;
	unsigned char *p;
	int j;
	int ex_keyid =  -1;
	int total = 0;

	while(len<(buf_len)) {
		p_stKey = (KEY_IDX_REC *)(data_buf+len);
		p = (unsigned char *)p_stKey + sizeof(KEY_IDX_REC);
		LOGD("keyid = [%04x], offset = %[%04x]\n", p_stKey->keyid, p_stKey ->keyoffset);
		if(ex_keyid == p_stKey->keyid){
			LOGD("key id error!\n");
			return KEY_IDX_ID_ERROR;
		}
		ex_keyid = p_stKey->keyid;
		len += sizeof(KEY_IDX_REC);
		total++;
		*index_total = total;
		LOGD("total = %d\n", total);
	}
	return 0;
}


int ndk_dlload( ){
		  int   rc;             /*  return codes            */
    functionLib = dlopen("libnlposapi.npt.so",RTLD_LAZY);
    if (functionLib == NULL){
        functionLib = dlopen("libnlposapi.so",RTLD_LAZY);
        dlError = (char *)dlerror();
        if( functionLib == NULL ) return(-1000);
    }
		   rc = 0;
	DLSYM(functionLib,NDK_KbFlush);
    DLSYM(functionLib,NDK_SysGetPosInfo);
    DLSYM(functionLib,NDk_SysGetK21Version);
		    DLSYM(functionLib,NDK_FsExist);
		   	DLSYM(functionLib,NDK_FsOpen);
		    DLSYM(functionLib,NDK_FsClose);
		   	DLSYM(functionLib,NDK_FsRead);
		   	DLSYM(functionLib,NDK_IccDetect);
		   	DLSYM(functionLib,NDK_IccPowerUp);
		   	DLSYM(functionLib,NDK_Iccrw);
		   	DLSYM(functionLib,NDK_RfidInit);
		   	DLSYM(functionLib,NDK_RfidPiccType);
		   	DLSYM(functionLib,NDK_RfidPiccActivate);
		   	DLSYM(functionLib,NDK_RfidPiccDeactivate);
		   	DLSYM(functionLib,NDK_M1Request);
		   	DLSYM(functionLib,NDK_PrnInit);
		   	DLSYM(functionLib,NDK_PrnSetFont);
		   	DLSYM(functionLib,NDK_PrnGetStatus);
		   	DLSYM(functionLib,NDK_PrnSetForm);
		   	DLSYM(functionLib,NDK_PrnStr);
		   	DLSYM(functionLib,NDK_PrnStart);
			DLSYM(functionLib,NDK_SecVppTpInit);
			DLSYM(functionLib,NDK_SecSetKeyOwner);
			DLSYM(functionLib,NDK_RfidPiccDetect);
			DLSYM(functionLib,NDK_SecGetKcv);
			DLSYM(functionLib,NDK_SecGetDukptKsn);
			DLSYM(functionLib,NDK_KbGetCode);
			DLSYM(functionLib,NDK_SysBeep);
			DLSYM(functionLib,NDK_SecGetPinResult);
			DLSYM(functionLib,NDK_FsSeek);
			DLSYM(functionLib,NDK_MagOpen);
			DLSYM(functionLib,NDK_MagClose);
			DLSYM(functionLib,NDK_MagReset);
			DLSYM(functionLib,NDK_MagSwiped);
			DLSYM(functionLib,NDK_MagReadNormal);
			DLSYM(functionLib,NDK_LedStatus);
			DLSYM(functionLib,NDK_SecGetPin);
			DLSYM(functionLib,NDK_SecLoadKey);
			DLSYM(functionLib,NDK_FsFileSize);
			DLSYM(functionLib,NDK_IccPowerDown);
			DLSYM(functionLib,NDK_Getlibver);
			DLSYM(functionLib,NDK_szGetBuildingTime);
			DLSYM(functionLib,NDK_szGetBuildingDate);            
            DLSYM(functionLib, NDK_PrnFeedByPixel);
            DLSYM(functionLib, NDK_SDK_EP_GetVersion);
            DLSYM(functionLib, NDK_SDK_PP_GetVersion);
            DLSYM(functionLib, NDK_SDK_PW_GetVersion);
            DLSYM(functionLib, NDK_SDK_Qpboc_GetVersion);
            DLSYM(functionLib, NDK_RfidVersion);
            DLSYM(functionLib, NDK_M1Anti);
             DLSYM(functionLib, NDK_RfidPiccDetect_Atq);
    DLSYM(functionLib, NDK_MifareActive);
    DLSYM(functionLib, NDK_PortOpen);
    DLSYM(functionLib, NDK_PortClose);
    DLSYM(functionLib, NDK_PortRead);
    DLSYM(functionLib, NDK_PortWrite);
    DLSYM(functionLib, NDK_PortClrBuf);
    DLSYM(functionLib, NDK_PortReadLen);
	        DLSYM(functionLib, _NDK_SecGetServKeyOwner);
	        DLSYM(functionLib, _NDK_SecGetMposKeyOwner);
	        DLSYM(functionLib, NDK_IccSetPowerUpMode);
	DLSYM(functionLib, NDK_SetCheckinCardEventFlag);
    DLSYM(functionLib, NDK_SecKeyDelete);

	DLSYM(functionLib,NAPI_SecGetKeyInfo);

    functionLib = dlopen("libnl_ndk.npt.so",RTLD_LAZY);
    if (functionLib == NULL){
        functionLib = dlopen("libnl_ndk.so",RTLD_LAZY);
        dlError = (char *)dlerror();
        if( functionLib == NULL ) return(-2000);
    }

    functionLib = dlopen("libnlemvl2.so",RTLD_LAZY);
    dlError = (char *)dlerror();
    if( functionLib == NULL ) {
        LOGD("dlopen  libnlemvl2 error\n");
        hasEmv2 = 0;
        return(-1000);
    }
    rc = 0;
    DLSYM(functionLib,NAPI_EMVL2GetKernelChecksum);
    DLSYM(functionLib,NAPI_EntryPointGetKernelChecksum);
    DLSYM(functionLib,NAPI_PaypassGetKernelChecksum);
    DLSYM(functionLib,NAPI_PaywaveGetKernelChecksum);
    DLSYM(functionLib,NAPI_ExpresspayGetKernelChecksum);
    DLSYM(functionLib,NAPI_DiscoverpayGetKernelChecksum);
    DLSYM(functionLib,NAPI_InteracGetKernelChecksum);
    DLSYM(functionLib,NAPI_JCBGetKernelChecksum);
    DLSYM(functionLib,NAPI_PureGetKernelChecksum);
    DLSYM(functionLib,NAPI_RupayGetKernelChecksum);
    DLSYM(functionLib,NAPI_QpbocGetKernelChecksum);
    DLSYM(functionLib,NAPI_MIRGetKernelChecksum);
    DLSYM(functionLib,NAPI_MultibancoGetKernelChecksum);
    DLSYM(functionLib,NAPI_EMVL2GetVersion);
    DLSYM(functionLib,NAPI_CLL2_EntryPointGetVersion);
    DLSYM(functionLib,NAPI_CLL2_PaypassGetVersion);
    DLSYM(functionLib,NAPI_CLL2_PaywaveGetVersion);
    DLSYM(functionLib,NAPI_CLL2_QpbocGetVersion);
    DLSYM(functionLib,NAPI_CLL2_ExpresspayGetVersion);
    DLSYM(functionLib,NAPI_CLL2_DiscoverpayGetVersion);
    DLSYM(functionLib,NAPI_CLL2_PureGetVersion);
    DLSYM(functionLib,NAPI_CLL2_JCBGetVersion);
    DLSYM(functionLib,NAPI_CLL2_InteracGetVersion);
    DLSYM(functionLib,NAPI_CLL2_RupayGetVersion);
    DLSYM(functionLib,NAPI_CLL2_MIRGetVersion);
    DLSYM(functionLib,NAPI_CLL2_MultibancoGetVersion);
    return rc;
	}


//int(*NDK_SysGetPosInfo)(EM_SYS_HWINFO emFlag,uint *punLen,char *psBuf);
jint POS_NDK_GetSN(JNIEnv* env,jobject obj,jintArray jLen,jbyteArray jBuf){
	int nRet;
	int *punLen = NULL;
	char *psBuf = NULL;

	if (jLen != NULL){
		punLen = (int *)(*env)->GetIntArrayElements(env,jLen,NULL);
	}
	if (jBuf != NULL){
		psBuf = (char *)(*env)->GetByteArrayElements(env,jBuf,NULL);
	}

	nRet = NDK_SysGetPosInfo(SYS_HWINFO_GET_POS_USN,punLen,psBuf);
	LOGD("NDK_SysGetPosInfo SN nRet %d ",nRet);

	if (punLen != NULL){
		(*env)->ReleaseIntArrayElements(env,jLen,(jint *)punLen,0);
	}
	if (psBuf != NULL){
		(*env)->ReleaseByteArrayElements(env,jBuf,(jchar *)psBuf,0);
	}

	return nRet;
}




/*JNI������ */
static const JNINativeMethod method_table[] = {
		{"JNI_NDK_GetSN","([I[B)I",(void *)POS_NDK_GetSN},
};
/*static const JNINativeMethod method_table1[] = {
		{"NDK_SecGetKcv","(II)I",(void *)_NDK_SecGetKcv},
};*/

/*ע��JNI����*/

/* >>>>>>>>>>>>>>>>>>>>>>> */

/*
 * Register native JNI-callable methods.
 *
 * "className" looks like "java/lang/String".
 */

int jniRegisterNativeMethods(JNIEnv* env, const char* className,
		const JNINativeMethod* gMethods, int numMethods) {
	jclass clazz;

    LOGE("Registering %s natives\n",
			className);
#ifdef __cplusplus
	clazz = env->FindClass(className);
	if (clazz == NULL) {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Native registration unable to find class '%s'\n", className);
		return -1;
	}
	if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
		__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "RegisterNatives failed for '%s'\n", className);
		return -1;
	}
	return 0;

#else
	clazz = (*env)->FindClass(env, className);

	if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'\n", className);
		return -1;
	}
	if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'\n", className);
		return -1;
	}
	return 0;
#endif
}

int register_JNILib_im81ndk(JNIEnv *env) {
	return jniRegisterNativeMethods(env, JAVA_CLASS_NAME, method_table,
			NELEM(method_table));
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = JNI_ERR;
	sVm = vm;
	jclass cls;

#ifdef __cplusplus

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
#else
	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
#endif
        LOGE("GetEnv failed!");
		return result;
	}

    LOGI( "loading81 .123 . .");

	if (register_JNILib_im81ndk(env) != JNI_OK) {
        LOGE("can't load register_JNILib_im81ndk():  %s",
				JAVA_CLASS_NAME);
		goto end;
	}
    LOGI("loaded");
	result = ndk_dlload();
	result = JNI_VERSION_1_4;

	end: return result;
}

void newlandAPI_printf(char * fmt, ...) {
	char buffer[80];

	va_list args;
	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	LOGI("printf call from demoLib:  %s",buffer);
//	LOGI(buffer);
	va_end(args);

}
