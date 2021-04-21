
#define JAVA_CLASS_NAME 	"com/updatedemo/util/NdkApi"
#define LOG_TAG "NDKTest"

#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <android/log.h>

#include <dlfcn.h>
#include <jni.h>

#include "/include/NDK.h"
#include "include/NDK.h"

#define DLSYM(lib, foo) {	\
		foo =dlsym( lib , #foo);		\
		dlError = (char *)dlerror();					\
		__android_log_print(ANDROID_LOG_INFO, TAG,"DLSYM "#foo" , = %x !",foo);	\
		if( dlError || NULL == foo ){		\
			foo = NDK_Null;	\
			rc -= 1;	\
			__android_log_print(ANDROID_LOG_INFO, TAG, "dlsym fail:  %s . "#foo"=%x ,ret will be %x\n", dlError,(int)foo,NDK_Null());	\
		} \
};\


#define _DEBUG 1
#ifdef _DEBUG

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型


#else
#define LOGI(...) 	;
#endif

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

JNIEnv* getJNIEnv();

//void  *functionLib;         /*  Handle to shared lib file   */
//char *dlError;        /*  Pointer to error string     */
struct tm g_stTm;

int jniRegisterNativeMethods(JNIEnv* env,const char* className,const JNINativeMethod* gMethods,int numMethods);

static JavaVM *sVm;
#define TAG LOG_TAG
/**
 * 获取kla版本
 */
//jint Pos_sayHello(JNIEnv* env, jobject obj,int type ,jbyteArray buf){
//
//	uchar *KLA_FrameBuf = NULL;
//
//	/*****************初始化***************/
////	DispInitConfig();
//
//
//	if (buf != NULL){
//		KLA_FrameBuf = (uchar *) (*env)->GetByteArrayElements(env, buf,NULL);
//	}
//	//NDK接口调用，设置机器sn号
////    NDK_SysSetPosInfo(SYS_HWINFO_GET_POS_USN,"R1NL00000002");
//    sprintf(KLA_FrameBuf,"jni to java say %s","hello");
//	if (KLA_FrameBuf != NULL){
//		(*env)->ReleaseByteArrayElements(env, buf, (jbyte *) KLA_FrameBuf ,0);
//	}
//	return (jint)0;
//}

//int (*NDK_SysGetConfigInfo)(EM_SYS_CONFIG emConfig,int *pnValue);
jint POS_NDK_SysGetConfigInfo(JNIEnv* env,jobject obj,jint jConfig,jintArray jValue){
    int *pnValue = NULL;
    if (jValue != NULL){
        pnValue = (int *)(*env)->GetIntArrayElements(env,jValue,NULL);
    }
    LOGD("##########Mdebug pnValue %d ",pnValue);

    int nRet = NDK_SysGetConfigInfo(jConfig,pnValue);
    LOGD("##########Mdebug nRet %d ",nRet);

    if(pnValue != NULL){
        (*env)->ReleaseIntArrayElements(env,jValue,(jint *)pnValue,0);
    }

    return (jint)nRet;
}

//int (*NDK_SysGetPosInfo)(EM_SYS_HWINFO emFlag,uint *punLen,char *psBuf);
jint POS_NDK_SysGetPosInfo(JNIEnv* env,jobject obj,jint jFlag,jintArray jLen,jbyteArray jBuf){
    uint *punLen = NULL;
    uchar *psBuf = NULL;
    if (jLen != NULL){
        punLen = (uint *)(*env)->GetIntArrayElements(env,jLen,0);
    }
    LOGD("##########Mdebug punLen %d ",punLen);
    if (jBuf != NULL){
        psBuf = (uchar *)(*env)->GetByteArrayElements(env,jBuf,NULL);
    }
    LOGD("##########Mdebug psBuf %s ",psBuf);

    int nRet = NDK_SysGetPosInfo(jFlag,punLen,psBuf);
    LOGD("##########Mdebug nRet %d ",nRet);

    if (punLen != NULL){
        (*env)->ReleaseIntArrayElements(env,jLen,(jint *)punLen,0);
    }
    if (psBuf != NULL){
        (*env)->ReleaseByteArrayElements(env,jBuf,(jbyte *)psBuf,0);
    }

    return (jint)nRet;
}

//int (*NDK_AscToHex )(const uchar* pszAsciiBuf, int nLen, uchar ucType, uchar* psBcdBuf);
jint POS_NDK_AscToHex(JNIEnv* env,jobject obj,jbyteArray jAsciiBuf,jint jLen,jbyte jType,jbyteArray jBcdBuf){
    char *pszAsciiBuf = NULL;
    char *psBcdBuf = NULL;
    if (jAsciiBuf != NULL){
        pszAsciiBuf = (char *)(*env)->GetByteArrayElements(env,jAsciiBuf,NULL);
    }
    LOGD("##########Mdebug pszAsciiBuf %s ",pszAsciiBuf);
    if (jBcdBuf != NULL){
        psBcdBuf = (char *)(*env)->GetByteArrayElements(env,jBcdBuf,NULL);
    }
    LOGD("##########Mdebug psBcdBuf %s ",psBcdBuf);
    int nRet = NDK_AscToHex(pszAsciiBuf,jLen,jType,psBcdBuf);
    LOGD("##########Mdebug nRet %d ",nRet);
    if (pszAsciiBuf != NULL){
        (*env)->ReleaseByteArrayElements(env,jAsciiBuf,(jchar *)pszAsciiBuf,0);
    }
    if (psBcdBuf != NULL){
        (*env)->ReleaseByteArrayElements(env,jBcdBuf,(jchar *)psBcdBuf,0);
    }
    return nRet;
}


//int (*NDK_HexToAsc )(const uchar* psBcdBuf, int nLen, uchar ucType, uchar* pszAsciiBuf);
jint POS_NDK_HexToAsc(JNIEnv* env,jobject obj,jbyteArray jBcdBuf,jint jLen,jbyte jType,jbyteArray jAsciiBuf){
    char *psBcdBuf = NULL;
    char *pszAsciiBuf = NULL;
    if (jBcdBuf != NULL){
        psBcdBuf = (char *)(*env)->GetByteArrayElements(env,jBcdBuf,NULL);
    }
    LOGD("##########Mdebug psBcdBuf %s ",psBcdBuf);
    if (jAsciiBuf != NULL){
        pszAsciiBuf = (char *)(*env)->GetByteArrayElements(env,jAsciiBuf,NULL);
    }
    LOGD("##########Mdebug pszAsciiBuf %s ",pszAsciiBuf);

    int nRet = NDK_HexToAsc(psBcdBuf,jLen,jType,pszAsciiBuf);
    LOGD("##########Mdebug nRet %d ",nRet);
    if (pszAsciiBuf != NULL){
        (*env)->ReleaseByteArrayElements(env,jBcdBuf,(jchar *)psBcdBuf,0);
    }
    if (psBcdBuf != NULL){
        (*env)->ReleaseByteArrayElements(env,jAsciiBuf,(jchar *)pszAsciiBuf,0);
    }
    return nRet;

}

//静态注册java方法，包名+类名
jint
Java_com_newland_nllearn_NdkApi_sayByStatic( JNIEnv* env,jobject thiz, jbyteArray pucPwd)
{
    uchar *ppucPwd = NULL;
    int nRet = -1;
    if(pucPwd != NULL){
        ppucPwd = (uchar *)(*env)->GetByteArrayElements(env, pucPwd, NULL);
    }
    LOGI("12345");
    sprintf(ppucPwd,"static to java say %s","hello");
    if (ppucPwd != NULL){
        (*env)->ReleaseByteArrayElements(env, pucPwd, (jbyte *) ppucPwd,0);
    }
    return 0;
}

void newlandNDK_printf_string(char *BUF, int LEN) {
	int i;
	int len;
	int size;
	int temp;
	int offset;
	char s[2048];
	size = LEN;
	for (i = 0; i < LEN;) {
		offset = 0;
		memset(s, 0, sizeof(s));
		len = (size > 256) ? 256 : size;
		for (temp = 0; temp < len; temp++) {
			offset += sprintf(s + offset, "%02x ", BUF[temp + i]);\
		}
		i += len;
		size -= len;
		s[offset - 1] = '\n';
		LOGI("newlandNDK=%s", s);
	}
}



/*动态注册java方法 */
static const JNINativeMethod method_table[] = {

		{"JNI_NDK_SysGetConfigInfo","(I[I)I",(void *)POS_NDK_SysGetConfigInfo},
		{"JNI_NDK_SysGetPosInfo","(I[I[B)I",(void *)POS_NDK_SysGetPosInfo},
		{"JNI_NDK_AscToHex","([BIB[B)I",(void *)POS_NDK_AscToHex},
		{"JNI_NDK_HexToAsc","([BIB[B)I",(void *)POS_NDK_HexToAsc},
};


/* Register JNI methods */

        /* >>>>>>>>>>>>>>>>>>>>>>> */


    /*
      * Register native JNI-callable methods.
      *
      * "className" looks like "java/lang/String".
      */

    int jniRegisterNativeMethods(JNIEnv* env,
                                  const char* className,
                                  const JNINativeMethod* gMethods,
                                  int numMethods)
     {
         jclass clazz;

         __android_log_print(ANDROID_LOG_INFO, TAG, "Registering %s natives\n", className);
#ifdef __cplusplus
     clazz = env->FindClass(className);
     if (clazz == NULL) {
             __android_log_print(ANDROID_LOG_ERROR, TAG, "Native registration unable to find class '%s'\n", className);
             return -1;
         }
         if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
             __android_log_print(ANDROID_LOG_ERROR, TAG, "RegisterNatives failed for '%s'\n", className);
             return -1;
         }
         return 0;

#else
     clazz = (*env)->FindClass(env,className);

     if (clazz == NULL)
     {
             __android_log_print(ANDROID_LOG_ERROR, TAG, "Native registration unable to find class '%s'\n", className);
             return -1;
      }
      if ((*env)->RegisterNatives(env,clazz, gMethods, numMethods) < 0)
      {
             __android_log_print(ANDROID_LOG_ERROR, TAG, "RegisterNatives failed for '%s'\n", className);
             return -1;
       }
      return 0;
#endif
     }


    int register_JNILib_im81ndk(JNIEnv *env) {
    	return jniRegisterNativeMethods(env, JAVA_CLASS_NAME, method_table, NELEM(method_table));
    }

 // JNI_OnLoad() will be runned first, when  VM load a Clib
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
        JNIEnv* env = NULL;
        jint result = JNI_ERR;
        sVm = vm;

#ifdef __cplusplus

        if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
#else
        if ((*vm)->GetEnv(vm,(void**)&env, JNI_VERSION_1_4) != JNI_OK){
#endif
        	__android_log_print(ANDROID_LOG_ERROR, TAG, "GetEnv failed!");
            return result;
        }

        __android_log_print(ANDROID_LOG_INFO, TAG, "loading81 .123 . .");


        if(register_JNILib_im81ndk(env) != JNI_OK) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "can't load register_JNILib_im81ndk():  %s",JAVA_CLASS_NAME);
            goto end;
        }
        __android_log_print(ANDROID_LOG_INFO, TAG, "loaded");

        // 动态加载ndk库，nl_ndk.so和nlposapi.so
        result =  ndk_dlload();


         if(result < 0){
        	 __android_log_print(ANDROID_LOG_ERROR, TAG, "JNI_OnLoad dllload  fail!!");
        	 // TODO  load fail..,   	 Error handling required
        	 //             goto end;
         }

          __android_log_print(ANDROID_LOG_ERROR, TAG, "JNI_OnLoad succ.");
        result = JNI_VERSION_1_4;

    end:
        return result;
    }


	void newlandAPI_printf(char * fmt,...){
        	char buffer[80];

        	va_list args;
        	va_start(args, fmt);
        	vsprintf(buffer, fmt, args);
        	LOGI("printf call from demoLib:  %s",buffer);
        	va_end(args);

        }

