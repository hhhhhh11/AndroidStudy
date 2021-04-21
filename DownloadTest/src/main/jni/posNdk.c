#include <stdio.h>
#include <android/log.h>

#include "jni.h"

#include "include/ndk.h"
#define TAG "Lib-PosNdk"

#include <dlfcn.h>
void  *functionLib;         /*  Handle to shared lib file   */
char *dlError;        /*  Pointer to error string     */


//kb
int (*NDK_KbFlush)(void);
int (*NDK_KbGetCode)(unsigned int unTime, int *pnCode);
int (*NDK_KbHit)(int *pnCode);
int (*NDK_SysBeep)(void);
int (*NDK_KbGetInput)(char *pszBuf,unsigned int unMinLen,unsigned int unMaxLen,
	                                    unsigned int *punLen,EM_INPUTDISP emMode,unsigned int unWaitTime,
	                                    EM_INPUT_CONTRL emControl);


//mag
int (*NDK_MagOpen)(void);
int (*NDK_MagClose)(void);
int (*NDK_MagReset)(void);
int (*NDK_MagSwiped)(unsigned char * psSwiped);
int (*NDK_MagReadNormal)(char *pszTk1, char *pszTk2, char *pszTk3, int *pnErrorCode);
int (*NDK_MagReadRaw)(uchar *pszTk1, ushort* pusTk1Len, uchar *pszTk2, ushort* pusTk2Len,uchar *pszTk3, ushort* pusTk3Len );
int (*NDK_MagReadRawData)(ENUM_MAG_DATA_TYPE type, ENUM_MAG_TRACK track, uint off, uint unLen, uchar *tkdata, uint *pnReadlen);

//print
int (*NDK_PrnInit)(uint unPrnDirSwitch);
int (*NDK_PrnStr)(const char *pszBuf);
int (*NDK_PrnStart)(void);
int (*NDK_PrnImage)(uint unXsize,uint unYsize,uint unXpos,const char *psImgBuf);
int (*NDK_PrnGetVersion)(char *pszVer);
int (*NDK_PrnSetFont)(EM_PRN_HZ_FONT emHZFont,EM_PRN_ZM_FONT emZMFont);
int (*NDK_PrnGetStatus)(EM_PRN_STATUS *pemStatus);
int (*NDK_PrnSetMode)(EM_PRN_MODE emMode,uint unSigOrDou);
int (*NDK_PrnSetGreyScale)(uint unGrey);
int (*NDK_PrnSetForm)(uint unBorder,uint unColumn, uint unRow);
int (*NDK_PrnFeedByPixel)(uint unPixel);
int (*NDK_PrnSetUnderLine)(EM_PRN_UNDERLINE_STATUS emStatus);
int (*NDK_Script_Print)(char* prndata,int indata_len);

//file
int (*NDK_FsOpen)(const char *pszName,const char *pszMode);
int (*NDK_FsClose)(int nHandle);
int (*NDK_FsRead)(int nHandle, char *psBuffer, uint unLength );
int (*NDK_FsWrite)(int nHandle, const char *psBuffer, uint unLength );
int (*NDK_FsSeek)(int nHandle, ulong ulDistance, uint unPosition );
int (*NDK_FsDel)(const char *pszName);
int (*NDK_FsFileSize)(const char *pszName,uint *punSize);
int (*NDK_FsExist)(const char *pszName);
int (*NDK_FsTruncate)(const char *pszPath ,uint unLen );
int (*NDK_FsTell)(int nHandle,ulong *pulRet);
int (*NDK_FsRename)(const char *pszSrcName, const char *pszDstName );
int (*NDK_FsFormat)(void);
int (*NDK_CopyFileToSecMod)(const unsigned char* sourcefile, const unsigned char* destfile);

//tool
int (*NDK_AddDigitStr)(const uchar *pszDigStr1, const uchar *pszDigStr2, uchar* pszResult, int *pnResultLen );
int (*NDK_IncNum )(uchar * pszStrNum );
int (*NDK_FmtAmtStr )(const uchar* pszSource, uchar* pszTarget, int* pnTargetLen );
int (*NDK_AscToHex )(const uchar* pszAsciiBuf, int nLen, uchar ucType, uchar* psBcdBuf);
int (*NDK_HexToAsc )(const uchar* psBcdBuf, int nLen, uchar ucType, uchar* pszAsciiBuf);
int (*NDK_IntToC4 )(uchar* psBuf, uint unNum );
int (*NDK_IntToC4 )(uchar* psBuf, uint unNum );
int (*NDK_C4ToInt)(uint* unNum, uchar* psBuf );
int (*NDK_ByteToBcd)(int nNum, uchar *psCh);
int (*NDK_BcdToByte)(uchar ucCh, int *pnNum);
int (*NDK_IntToBcd)(uchar *psBcd, int *pnBcdLen, int nNum);
int (*NDK_BcdToInt)(const uchar * psBcd, int *nNum);
int (*NDK_CalcLRC)(const uchar *psBuf, int nLen, uchar *ucLRC);
int (*NDK_LeftTrim)(uchar *pszBuf);
int (*NDK_RightTrim)(uchar *pszBuf);
int (*NDK_AllTrim)(uchar *pszBuf);
int (*NDK_AddSymbolToStr)(uchar *pszString, int nLen, uchar ucCh, int nOption);
int (*NDK_SubStr)(const uchar *pszSouStr, int nStartPos, int nNum, uchar *pszObjStr, int *pnObjStrLen);
int (*NDK_IsDigitChar)(uchar ucCh);
int (*NDK_IsDigitStr)(const uchar *pszString);
int (*NDK_IsLeapYear)(int nYear);
int (*NDK_MonthDays)(int nYear, int nMon, int *pnDays);
int (*NDK_IsValidDate)(const uchar *pszDate);


//app
int (*NDK_AppRun)(const char *pszAppName);
int (*NDK_AppLoad)(const char *pszFileName, int nRebootFlag);
int (*NDK_AppDel)(const char *pszAppName);

//ic
int (*NDK_IccGetVersion)(char *version);
int (*NDK_IccPowerUp )(EM_ICTYPE emIctype, unsigned char *psAtrbuf,int *pnAtrlen);
int (*NDK_IccPowerDown)(EM_ICTYPE emIctype);
int (*NDK_IccDetect)(int *pnSta);
int (*NDK_Iccrw)(EM_ICTYPE emIcType, int nSendLen,  unsigned char *psSendBuf, int *pnRecvLen,  unsigned char *psRecvBuf);

//sys
int (*NDK_SysBeep)(void);
int (*NDK_Getlibver)(char *version);
int (*NDK_SysTimeBeep)(unsigned int unFrequency,unsigned int unSeconds);
int (*NDK_SysSetPosTime)(struct tm stTime);
int (*NDK_SysGetPosTime)(struct tm *pstTime);
int (*NDK_SysStartWatch)(void);
int (*NDK_SysStopWatch)(unsigned int *punTime);
int (*NDK_SysDelay)(unsigned int unDelayTime);
int (*NDK_SysMsDelay)(unsigned int unDelayTime);
int (*NDK_SysExit)(int nErrCode);
int (*NDK_SysReboot)(void);
int (*NDK_SysShutDown)(void);
int (*NDK_SysSetBeepVol)(unsigned int unVolNum);
int (*NDK_SysGetBeepVol)(unsigned int *punVolNum);
int (*NDK_SysSetSuspend)(unsigned int  unFlag);
int (*NDK_SysGoSuspend)(void);
int (*NDK_SysGetPowerVol)(unsigned int *punVol);
int (*NDK_LedStatus)(EM_LED emStatus);
int (*NDK_SysReadWatch)(unsigned int *punTime);
int (*NDK_SysGetPosInfo)(EM_SYS_HWINFO emFlag,unsigned int *punLen,char *psBuf);
int (*NDK_SysGetConfigInfo)(EM_SYS_CONFIG emConfig,int *pnValue);
int (*NDK_SysInitStatisticsData)(void);
int (*NDK_SysGetStatisticsData)(EM_SS_DEV_ID emDevId,unsigned long *pulValue);
int (*NDK_SysGetFirmwareInfo)(EM_SYS_FWINFO *emFWinfo);
int (*NDK_SysTime)(unsigned long *ulTime);
int (*NDK_SysSetSuspendDuration)(unsigned int unSec);
int (*NDK_SysGetPowerVolRange)(unsigned int *punMax,unsigned int *punMin);
int (*NDK_SysKeyVolSet)(uint sel);
int (*NDK_SysPeerOper)(EM_SYS_PEEROPER oper);
int (*NDK_SysEnterBoot)(void);
int (*NDK_SysSetPosInfo)(EM_SYS_HWINFO emFlag, const char *psBuf);
int (*NDk_SysGetK21Version)(char *version);
int (*NDK_SysWakeUp)(void);


//rf
int (*NDK_RfidVersion)(unsigned char *pszVersion);
int (*NDK_RfidInit)(uchar *psStatus);
int (*NDK_RfidOpenRf)(void);
int (*NDK_RfidCloseRf)(void);
int (*NDK_RfidPiccState)(void);
int (*NDK_RfidSuspend)(void);
int (*NDK_RfidResume)(void);
int (*NDK_RfidPiccType)(uchar ucPicctype);
int (*NDK_RfidPiccDetect)(uchar *psPicctype);
int (*NDK_RfidPiccActivate)(uchar *psPicctype, int *pnDatalen,  uchar *psDatabuf);
int (*NDK_RfidPiccDeactivate)(uchar ucDelayms);
int (*NDK_RfidPiccApdu)(int nSendlen, uchar *psSendbuf, int *pnRecvlen,  uchar *psRecebuf);
int (*NDK_M1Request)(uchar ucReqcode, int *pnDatalen, uchar *psDatabuf);
int (*NDK_M1Anti)(int *pnDatalen, uchar *psDatabuf);
int (*NDK_M1Select)(int nUidlen, uchar *pnUidbuf, uchar *psSakbuf);
int (*NDK_M1KeyStore)(uchar ucKeytype,  uchar ucKeynum, uchar *psKeydata);
int (*NDK_M1KeyLoad)(uchar ucKeytype,  uchar ucKeynum);
int (*NDK_M1InternalAuthen)(int nUidlen, uchar *psUidbuf, uchar ucKeytype, uchar ucBlocknum);
int (*NDK_M1ExternalAuthen)(int nUidlen, uchar *psUidbuf, uchar ucKeytype, uchar *psKeydata, uchar ucBlocknum);
int (*NDK_M1Read)(uchar ucBlocknum, int *pnDatalen, uchar *psBlockdata);
int (*NDK_M1Write)(uchar ucBlocknum,  int *pnDataLen, uchar *psBlockdata);
int (*NDK_M1Increment)(uchar ucBlocknum, int nDatalen, uchar *psDatabuf);
int (*NDK_M1Decrement)(uchar ucBlocknum, int nDanalen, uchar *psDatabuf);
int (*NDK_M1Transfer)(uchar ucBlocknum);
int (*NDK_M1Restore)(uchar ucBlocknum);
int (*NDK_PiccQuickRequest)(int nModecode);
int (*NDK_SetIgnoreProtocol)(int nModecode);
int (*NDK_GetIgnoreProtocol)(int *pnModecode);
int (*NDK_GetRfidType)(int *pnRfidtype);
int (*NDK_RfidTypeARats)(uchar cid,int *pnDatalen, uchar *psDatabuf);

//alg
int (*NDK_AlgTDes)(uchar *psDataIn, uchar *psDataOut, uchar *psKey, int nKeyLen, int nMode);
int (*NDK_AlgSHA1)(uchar *psDataIn, int nInlen, uchar *psDataOut);
int (*NDK_AlgSHA256)(uchar *psDataIn, int nInlen, uchar *psDataOut);
int (*NDK_AlgSHA512)(uchar *psDataIn, int nInlen, uchar *psDataOut);
int (*NDK_AlgRSAKeyPairGen)( int nProtoKeyBit, int nPubEType, ST_RSA_PUBLIC_KEY *pstPublicKeyOut, ST_RSA_PRIVATE_KEY *pstPrivateKeyOut);
int (*NDK_AlgRSARecover)(uchar *psModule, int nModuleLen, uchar *psExp, uchar *psDataIn, uchar *psDataOut);
int (*NDK_AlgRSAKeyPairVerify)(ST_RSA_PUBLIC_KEY *pstPublicKey, ST_RSA_PRIVATE_KEY *pstPrivateKey);

//port
int (*NDK_PortOpen)(EM_PORT_NUM emPort, const char *pszAttr);
int (*NDK_PortClose)(EM_PORT_NUM emPort);
int (*NDK_PortRead)(EM_PORT_NUM emPort, unsigned int unLen, char *pszOutbuf,int nTimeoutMs, int *pnReadlen);
int (*NDK_PortWrite)(EM_PORT_NUM emPort, unsigned int  unLen,const char *pszInbuf);
int (*NDK_PortTxSendOver)(EM_PORT_NUM emPort);
int (*NDK_PortClrBuf)(EM_PORT_NUM emPort);
int (*NDK_PortReadLen)(EM_PORT_NUM emPort,int *pnReadLen);

//sec
int (*NDK_SecGetVer)(uchar * pszVerInfoOut);
int (*NDK_SecGetRandom)(int nRandLen , void *pvRandom);
int (*NDK_SecSetCfg)(unsigned int unCfgInfo);
int (*NDK_SecGetCfg)(unsigned int *punCfgInfo);
int (*NDK_SecGetKcv)(uchar ucKeyType, uchar ucKeyIdx, ST_SEC_KCV_INFO *pstKcvInfoOut);
int (*NDK_SecKeyErase)(void);
int (*NDK_SecLoadKey)(ST_SEC_KEY_INFO * pstKeyInfoIn, ST_SEC_KCV_INFO * pstKcvInfoIn);
int (*NDK_SecSetIntervaltime)(unsigned int unTPKIntervalTimeMs, unsigned int unTAKIntervalTimeMs);
int (*NDK_SecSetFunctionKey)(uchar ucType);
int (*NDK_SecGetMac)(uchar ucKeyIdx, uchar *psDataIn, int nDataInLen, uchar *psMacOut, uchar ucMod);
int (*NDK_SecGetPin)(uchar ucKeyIdx, uchar *pszExpPinLenIn,const uchar * pszDataIn, uchar *psPinBlockOut, uchar ucMode, unsigned int nTimeOutMs);
int (*NDK_SecCalcDes)(uchar ucKeyType, uchar ucKeyIdx, uchar * psDataIn, int nDataInLen, uchar *psDataOut, uchar ucMode);
int (*NDK_SecVerifyPlainPin)(uchar ucIccSlot, uchar *pszExpPinLenIn, uchar *psIccRespOut, uchar ucMode,  unsigned int unTimeoutMs);
int (*NDK_SecVerifyCipherPin)(uchar ucIccSlot, uchar *pszExpPinLenIn, ST_SEC_RSA_KEY *pstRsaPinKeyIn, uchar *psIccRespOut, uchar ucMode, unsigned int unTimeoutMs);
int (*NDK_SecLoadTIK)(uchar ucGroupIdx, uchar ucSrcKeyIdx, uchar ucKeyLen, uchar * psKeyValueIn, uchar * psKsnIn, ST_SEC_KCV_INFO * pstKcvInfoIn);
int (*NDK_SecGetDukptKsn)(uchar ucGroupIdx, uchar * psKsnOut);
int (*NDK_SecIncreaseDukptKsn)(uchar ucGroupIdx);
int (*NDK_SecGetPinDukpt)(uchar ucGroupIdx, uchar *pszExpPinLenIn, uchar * psDataIn, uchar* psKsnOut, uchar *psPinBlockOut, uchar ucMode, unsigned int unTimeoutMs);
int (*NDK_SecGetMacDukpt)(uchar ucGroupIdx, uchar *psDataIn, int nDataInLen, uchar *psMacOut, uchar *psKsnOut, uchar ucMode);
int (*NDK_SecCalcDesDukpt)(uchar ucGroupIdx, uchar ucKeyVarType, uchar *psIV, unsigned short usDataInLen, uchar *psDataIn,uchar *psDataOut,uchar *psKsnOut ,uchar ucMode);
int (*NDK_SecLoadRsaKey)(uchar ucRsaKeyIndex, ST_SEC_RSA_KEY *pstRsaKeyIn);
int (*NDK_SecRecover)(uchar ucRsaKeyIndex, const uchar *psDataIn, int nDataLen, uchar *psDataOut);
int (*NDK_SecGetPinResult)(uchar *psPinBlock, int *nStatus);
int (*NDK_SecSetKeyOwner)(char *pszName);
int (*NDK_SecGetTamperStatus)(int *pnStatus);
int (*NDK_SecGetPinResultDukpt)(uchar *psPinBlock, uchar *psKsn, int *nStatus);
int (*NDK_GetTamperStatus)();
int (*NDK_SecKeyDelete)(uchar ucKeyIdx,uchar ucKeyType);
int (*NDK_SysGoSuspend_Extern)(void);
int (*NDK_SecGetDrySR)(int *pnVal);
int (*NDK_SecClear)(void);
int (*NDK_SecVppTpInit)(uchar *num_btn, uchar *func_key, uchar *out_seq);
int (*NDK_SecUserKeyDelete)(void);
int (*NDK_SecKlaMKLDAuth)(int nLenAuthData, uchar* psAuthData, int nLenAuthCert, uchar* psAuthCert, int nLenEncCert, uchar* psEncCert, int* pnLenSsKeyCyTxt, uchar* psSsKeyCyTxt);
int (*NDK_SecKlaGenNonce)(int nLenRandom,uchar* psRandom);
//cos
int (*NDK_CosCmdRW)(unsigned int sendlen, unsigned char *sendbuf, unsigned int recvLen, unsigned char *recvbuf);
int (*NDK_CosGetMode)(unsigned char *mode);
int (*NDK_CosSetMode)(unsigned char mode);
int (*NDK_CosGetVer)(unsigned char *ver);
int (*NDK_CosReset)(void);


//ext
int (*NDK_EMV_rf_start)(emv_opt* pstEmvOption, unsigned long long transAmount);
int (*NDK_EMV_rf_suspend)(int nFinalFlag);
int (*NDK_EMV_FetchData)(unsigned int* punTagName, int nTagCnt, unsigned char* pusOutBuf, int nMaxOutLen);
int (*NDK_EMV_getdata)(unsigned int unTagName, unsigned char *pusData, int nMaxOutLen);
int (*NDK_EMV_setdata)(unsigned int unTagName, unsigned char* pusData, int nMaxLen);
int (*NDK_EMV_buildAidList)(void);
int (*NDK_EMV_ErrorCode)(void);

int (*NDK_RpcTransRW)(unsigned char* DataIn,int LenIn,unsigned char *DataOut,int *LenOut,int maxlen,int utimeout);
int (*NDK_initSdtp)();
int (*NDK_InitCom )( int (*send)(unsigned char * data, int len, int timeout),int (*recv)(unsigned char * buf, int buflen, int timeout), char * info );

int (*Ndk_beginTransactions)(int iTimeoutSec);
int (*Ndk_endTransactions)();
int (*Ndk_getStatus)();
int (*Ndk_getVKeybPin)(char* pinlen, char index, char mode, int timeout, char* account, char* KSN, char* pinblock);
int (*NDK_SYS_RegisterEvent)(EM_SYS_EVENT eventNum, int timeOutMs, int (* notifyEvent )( EM_SYS_EVENT eventNum, int msgLen, char * msg));
int (*NDK_SYS_UnRegisterEvent)(EM_SYS_EVENT eventNum);

int (*NDK_ConsoleValueGet)(int *value);
int (*SpService_nUpgradeNlpFile)(char * firmpath,char * firmtype);
int (*SpService_nRequestChannel)();
int (*SpService_nReleaseChannel)();
int  (*NDK_AuthCheckDone)(char* cpuid, char* flashid, char* customid);
int (*NAPI_SecGetKeyInfo)(EM_SEC_KEY_INFO_ID InfoID, uchar ucKeyID, EM_SEC_CRYPTO_KEY_TYPE KeyType, EM_SEC_KEY_USAGE KeyUsage,uchar *pAD, uint unADSize, uchar *psOutInfo, int *pnOutInfoLen);
int NDK_Null();


#define DLSYM(lib, foo) {	\
	foo =dlsym( lib , #foo);		\
	dlError = (char *)dlerror();					\
	if( dlError || NULL == foo ){		\
		foo = NDK_Null;	\
		rc -= 1;	\
		__android_log_print(ANDROID_LOG_INFO, TAG,"dlsym fail:  %s . "#foo"=%x ,ret will be %x\n", dlError,(unsigned int)foo,(unsigned int)NDK_Null);	\
		} \
	};\


/**
 *@brief   ��ϵͳ��̬����� POS-NDK �� API����ָ��
 *@return
 *@li   0 �����ɹ�
 *@li   -1000 - libnlposapi.so ����ʧ��
 *@li   -2000 - libnl_ndk.so ����ʧ��
 *@li    -1~-1000 ����api����ʧ�ܣ���0��ʼÿʧ��һ������ֵ��1
*/
int ndk_dlload( ){
	  int   rc;             /*  return codes            */

	  functionLib = dlopen("libnlposapi.so",RTLD_LAZY);
	   dlError = (char *)dlerror();
	   //__android_log_print(ANDROID_LOG_INFO, TAG, "1 %s %x\n", dlError,functionLib);
	   //printf("1 %s %x\n", dlError,functionLib);
	   if( functionLib == NULL ) return(-1000);

	   rc = 0;

	   //kb
	   	   DLSYM(functionLib,NDK_KbFlush);
	   	   DLSYM(functionLib,NDK_KbGetCode);
	   	   DLSYM(functionLib,NDK_KbHit);
	   	   DLSYM(functionLib,NDK_SysBeep);
	   	   DLSYM(functionLib,NDK_KbGetInput);

	   //mag
	   	   DLSYM(functionLib,NDK_MagOpen);
	   	   DLSYM(functionLib,NDK_MagClose);
	   	   DLSYM(functionLib,NDK_MagReset);
	   	   DLSYM(functionLib,NDK_MagSwiped);
	   	   DLSYM(functionLib,NDK_MagReadNormal);
	   	   DLSYM(functionLib,NDK_MagReadRaw);
	   	   DLSYM(functionLib,NDK_MagReadRawData);

	   //print
	   	   DLSYM(functionLib,NDK_PrnInit);
	   	   DLSYM(functionLib,NDK_PrnStr);
	   	   DLSYM(functionLib,NDK_PrnStart);
	   	   DLSYM(functionLib,NDK_PrnImage);
	   	   DLSYM(functionLib,NDK_PrnGetVersion);
	   	   DLSYM(functionLib,NDK_PrnSetFont);
	   	   DLSYM(functionLib,NDK_PrnGetStatus);
	   	   DLSYM(functionLib,NDK_PrnSetMode);
	   	   DLSYM(functionLib,NDK_PrnSetGreyScale);
	   	   DLSYM(functionLib,NDK_PrnSetForm);
	   	   DLSYM(functionLib,NDK_PrnFeedByPixel);
	   	   DLSYM(functionLib,NDK_PrnSetUnderLine);
	   	   DLSYM(functionLib,NDK_Script_Print);

	   //file
	   	   DLSYM(functionLib,NDK_FsOpen);
	   	   DLSYM(functionLib,NDK_FsClose);
	   	   DLSYM(functionLib,NDK_FsRead);
	   	   DLSYM(functionLib,NDK_FsWrite);
	   	   DLSYM(functionLib,NDK_FsSeek);
	   	   DLSYM(functionLib,NDK_FsDel);
	   	   DLSYM(functionLib,NDK_FsFileSize);
	   	   DLSYM(functionLib,NDK_FsExist);
	   	   DLSYM(functionLib,NDK_FsTruncate);
	   	   DLSYM(functionLib,NDK_FsTell);
	   	   DLSYM(functionLib,NDK_FsRename);
	   	   DLSYM(functionLib,NDK_FsFormat);
	   	   DLSYM(functionLib,NDK_CopyFileToSecMod);

	   //tool
	   	   DLSYM(functionLib,NDK_AddDigitStr);
	   	   DLSYM(functionLib,NDK_IncNum );
	   	   DLSYM(functionLib,NDK_FmtAmtStr );
	   	   DLSYM(functionLib,NDK_AscToHex );
	   	   DLSYM(functionLib,NDK_HexToAsc );
	   	   DLSYM(functionLib,NDK_IntToC4 );
	   	   DLSYM(functionLib,NDK_IntToC4 );
	   	   DLSYM(functionLib,NDK_C4ToInt);
	   	   DLSYM(functionLib,NDK_ByteToBcd);
	   	   DLSYM(functionLib,NDK_BcdToByte);
	   	   DLSYM(functionLib,NDK_IntToBcd);
	   	   DLSYM(functionLib,NDK_BcdToInt);
	   	   DLSYM(functionLib,NDK_CalcLRC);
	   	   DLSYM(functionLib,NDK_LeftTrim);
	   	   DLSYM(functionLib,NDK_RightTrim);
	   	   DLSYM(functionLib,NDK_AllTrim);
	   	   DLSYM(functionLib,NDK_AddSymbolToStr);
	   	   DLSYM(functionLib,NDK_SubStr);
	   	   DLSYM(functionLib,NDK_IsDigitChar);
	   	   DLSYM(functionLib,NDK_IsDigitStr);
	   	   DLSYM(functionLib,NDK_IsLeapYear);
	   	   DLSYM(functionLib,NDK_MonthDays);
	   	   DLSYM(functionLib,NDK_IsValidDate);

	   //app
	   	   DLSYM(functionLib,NDK_AppRun);
	   	   DLSYM(functionLib,NDK_AppLoad);
	   	   DLSYM(functionLib,NDK_AppDel);

	   //ic
	   	   DLSYM(functionLib,NDK_IccGetVersion);
	   	   DLSYM(functionLib,NDK_IccPowerUp );
	   	   DLSYM(functionLib,NDK_IccPowerDown);
	   	   DLSYM(functionLib,NDK_IccDetect);
	   	   DLSYM(functionLib,NDK_Iccrw);

	   //sys
	   	   DLSYM(functionLib,NDK_SysBeep);
	   	   DLSYM(functionLib,NDK_Getlibver);
	   	   DLSYM(functionLib,NDK_SysTimeBeep);
	   	   DLSYM(functionLib,NDK_SysSetPosTime);
	   	   DLSYM(functionLib,NDK_SysGetPosTime);
	   	   DLSYM(functionLib,NDK_SysStartWatch);
	   	   DLSYM(functionLib,NDK_SysStopWatch);
	   	   DLSYM(functionLib,NDK_SysDelay);
	   	   DLSYM(functionLib,NDK_SysMsDelay);
	   	   DLSYM(functionLib,NDK_SysExit);
	   	   DLSYM(functionLib,NDK_SysReboot);
	   	   DLSYM(functionLib,NDK_SysShutDown);
	   	   DLSYM(functionLib,NDK_SysSetBeepVol);
	   	   DLSYM(functionLib,NDK_SysGetBeepVol);
	   	   DLSYM(functionLib,NDK_SysSetSuspend);
	   	   DLSYM(functionLib,NDK_SysGoSuspend);
	   	   DLSYM(functionLib,NDK_SysGetPowerVol);
	   	   DLSYM(functionLib,NDK_LedStatus);
	   	   DLSYM(functionLib,NDK_SysReadWatch);
	   	   DLSYM(functionLib,NDK_SysGetPosInfo);
	   	   DLSYM(functionLib,NDK_SysGetConfigInfo);
	   	   DLSYM(functionLib,NDK_SysInitStatisticsData);
	   	   DLSYM(functionLib,NDK_SysGetStatisticsData);
	   	   DLSYM(functionLib,NDK_SysGetFirmwareInfo);
	   	   DLSYM(functionLib,NDK_SysTime);
	   	   DLSYM(functionLib,NDK_SysSetSuspendDuration);
	   	   DLSYM(functionLib,NDK_SysGetPowerVolRange);
	   	   DLSYM(functionLib,NDK_SysKeyVolSet);
	   	   DLSYM(functionLib,NDK_SysPeerOper);
	   	   DLSYM(functionLib,NDK_SysEnterBoot);
	   	   DLSYM(functionLib,NDK_SysSetPosInfo);
	   	   DLSYM(functionLib,NDk_SysGetK21Version);
	   	   DLSYM(functionLib,NDK_SysWakeUp);

	   //rf
	   	   DLSYM(functionLib,NDK_RfidVersion);
	   	   DLSYM(functionLib,NDK_RfidInit);
	   	   DLSYM(functionLib,NDK_RfidOpenRf);
	   	   DLSYM(functionLib,NDK_RfidCloseRf);
	   	   DLSYM(functionLib,NDK_RfidPiccState);
	   	   DLSYM(functionLib,NDK_RfidSuspend);
	   	   DLSYM(functionLib,NDK_RfidResume);
	   	   DLSYM(functionLib,NDK_RfidPiccType);
	   	   DLSYM(functionLib,NDK_RfidPiccDetect);
	   	   DLSYM(functionLib,NDK_RfidPiccActivate);
	   	   DLSYM(functionLib,NDK_RfidPiccDeactivate);
	   	   DLSYM(functionLib,NDK_RfidPiccApdu);
	   	   DLSYM(functionLib,NDK_M1Request);
	   	   DLSYM(functionLib,NDK_M1Anti);
	   	   DLSYM(functionLib,NDK_M1Select);
	   	   DLSYM(functionLib,NDK_M1KeyStore);
	   	   DLSYM(functionLib,NDK_M1KeyLoad);
	   	   DLSYM(functionLib,NDK_M1InternalAuthen);
	   	   DLSYM(functionLib,NDK_M1ExternalAuthen);
	   	   DLSYM(functionLib,NDK_M1Read);
	   	   DLSYM(functionLib,NDK_M1Write);
	   	   DLSYM(functionLib,NDK_M1Increment);
	   	   DLSYM(functionLib,NDK_M1Decrement);
	   	   DLSYM(functionLib,NDK_M1Transfer);
	   	   DLSYM(functionLib,NDK_M1Restore);
	   	   DLSYM(functionLib,NDK_PiccQuickRequest);
	   	   DLSYM(functionLib,NDK_SetIgnoreProtocol);
	   	   DLSYM(functionLib,NDK_GetIgnoreProtocol);
	   	   DLSYM(functionLib,NDK_GetRfidType);
	   	   DLSYM(functionLib,NDK_RfidTypeARats);

	   //alg
	   	   DLSYM(functionLib,NDK_AlgTDes);
	   	   DLSYM(functionLib,NDK_AlgSHA1);
	   	   DLSYM(functionLib,NDK_AlgSHA256);
	   	   DLSYM(functionLib,NDK_AlgSHA512);
	   	   DLSYM(functionLib,NDK_AlgRSAKeyPairGen);
	   	   DLSYM(functionLib,NDK_AlgRSARecover);
	   	   DLSYM(functionLib,NDK_AlgRSAKeyPairVerify);

	   //port
	   	   DLSYM(functionLib,NDK_PortOpen);
	   	   DLSYM(functionLib,NDK_PortClose);
	   	   DLSYM(functionLib,NDK_PortRead);
	   	   DLSYM(functionLib,NDK_PortWrite);
	   	   DLSYM(functionLib,NDK_PortTxSendOver);
	   	   DLSYM(functionLib,NDK_PortClrBuf);
	   	   DLSYM(functionLib,NDK_PortReadLen);
	DLSYM(functionLib,NAPI_SecGetKeyInfo);

	   //sec
	   	   DLSYM(functionLib,NDK_SecGetVer);
	   	   DLSYM(functionLib,NDK_SecGetRandom);
	   	   DLSYM(functionLib,NDK_SecSetCfg);
	   	   DLSYM(functionLib,NDK_SecGetCfg);
	   	   DLSYM(functionLib,NDK_SecGetKcv);
	   	   DLSYM(functionLib,NDK_SecKeyErase);
	   	   DLSYM(functionLib,NDK_SecLoadKey);
	   	   DLSYM(functionLib,NDK_SecSetIntervaltime);
	   	   DLSYM(functionLib,NDK_SecSetFunctionKey);
	   	   DLSYM(functionLib,NDK_SecGetMac);
	   	   DLSYM(functionLib,NDK_SecGetPin);
	   	   DLSYM(functionLib,NDK_SecCalcDes);
	   	   DLSYM(functionLib,NDK_SecVerifyPlainPin);
	   	   DLSYM(functionLib,NDK_SecVerifyCipherPin);
	   	   DLSYM(functionLib,NDK_SecLoadTIK);
	   	   DLSYM(functionLib,NDK_SecGetDukptKsn);
	   	   DLSYM(functionLib,NDK_SecIncreaseDukptKsn);
	   	   DLSYM(functionLib,NDK_SecGetPinDukpt);
	   	   DLSYM(functionLib,NDK_SecGetMacDukpt);
	   	   DLSYM(functionLib,NDK_SecCalcDesDukpt);
	   	   DLSYM(functionLib,NDK_SecLoadRsaKey);
	   	   DLSYM(functionLib,NDK_SecRecover);
	   	   DLSYM(functionLib,NDK_SecGetPinResult);
	   	   DLSYM(functionLib,NDK_SecSetKeyOwner);
	   	   DLSYM(functionLib,NDK_SecGetTamperStatus);
	   	   DLSYM(functionLib,NDK_SecGetPinResultDukpt);
	   	   DLSYM(functionLib,NDK_GetTamperStatus);
	   	   DLSYM(functionLib,NDK_SecKeyDelete);
	   	   DLSYM(functionLib,NDK_SysGoSuspend_Extern);
	   	   DLSYM(functionLib,NDK_SecGetDrySR);
	   	   DLSYM(functionLib,NDK_SecClear);
	   	   DLSYM(functionLib,NDK_SecVppTpInit);
	   	   DLSYM(functionLib,NDK_SecUserKeyDelete);
	   	   DLSYM(functionLib,NDK_SecKlaMKLDAuth);
	   	   DLSYM(functionLib,NDK_SecKlaGenNonce);

	DLSYM(functionLib,NDK_ConsoleValueGet);
	   //cos
	   	   DLSYM(functionLib,NDK_CosCmdRW);
	   	   DLSYM(functionLib,NDK_CosGetMode);
	   	   DLSYM(functionLib,NDK_CosSetMode);
	   	   DLSYM(functionLib,NDK_CosGetVer);
	   	   DLSYM(functionLib,NDK_CosReset);

	   //ext
	   	   DLSYM(functionLib,NDK_EMV_rf_start);
	   	   DLSYM(functionLib,NDK_EMV_rf_suspend);
	   	   DLSYM(functionLib,NDK_EMV_FetchData);
	   	   DLSYM(functionLib,NDK_EMV_getdata);
	   	   DLSYM(functionLib,NDK_EMV_setdata);
	   	   DLSYM(functionLib,NDK_EMV_buildAidList);
	   	   DLSYM(functionLib,NDK_EMV_ErrorCode);

			DLSYM(functionLib, NDK_AuthCheckDone);

	   // android
	   functionLib = dlopen("libnl_ndk.so",RTLD_LAZY);
	   dlError = (char *)dlerror();
	 	//__android_log_print(ANDROID_LOG_INFO, TAG, "2 %s %x\n", dlError,functionLib);
	 	printf("2 %s %x\n", dlError,functionLib);
	 	if( functionLib == NULL ) return(-2000);

	   	   DLSYM(functionLib,Ndk_beginTransactions);
	   	   DLSYM(functionLib,Ndk_endTransactions);
	   	   DLSYM(functionLib,Ndk_getStatus);
	   	   DLSYM(functionLib,Ndk_getVKeybPin);
	   	   DLSYM(functionLib,NDK_SYS_RegisterEvent);
	   	   DLSYM(functionLib,NDK_SYS_UnRegisterEvent);


	// N910-A7
	functionLib = dlopen("libISPServiceDelegation.so",RTLD_LAZY);
	dlError = (char *)dlerror();
	//__android_log_print(ANDROID_LOG_INFO, TAG, "2 %s %x\n", dlError,functionLib);
	printf("2 %s %x\n", dlError,functionLib);
	if( functionLib == NULL ) return(-2000);

	DLSYM(functionLib, SpService_nUpgradeNlpFile);
	DLSYM(functionLib, SpService_nRequestChannel);
	DLSYM(functionLib, SpService_nReleaseChannel);


	// NDK_PrnGetVersion = (int (*)(char *pszVer)) dlsym( functionLib, "NDK_PrnGetVersion");
	   //char prnVer[100];
	  // memset(prnVer,0,100);
	   //__android_log_print(ANDROID_LOG_INFO, TAG, "NDK_PrnGetVersion = %x ; %s \n", NDK_PrnGetVersion(prnVer),prnVer);

	   	__android_log_print(ANDROID_LOG_INFO, TAG, "ndk_dlload = %d",rc);
	   return rc;

}


int NDK_Null()
{
    //__android_log_print(ANDROID_LOG_INFO, TAG, "into NDK_Null %x", NDK_Null);
	return -10000;
}

