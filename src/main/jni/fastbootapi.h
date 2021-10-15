#ifndef _FASTBOOTAPI_H_
#define _FASTBOOTAPI_H_
#include "usb.h"


#ifdef __cplusplus
 extern "C"
 {
#endif // __cplusplus 



 int  OSX_GetHandle();
 int  OSX_GetBootVersion(usb_handle *pUHandle,char *pcBootVersion);		//获取boot版本
 int  OSX_GetVariousInfo(usb_handle *pUHandle,char *pcVariousInfo);		//获取多个信息
 usb_handle*  OSX_OpenUsb();		//打开bootloader Usb端口
 int  OSX_CloseUsb(usb_handle *pUHandle);								//关闭bootloader Usb的端口
 int  OSX_Reboot(usb_handle *pUHandle);
 int  OSX_BootLoader(usb_handle *pUHandle);
 int  OSX_NewDownLoad(usb_handle *pUHandle,char *pcPatitionName,char *pcFileName ,char *pErrMsg);
#ifdef __cplusplus
}
#endif // __cplusplus 

#endif

