#ifndef _FASTBOOTAPI_H_
#define _FASTBOOTAPI_H_
#include "usb.h"


#ifdef __cplusplus
 extern "C"
 {
#endif // __cplusplus 



 int  OSX_GetHandle();
 int  OSX_GetBootVersion(usb_handle *pUHandle,char *pcBootVersion);		//��ȡboot�汾
 int  OSX_GetVariousInfo(usb_handle *pUHandle,char *pcVariousInfo);		//��ȡ�����Ϣ
 usb_handle*  OSX_OpenUsb();		//��bootloader Usb�˿�
 int  OSX_CloseUsb(usb_handle *pUHandle);								//�ر�bootloader Usb�Ķ˿�
 int  OSX_Reboot(usb_handle *pUHandle);
 int  OSX_BootLoader(usb_handle *pUHandle);
 int  OSX_NewDownLoad(usb_handle *pUHandle,char *pcPatitionName,char *pcFileName ,char *pErrMsg);
#ifdef __cplusplus
}
#endif // __cplusplus 

#endif

