#include <jni.h>
#include <string>
#include <jni.h>
#include "downloader.h"
#include "native-lib.h"
#include <android/log.h>
#include <asm/fcntl.h>
#include <fcntl.h>

JavaVM *g_VM;
jobject g_obj;
jclass jSdkClass;

int mNeedDetach = 0;

int GetStatus(char* pszCurFile ,int nProgress)
{
    LOGE("pszCurFile:%s,nProgress:%d\n",pszCurFile,nProgress);

    JNIEnv *env;
    //获取当前native线程是否有没有被附加到jvm环境中
    int getEnvStat = g_VM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        //如果没有， 主动附加到jvm环境中，获取到env
        if (g_VM->AttachCurrentThread(&env, NULL) != 0) {
            return NULL;
        }
        mNeedDetach = JNI_TRUE;
    }

    //通过全局变量g_obj 获取到要回调的类
    jclass javaClass = env->GetObjectClass(g_obj);
    if (javaClass == 0) {
        LOGE("Unable to find class");
        g_VM->DetachCurrentThread();
        return NULL;
    }
    //获取要回调的方法ID
    jmethodID javaCallbackId = env->GetMethodID(javaClass,"onProgressCallBack", "(Ljava/lang/String;I)I");
    if (javaCallbackId == NULL) {
        LOGD("Unable to find method:onProgressCallBack");
        return NULL;
    }

    jstring rtstr = env->NewStringUTF(pszCurFile);

    //执行回调
    env->CallIntMethod( g_obj, javaCallbackId,rtstr,nProgress);
    env->DeleteLocalRef(rtstr);
    env->DeleteLocalRef(javaClass);

//    env->ReleaseStringUTFChars(rtstr, "utf-8");
    //释放当前线程
    if(mNeedDetach) {
        g_VM->DetachCurrentThread();
    }
    env = NULL;
    return 1;
}

extern "C"
JNIEXPORT jint
Java_com_newland_download_ndk_NdkApi_Pos_1DownLoaderInterface(JNIEnv *env, jclass clazz,
                                                              jint n_type, jint n_plat,
                                                              jstring sz_dev, jstring sz_down_file,
                                                              jstring sz_app_list, jboolean b_clear,
                                                              jbyteArray isz_result,jbyteArray sz_result) {
    // TODO: implement Pos_DownLoaderInterface()

    env->GetJavaVM(&g_VM);
    // 生成一个全局引用保留下来，以便回调
    g_obj = env->NewGlobalRef(clazz);

     char * psz_dev;
     char * psz_down_file;
     char * psz_app_list;
     char * psz_result;
     char * pisz_result;;

    if (sz_dev != NULL){
        psz_dev = const_cast<char *>(env->GetStringUTFChars(sz_dev, 0));
    }
    if (sz_down_file != NULL){
        psz_down_file = const_cast<char *>(env->GetStringUTFChars(sz_down_file, 0));
    }
    if (sz_app_list != NULL){
        psz_app_list = const_cast<char *>(env->GetStringUTFChars(sz_app_list, 0));
    }

    if (sz_result != NULL){
        psz_result = reinterpret_cast<char *>(env->GetByteArrayElements(sz_result, 0));
    }

    if (isz_result != NULL){
        pisz_result = reinterpret_cast<char *>(env->GetByteArrayElements(isz_result, 0));
    }

    int ret = -1;
    ret = DownLoaderInterface(n_type,n_plat, psz_dev,psz_down_file,psz_app_list,b_clear, pisz_result,GetStatus);

    LOGE("psz_dev = %s", psz_dev);
    LOGE("psz_result = %s", pisz_result);

    if (psz_dev != NULL){
        env->ReleaseStringUTFChars(sz_dev, psz_dev);
    }

    if (psz_down_file != NULL){
        env->ReleaseStringUTFChars(sz_down_file, psz_down_file);
    }

    if (psz_app_list != NULL){
        env->ReleaseStringUTFChars(sz_app_list, psz_app_list);
    }

    memcpy(psz_result,pisz_result,strlen(pisz_result));
    if (psz_result != NULL){
        env->ReleaseByteArrayElements( sz_result, (jbyte *) psz_result ,0);
    }

    if (pisz_result != NULL){
        env->ReleaseByteArrayElements( isz_result, (jbyte *) pisz_result ,0);
    }

    return ret;
}
extern "C"
JNIEXPORT void
Java_com_newland_download_ndk_NdkApi_Pos_1SetUsbStatus(JNIEnv *env, jobject clazz,jboolean jConnect){
    SetUsbStatus(jConnect);
}

//JavaVM *g_VM;
//int mNeedDetach;
//在此处跑在子线程中，并回调到java层
//void *download(void *p) {
//
//    if(p == NULL) return NULL;
//
//    JNIEnv *env;
//    //获取当前native线程是否有没有被附加到jvm环境中
//    int getEnvStat = g_VM->GetEnv((void **)   &env,JNI_VERSION_1_6);
//    if (getEnvStat == JNI_EDETACHED) {
//        //如果没有， 主动附加到jvm环境中，获取到env
//        if (g_VM->AttachCurrentThread(&env, NULL) != 0) {
//            return NULL;
//        }
//        mNeedDetach = JNI_TRUE;
//    }
//    //强转回来
//    jobject jcallback = (jobject)p;
//
//    //通过强转后的jcallback 获取到要回调的类
//    jclass javaClass = env->GetObjectClass(jcallback);
//
//    if (javaClass == 0) {
//        LOGE("Unable to find class");
//        g_VM->DetachCurrentThread();
//        return NULL;
//    }
//
//    //获取要回调的方法ID
//    jmethodID javaCallbackId = env->GetMethodID(javaClass,
//                                                   "onProgressChange", "(II)I");
//    if (javaCallbackId == NULL) {
//        LOGD("Unable to find method:onProgressCallBack");
//        return NULL;
//    }
//    //执行回调
//    env->CallIntMethod( jcallback, javaCallbackId,1,1);
//    //释放当前线程
//    if(mNeedDetach) {
//        g_VM->DetachCurrentThread();
//    }
////    env = NULL;
////    //释放你的全局引用的接口，生命周期自己把控
////    env->DeleteGlobalRef( jcallback);
////    jcallback = NULL;
//
//    return NULL;
//}
//
//extern "C"
//JNIEXPORT jint JNICALL
//Java_com_newland_download_ndk_NdkApi_nativeDownload(JNIEnv *env, jobject thiz,jstring jpath, jobject jcallback) {
//    // TODO: implement nativeDownload()
//
//    //JavaVM是虚拟机在JNI中的表示，等下再其他线程回调java层需要用到
//    env->GetJavaVM( &g_VM);
//
//    //生成一个全局引用，回调的时候findclass才不会为null
//    jobject callback = env->NewGlobalRef(jcallback);
//    pthread_t tidp;
//    // 把接口传进去，或者保存在一个结构体里面的属性， 进行传递也可以
////    pthread_create(&tidp,NULL, download,callback);
//    return 0;
//}




/***********************/

//
//JavaVM *g_VM;
//jobject g_obj;
//
//int mNeedDetach = 0;
////在此处跑在子线程中，并回调到java层
//void *download(void *p) {
//    JNIEnv *env;
//
//    //获取当前native线程是否有没有被附加到jvm环境中
//    int getEnvStat = g_VM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
//    if (getEnvStat == JNI_EDETACHED) {
//        //如果没有， 主动附加到jvm环境中，获取到env
//        if (g_VM->AttachCurrentThread(&env, NULL) != 0) {
//            return NULL;
//        }
//        mNeedDetach = JNI_TRUE;
//    }
//
//    //通过全局变量g_obj 获取到要回调的类
//    jclass javaClass = env->GetObjectClass(g_obj);
//    if (javaClass == 0) {
//        LOGE("Unable to find class");
//        g_VM->DetachCurrentThread();
//        return NULL;
//    }
//    //获取要回调的方法ID
//    jmethodID javaCallbackId = env->GetMethodID(javaClass,"onProgressCallBack", "(II)I");
//    if (javaCallbackId == NULL) {
//        LOGD("Unable to find method:onProgressCallBack");
//        return NULL;
//    }
//    LOGE("Unable to find class2");
//    //执行回调
//    env->CallIntMethod( g_obj, javaCallbackId,1,1);
//
//    //释放当前线程
//    if(mNeedDetach) {
//        g_VM->DetachCurrentThread();
//    }
//    env = NULL;
//    return NULL;
//}
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_com_newland_download_ndk_NdkApi_nativeDownload(JNIEnv *env, jclass thiz) {
//    // TODO: implement nativeDownload()
//
//    //直接用GetObjectClass找到Class, 也就是Sdk.class.
////    jSdkClass = env->FindClass("com/newland/download/ndk/NdkApi");
////    if (jSdkClass == 0) {
////        LOGE("Unable to find class");
////        return;
////    }
//
////    //找到需要调用的方法ID
////    jmethodID javaCallback = env->GetMethodID(jSdkClass,"onProgressCallBack", "(II)I");
////    //这时候要回调还没有jobject，那就new 一个
////    jmethodID sdkInit = env->GetMethodID( jSdkClass,"<init>","()V");
////    jobject jSdkObject = env->NewObject(jSdkClass,sdkInit);
////
////    //进行回调，ret是java层的返回值（这个有些场景很好用）
////    jint ret = env->CallIntMethod(jSdkObject, javaCallback,1,1);
//
//
//    //JavaVM是虚拟机在JNI中的表示，等下再其他线程回调java层需要用到
//    env->GetJavaVM(&g_VM);
//    // 生成一个全局引用保留下来，以便回调
//    g_obj = env->NewGlobalRef(thiz);
//
//    pthread_t tidp;
//    // 此处使用c语言开启一个线程，进行回调，这时候java层就不会阻塞，只是在等待回调
//    pthread_create(&tidp, NULL, download, NULL);
//    return ;
//}
