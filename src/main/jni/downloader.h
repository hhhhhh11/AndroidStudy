/*******************************************************************************
 * Copyright (C) 2021 Newland Payment Technology Co., Ltd All Rights Reserved
 * @file 	downloader.h
 * @brief	dl.so的接口说明文件
 * @version	1.0
 * @author: ym
 * @date	2021/05/18
 ******************************************************************************/
#ifndef _DOWNLOADER_H_
#define _DOWNLOADER_H_


/**
 * @brief	dl.so 调用接口
 * @param[in] nType	DOWN_TYPE
 * @param[in] nPlat    POS_PLAT
 * @param[in] pszDev    设备号
 * @param[in] pszDownFile   要下载的文件，可以是新大陆指定类型包文件，或者list文件
 * @param[in] pszAppList    删除的应用名，通过‘|’连接,不是删除应用，传入NULL
 * @param[in] bClear    是否清空应用，true 清空，false 不清空
 * @param[out] pszResult    返回错误信息
 * @param[out] pszCurFile    回调，返回当前正在下载文件
 * @param[out] nProgress    回调，返回当前下载进度
 * @return
 * @li  0		成功
 * @li  <0		错误
*/
extern "C"{
int DownLoaderInterface(int nType,int nPlat,char* pszDev,char* pszDownFile,char *pszAppList,bool bClear, char* pszResult,int (*pnCallBackStatus)(char* pszCurFile ,int nProgress));
void SetUsbStatus(bool bConnect);
}



#endif 