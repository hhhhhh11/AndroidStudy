/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#define JAVA_CLASS_NAME 	"com/newland/downfrompc/DownloadFromPC"
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <jni.h>
#include "sdtp.h"
#include <android/log.h>
#include "ndk.h"

static JavaVM *sVm;

#define LOGI(...) __android_log_print(ANDROID_LOG_VERBOSE,"jni_download_app_firm",__VA_ARGS__)

/* This is a trivial JNI example where we use a native method
 * to return a new VM String. See the corresponding Java source
 * file located at:
 *
 *   apps/samples/hello-jni/project/src/com/example/hellojni/HelloJni.java
 */
jint
Java_com_newland_downfrompc_DownloadFromPC_downFile( JNIEnv* env,
                                                  jobject thiz, jbyteArray posInfo, jint len, jbyteArray dir, jbyteArray appInfo, jint appLen)
{
	char *pposInfo = NULL;
	char *pdir = NULL;
	char *pappInfo = NULL;

	if(posInfo != NULL){
		pposInfo = (char *)((*env)->GetByteArrayElements(env, posInfo, NULL));
	}
	if(appInfo != NULL){
		pappInfo = (char *)((*env)->GetByteArrayElements(env, appInfo, NULL));
	}
	if(dir != NULL){
		pdir = (char *)((*env)->GetByteArrayElements(env, dir, NULL));
	}

	char timeBuf[8];


	int ret = sdtp_updateFile(pposInfo, len, pdir ,pappInfo, appLen,timeBuf);
	LOGI("ret = %d\n", ret);

	if(ret == 5 || ret == 7){
        time_t lTime = 0;
        // PC同步的时间
        memcpy(&lTime,timeBuf,4);
		struct tm *tm_t;


		// 从PC传过来的时间，该时间表示的是当前时区的时间戳，获取tm结构体时就应该用gmtime进行转换，
		// 如果用localtime获取tm 结构体，会导致时间不对
		tm_t = gmtime(&lTime);


		// mktime函数通过tm_t结构体中的时间获取时间戳，
		// 它将tm_t中的时间当做local时间（无论tm_t结构体中的时区如何设置），
		// 然后获取时间戳，最后会将tm_t时区置为当前时区
        time_t  time0 = mktime(tm_t);


		jclass  jSystemClock = (*env)->FindClass(env, "android/os/SystemClock");

		if(jSystemClock == NULL){
            LOGI("===========1111  set System time, get SystemClock Class erro ");
            return ret;
        }
        jmethodID jsetCurrentTimeMillis = (*env)->GetStaticMethodID(env, jSystemClock, "setCurrentTimeMillis", "(J)Z");

		if(jsetCurrentTimeMillis == NULL){
			LOGI("===========2222   set System time, getsetCurrentTimeMillis  method erro ");
			return ret;
		}

		 // 通过反射，调用SystemClock.setCurrentTimeMillis(long time),设置系统时间
        jboolean isOk =  (*env)->CallStaticBooleanMethod(env,jSystemClock,jsetCurrentTimeMillis,time0 * 1000);

		// 设置K21时间
        int retval=NDK_SysSetPosTime(*tm_t);

		if(isOk){
			LOGI("===========333set SystemTime true " );
		} else{
            LOGI("===========333set SystemTime false " );
		}
	}



    return ret;
}

jint
Java_com_newland_downfrompc_DownloadFromPC_endDownFile( JNIEnv* env, jobject thiz){
	sdtp_endUpdateFile();
}

jbyteArray Java_com_newland_downfrompc_GetUninstallAppInfoFromPc_getAppInfo(
		JNIEnv* env, jobject thiz, jbyteArray appInfo, jint len )
{
	char *pappInfo = NULL;

	if (appInfo != NULL)
	{
		pappInfo = (char *)((*env)->GetByteArrayElements(env, appInfo, NULL));
	}

	char uninstallAppInfo[512] = {0};
	int outLen = 0;
	int ret = 0;
	do
	{
		ret = sdtp_getAppInfo(pappInfo, len, &uninstallAppInfo[1], &outLen);
	} while (ret == 1);

	uninstallAppInfo[0] = (char)ret;
	jbyteArray retArray = (*env)->NewByteArray(env, outLen + 1);
	(*env)->SetByteArrayRegion(env, retArray, 0, outLen + 1, uninstallAppInfo);
    return retArray;
}

jint Java_com_newland_downfrompc_GetUninstallAppInfoFromPc_endUninstall( JNIEnv* env, jobject thiz )
{
    sdtp_endUninstallApp();
}

jint Java_com_newland_downfrompc_DownloadFromPC_jniGetSysPosInfo(JNIEnv* env, jobject obj,int type ,jintArray len ,jbyteArray buf){

	int nRet;
	uint *lenvalue = NULL;
    uchar *KLA_FrameBuf = NULL;
	if (len != NULL){
		lenvalue =  (uint *)((*env)->GetIntArrayElements(env,len,0));
	}
	if (buf != NULL){
		KLA_FrameBuf = (uchar *) (*env)->GetByteArrayElements(env, buf,NULL);
	}
	nRet = NDK_SysGetPosInfo(type, lenvalue ,KLA_FrameBuf);
	if(nRet != 0)
	{
		return -1;
	}
	if (lenvalue != NULL){
		(*env)->ReleaseIntArrayElements(env, len, (jint *) lenvalue,0);
	}

	if (KLA_FrameBuf != NULL){
		(*env)->ReleaseByteArrayElements(env, buf, (jbyte *) KLA_FrameBuf ,0);
	}

	return nRet;
}

jint Java_com_newland_downfrompc_DownloadFromPC_jniGetK21Version(JNIEnv* env, jobject obj,jbyteArray buf){

    int flag;
    uchar *psSwiped = NULL;
    if (buf != NULL)
        psSwiped = (uchar *) (*env)->GetByteArrayElements(env, buf, NULL);
    flag = NDk_SysGetK21Version(psSwiped);
    if (psSwiped != NULL)
        (*env)->ReleaseByteArrayElements(env, buf, (jbyte *) psSwiped, 0);
    return flag;
}

jint Java_com_newland_downfrompc_DownloadFromPC_jniGetSnk(JNIEnv* env, jobject obj,jbyteArray buf){

    ST_SEC_KCV_INFO kcvst;
    memset(&kcvst, 0x00, sizeof(ST_SEC_KCV_INFO));
    int nRet;
     NDK_SecSetKeyOwner("_NL_TERM_MGR");
    kcvst.nCheckMode = SEC_KCV_ZERO;
    nRet= NDK_SecGetKcv(SEC_KEY_TYPE_TDK | SEC_KEY_SM4, 255, &kcvst);//此函数返回成功表示SNK已下载，否则未下载。
    LOGI("nCheckMode:%d",kcvst.nCheckMode);
    LOGI("Pos_SetSnkSucc:%d",nRet);
    NDK_SecSetKeyOwner("*");
    return nRet;
}


jint Java_com_newland_downfrompc_DownloadFromPC_jniGetCSN(JNIEnv* env, jobject obj, jstring fileName,
		jbyteArray buffer, uint unOffset, uint unLength){
	const unsigned char* pszName;
		uchar * psBuffer;
		int fd = -1;
		int nRet = -1;
		if (fileName != NULL)
		//	pszName = (uchar *) (*env)->GetByteArrayElements(env, fileName, NULL);
			pszName = (*env)->GetStringUTFChars(env, fileName, 0);
		if (buffer != NULL)
			psBuffer = (uchar *) (*env)->GetByteArrayElements(env, buffer, NULL);
		memset(psBuffer, 0x00, unLength);
		nRet = NDK_FsExist(pszName);
		if (nRet != 0) {
			LOGI("##########file not exist err\n'");
			return nRet;
		}
		fd = NDK_FsOpen(pszName, "r");
		LOGI("##########NDK_FsOpen:%d", fd);
		if (fd < 0) {
			LOGI("file not exist err\n");
			return -1;
		}
		nRet = NDK_FsSeek(fd, unOffset, SEEK_SET);
		nRet = NDK_FsRead(fd, (char *) psBuffer, unLength);
		if (nRet >= 0) {
			psBuffer[nRet] = 0;
		} else {
			return -3;
		}
		NDK_FsClose(fd);
	/*
		if (pszName != NULL)
			(*env)->ReleaseByteArrayElements(env, fileName, (jbyte *) pszName,
					JNI_ABORT); // C: (*env)->   (env,
	*/

		if (psBuffer != NULL)
			(*env)->ReleaseByteArrayElements(env, buffer, (jbyte *) psBuffer, 0); // C: (*env)->   (env,
		return nRet;
}


jint Java_com_newland_downfrompc_DownloadFromPC_jniSecGetKcvValue(JNIEnv  *env,jobject obj,jint type,jint index,jbyteArray checkBuf,jintArray kcvLen) {
	jint ret = 0;
	uchar *pCheckBuf = NULL;
	int *pkcvLen = NULL;
	if (checkBuf != NULL){
		pCheckBuf =  (*env)->GetByteArrayElements(env, checkBuf, NULL);
	}

	if(kcvLen != NULL){
		pkcvLen= (*env)->GetIntArrayElements(env, kcvLen, NULL);
	}

	ST_SEC_KCV_INFO stKcvInfoIn;
	stKcvInfoIn.nCheckMode = SEC_KCV_ZERO;
	ret = NDK_SecGetKcv(type, index, &stKcvInfoIn);
	// pCheckBuf = stKcvInfoIn.sCheckBuf;
	LOGI("##########jniSecGetKcvValue--type=%d index=%d , ret=%d , pkcvLen=%d", type,index,ret,stKcvInfoIn.nLen);
	if (ret == 0){
		memcpy(pCheckBuf,stKcvInfoIn.sCheckBuf,stKcvInfoIn.nLen);
		*pkcvLen = stKcvInfoIn.nLen;
	}

	if (pCheckBuf != NULL){
		(*env)->ReleaseByteArrayElements(env, checkBuf, (jbyte *)pCheckBuf,0);
	}
	if (pkcvLen != NULL){
		(*env)->ReleaseIntArrayElements(env, kcvLen,pkcvLen,0);
	}

	return ret;
}


jint Java_com_newland_downfrompc_DownloadFromPC_jniSecGetKcv(JNIEnv  *env,jobject obj,jint type,jint index,jint isShareSection,jstring packageName) {
	jint ret = 0;
	const unsigned char* pszName;
	if (packageName != NULL){
        pszName = (*env)->GetStringUTFChars(env, packageName, 0);
	}

//    if(!isShareSection){
//        LOGI("##########jniSecGetKcv:%d", isShareSection);
//        NDK_SecSetKeyOwner(pszName);
//    }
    ST_SEC_KCV_INFO stKcvInfoIn;
    stKcvInfoIn.nCheckMode = SEC_KCV_ZERO;
    ret = NDK_SecGetKcv(type, index, &stKcvInfoIn);
    LOGI("##########jniSecGetKcv-type=%d index=%d , ret=%d", type,index,ret);
//     if(!isShareSection){
//
//      }
    if (pszName != NULL){
        (*env)->ReleaseStringUTFChars(env, packageName, pszName);
    }

	return ret;
}

jint Java_com_newland_downfrompc_DownloadFromPC_jniSetKeyOwner(JNIEnv  *env,jobject obj,jbyteArray packageName) {
    jint ret = 0;
    char* pszName;
    if (packageName != NULL){
		pszName = (char *)((*env)->GetByteArrayElements(env, packageName, NULL));
    }
    NDK_SecSetKeyOwner(pszName);
    if (pszName != NULL){
		(*env)->ReleaseByteArrayElements(env, packageName, (jbyte *) pszName, 0);
	}

    return ret;
}


jint Java_com_newland_downfrompc_DownloadFromPC_jniSecGetDukpt(JNIEnv* env, jobject obj,jint type,jint index){
	jint ret = 0;
	if (type == -1) {
		char ksn[16];
		ret = NDK_SecGetDukptKsn(index,&ksn);
	} else {
		ST_SEC_KCV_INFO stKcvInfoIn;
		stKcvInfoIn.nCheckMode = SEC_KCV_ZERO;
		ret = NDK_SecGetKcv(type, index, &stKcvInfoIn);
	}
	return ret;
}
jint Java_com_newland_downfrompc_DownloadFromPC_jniReadAdb(JNIEnv* env, jobject obj,jint type){
	int ret = -1;
	if (type){
		int adb = -1;
		ret = NDK_ConsoleValueGet(&adb);
		LOGI("ndkReadAdb jniReadAdb: ret= %d，adb=%d",ret,adb );
		if (ret == 0){
			ret = adb;
		} else{
			LOGI("NDK_ConsoleValueGet: read failret= %d",ret);
		}
	} else{
		FILE *fp = NULL;
		char buff[10];

		fp = fopen("/newland/system/adb", "r");
		if(fp<0){
			LOGI("jniReadAdb: open fail");
			return -1;
		}

//    printf("123456: %s\n", buff );
		fscanf(fp, "%s", buff);
		fclose(fp);
		if (strchr(buff,'1') != NULL){
			ret = 1;
		} else if (strchr(buff,'0') != NULL){
			LOGI("jniReadAdb: close %s",buff);
			ret = 0;
		} else if (strchr(buff,'2') != NULL){
			ret = 2;
		} else if (strchr(buff,'3') != NULL){
			ret = 3;
		} else{
			ret = -1;
		}
		LOGI("jniReadAdb: open %s",buff );
	}
	return ret;
}




/**
*@fn        AscToHex(const unsigned char*, int, char , int, unsigned char *)
*@brief     DES3计算,根据密钥类型区分计算为加密还是解密
*@param     pszAsciiBuf ASCII输入缓冲
*@param     nLen ASCII长度
*@param     cType 转换类型
*@param     pszBcdBuf 输出BCD码缓冲
*@return    @li 0               成功
            @li -1              错误
*@section   history     修改历史
                \<author\>  \<time\>    \<desc\>
*/
int AscToHex(const unsigned char* pszAsciiBuf, int nLen, char cType, unsigned char* pszBcdBuf)
{
    int i = 0;
    char cTmp, cTmp1;

    if (pszAsciiBuf == NULL) {
        return -1;
    }

    if ((nLen&0x01) && cType) { /*判别是否为奇数以及往那边对齐*/
        cTmp1 = 0 ;
    } else {
        cTmp1 = 0x55 ;
    }

    for (i = 0; i < nLen; pszAsciiBuf ++, i ++) {
        if ( *pszAsciiBuf >= 'a' ) {
            cTmp = *pszAsciiBuf - 'a' + 10 ;
        } else if ( *pszAsciiBuf >= 'A' ) {
            cTmp = *pszAsciiBuf - 'A' + 10 ;
        } else if ( *pszAsciiBuf >= '0' ) {
            cTmp = *pszAsciiBuf - '0' ;
        } else {
            cTmp = *pszAsciiBuf;
            cTmp&=0x0f;
        }

        if ( cTmp1 == 0x55 ) {
            cTmp1 = cTmp;
        } else {
            *pszBcdBuf ++ = cTmp1 << 4 | cTmp;
            cTmp1 = 0x55;
        }
    }
    if (cTmp1 != 0x55) {
        *pszBcdBuf = cTmp1 << 4;
    }

    return 0;
}

/**
*@fn        HexToAsc(const unsigned char*, int, char , int, unsigned char *)
*@brief     DES3计算,根据密钥类型区分计算为加密还是解密
*@param     pszBcdBuf 输入BCD码缓冲
*@param     nLen ASCII长度
*@param     cType 转换类型
*@param     pszAsciiBuf ASCII输出缓冲
*@return    @li 0               成功
            @li -1              错误
*@section   history     修改历史
                \<author\>  \<time\>    \<desc\>
*/
int HexToAsc(const unsigned char* pszBcdBuf, int nLen, char cType, unsigned char* pszAsciiBuf)
{
    int i = 0;

    if (pszBcdBuf == NULL) {
        return -1;
    }
    if (nLen & 0x01 && cType) { /*判别是否为奇数以及往那边对齐*/
        /*0左，1右*/
        i = 1;
        nLen ++;
    } else {
        i = 0;
    }
    for (; i < nLen; i ++, pszAsciiBuf ++) {
        if (i & 0x01) {
            *pszAsciiBuf = *pszBcdBuf ++ & 0x0f;
        } else {
            *pszAsciiBuf = *pszBcdBuf >> 4;
        }
        if (*pszAsciiBuf > 9) {
            *pszAsciiBuf += 'A' - 10;
        } else {
            *pszAsciiBuf += '0';
        }

    }
    *pszAsciiBuf = 0;
    return 0;
}




//void newlandNDK_printf_string(char *BUF, int LEN) {
//	int i;
//	int len;
//	int size;
//	int temp;
//	int offset;
//	char s[2048];
//	size = LEN;
//	for (i = 0; i < LEN;) {
//		offset = 0;
//		memset(s, 0, sizeof(s));
//		len = (size > 256) ? 256 : size;
//		for (temp = 0; temp < len; temp++) {
//			offset += sprintf(s + offset, "%02x ", BUF[temp + i]);\
//		}
//		i += len;
//		size -= len;
//		s[offset - 1] = '\n';
//		LOGI("newlandNDK=%s", s);
//	}
//}

jint Java_com_newland_downfrompc_DownloadFromPC_PosNapiSecGetKcvValue(JNIEnv  *env,jobject obj,jint ucKeyIdx,jint keyInfo,jint keyType,jint keyUsage,jbyteArray checkBuf,jbyteArray kcvLen) {
	jint ret = 0;
	uchar *pCheckBuf = NULL;
	uchar *pkcvLen = NULL;
	if (checkBuf != NULL){
		pCheckBuf =  (*env)->GetByteArrayElements(env, checkBuf, NULL);
	}

	if(kcvLen != NULL){
		pkcvLen=  (*env)->GetByteArrayElements(env, kcvLen, NULL);
	}
	ret = NAPI_SecGetKeyInfo(keyInfo, ucKeyIdx, keyType, keyUsage, pCheckBuf, 0, pCheckBuf,
							 (int *) pkcvLen);
	LOGI("##########jniSecGetKcvValue--type=%d index=%d , ret=%d", KEY_TYPE_DES,ucKeyIdx,ret);

	if (pCheckBuf != NULL){
		(*env)->ReleaseByteArrayElements(env, checkBuf, pCheckBuf,0);
	}
	if (pkcvLen != NULL){
		(*env)->ReleaseByteArrayElements(env, kcvLen, pkcvLen,0);
	}

	return ret;
}

jint Java_com_newland_downfrompc_DownloadFromPC_updateNlpFile(JNIEnv* env, jobject obj,jbyteArray firmpath, jbyteArray firmtype){
	int nRet = -1;
	//  char firmpath[100] = {0};
	//  char firmtype[100] = {0};
	char* pfPath;
	char* pfType;
	if (firmpath != NULL){
		pfPath = (char *)((*env)->GetByteArrayElements(env, firmpath, NULL));
	}
	if (firmtype != NULL){
		pfType = (char *)((*env)->GetByteArrayElements(env, firmtype, NULL));
	}
	char cpuid[256] = {0};
	unsigned char cpuidSha256[256] = {0};
	unsigned char cpuidSha256Ascii[256] = {0};
	char flashid[256] = {0};
	char customid[256] = {0};
	char customidHex[256] = {0};



	//  memcpy(firmpath,"/data/master_N910D_V3.3.03.12.D.NLP",strlen("/data/master_N910D_V3.3.03.12.D.NLP"));
	// memcpy(firmtype,"master",strlen("master"));


	newlandNDK_printf_string(pfType,strlen(pfType));
	nRet = SpService_nRequestChannel();
	nRet = SpService_nUpgradeNlpFile(pfPath, pfType);
	nRet = SpService_nReleaseChannel();
	// property_get("ro.boot.cpuid",cpuid,"unknown");
	// property_get("sys.pos_flashid",flashid,"unknown");
	// property_get("sys.pos_customid",customid,"unknown");


	__system_property_get("ro.boot.cpuid",cpuid);
	__system_property_get("sys.pos_flashid",flashid);
	__system_property_get("sys.pos_customid",customid);

	usleep(8000);
	if(!strncmp(cpuid,"unknown",7)){
        LOGI("get cpuid failed");
        nRet = -2;
        return nRet;
    }



	if( strlen(cpuid) > 32 ){
        nRet = NDK_AlgSHA256((unsigned char*)cpuid, strlen(cpuid), (unsigned char*)cpuidSha256);
        if(nRet != 0){
            LOGI("AlgSHA256 cpuid fail, ndk ret:%d",nRet);
            nRet = -3;
            return nRet;
        }
        HexToAsc((unsigned char*)cpuidSha256, 64, 0, (unsigned char*)cpuidSha256Ascii);
        memset(cpuid, 0, 256 );
        memcpy(cpuid, cpuidSha256Ascii, 64);
	}



	if(strncmp(customid, "B23A6A8439C0DDE5515893E7C90C1E32", 32)) {
		AscToHex((unsigned char*)customid, 64, 0, (unsigned char*)customidHex);
		memset(customid, 0, 256);
		memcpy(customid, customidHex, 32);
	}



	nRet = NDK_AuthCheckDone(cpuid, flashid, customid);
	if(nRet<0){
		return nRet;
	}
	return nRet;

}



jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = JNI_ERR;
    sVm = vm;

#ifdef __cplusplus

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
#else
    if ((*vm)->GetEnv(vm,(void**)&env, JNI_VERSION_1_4) != JNI_OK){
#endif
        return result;
    }

    if(result < 0){

    }
    result = ndk_dlload();
    result = JNI_VERSION_1_4;

    end:
    return result;
}
