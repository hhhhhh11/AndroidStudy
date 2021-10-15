#include <stdio.h>
#include <dlfcn.h>
int main(int argc, char *argv[])
{

//int DownLoaderInterface(int nType,int nPlat,char* pszDev,char* pszDownFile,char *pszAppList,bool bClear, char* pszResult);

	void * libm_handle = NULL;
	  char *errorInfo;
	  char szResult[1024] = {0};
	  float result;
	 
	 libm_handle = (void *)dlopen("dl.so", RTLD_LAZY );
	 if (!libm_handle){
	     LOGE("Open Error:%s.\n",dlerror());
	     return 0;
	 }
	typedef int (*cosf_method)(int nType,int nPlat,char* pszDev,char* pszDownFile,char *pszAppList,int bClear, char* pszResult);
	cosf_method interfase;
	interfase = (cosf_method)dlsym(libm_handle,"DownLoaderInterface");
	errorInfo = dlerror();
	 if (errorInfo != NULL){
	    LOGE("Dlsym Error:%s.\n",errorInfo);
	     return 0;
	 }
	// result = (*cosf_method)(0,1,"ttyACM0",);
	  result = interfase(0,1,"ttyACM0","test/KIP_OFFLINE_V1.1.10-Lib_DEV.NLD","",-1,szResult);
	 LOGE("result = %.2f\n",result);
	 dlclose(libm_handle);
	 return 0;

}
