#include "function.h"
#include "inirw.h"
#include "arq.h"
#include "common.h"
#include "nlddownload.h"
#include "inirw.h"
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <errno.h>
#include <dlfcn.h>
#include <sys/time.h>

extern char g_szAppPath[1024];
extern char message[MSGSUM][100];

extern PFCALLBACK gCallBack ;





/**
 *@brief 构造
*/
CNLDDownload::CNLDDownload(void)
{	
	m_bClear = false;

	#if 0 // C++ 11,android设备可能不支持
	for (int  i = 0; i < sizeof(m_ArrInfoCount)/sizeof(int); i++)
	{
		m_ArrInfoCount[i] = 0;
	}
	for(auto itrow = begin(m_ArrInfoCmd);itrow != end(m_ArrInfoCmd);itrow++)
	{
		for (auto itcolumn = begin(*m_ArrInfoCmd); itcolumn != end(*m_ArrInfoCmd); itcolumn++) {
			*itcolumn = 0;
		}
	}
	#endif
	
}

/**
 *@brief	析构
*/
CNLDDownload::~CNLDDownload(void)
{
	
}

/**
 * @brief	设置是否清空
 * @param[in] bClear 清空标志
 * return void
*/
void CNLDDownload::SetClearFlag(bool bClear)
{
	m_bClear = bClear;
}

/**
 * @brief	通过usb-serail获取pos版本共包含哪些指令信息
 * @param[in] uProtocol 协议版本，变量用于兼容历史版本设计，当前使用实际都只有一个协议
 * @return void
*/
void CNLDDownload::GetInfoCmd(U16 uProtocol)
{
	int iCount = 5;
	char szSection[20] = {0};
	char szFile[1024] = {0} ;

	//对应配置文件para.ini中[protocol_%d] 节点配置，当前所有使用的都是protocol_1，默认pos信息有五个，0x0300-0x0334
	snprintf(szSection,sizeof(szSection),"protocol_%d",uProtocol);
	snprintf(szFile,sizeof(szFile),"%s/para.ini",g_szAppPath);

	CIniRW ini;
	if(1 == ini.iniFileLoad(szFile))
	{
		iCount = ini.iniGetInt(szSection,"Count",5);
		ini.iniFileFree();
	}
	
	m_ArrInfoCount[uProtocol] = iCount;
	for (int i=0; i<iCount; i++)
	{
		m_ArrInfoCmd[uProtocol][i] = 0x0300+i;

	}
	return;
}




/**
 * @brief	协议握手处理
 * @return 
 * @li 0	成功
 * @li -1	失败
*/
int CNLDDownload::ShakeHand(void)
{
	char szRecvBuff[30] = {0};
	char szSendBuff[30] = {0};
	int nRet = 0;
	int nTimes = 15;
	int nHeadLen = sizeof(MSG_HEAD);
	U16 uMagic = 0;
	U16 uCommand = 0;
	MSG_HEAD SendHead;

	memset(&SendHead,0,sizeof(MSG_HEAD));
	while(nTimes-- > 0)
	{
		memset(szRecvBuff,0x00,sizeof(szRecvBuff));
		nRet=m_objUsbSerial.ReadBuffData(szRecvBuff,nHeadLen,1000);	
		if (nRet == nHeadLen)
		{
			memcpy(&uMagic,szRecvBuff,2);
			memcpy(&uCommand,szRecvBuff+4,2);

			//步骤1，握手包
			if (POS_SHAKEHAND == uCommand && 0xAAAA == uMagic )
			{
				memset(&SendHead,0,sizeof(MSG_HEAD));
				memset(szSendBuff,0,sizeof(szSendBuff));
				
				SendHead.uMagic		=	0xCCCC;			
				SendHead.uSurport	=   1;
				SendHead.uCommand	=	POS_SHAKEHAND; 
				SendHead.uProVer	=	0x0001;			
				SendHead.uTotalNum	=	0x0001;			
				SendHead.uSeq		=	0x0001;		
				SendHead.uFieldCount=	0;				/*buff中包含几个TLV构成*/
				SendHead.uDataLen	=	0;				/*buff中的长度*/

				memcpy(szSendBuff,&SendHead, sizeof(MSG_HEAD));
				szSendBuff[2]=1;
				m_objUsbSerial.FlushPort();
				nRet = m_objUsbSerial.SendBuffData((char*) &szSendBuff, sizeof(MSG_HEAD));
				if( nRet !=  sizeof(MSG_HEAD))
				{
					return -1;
				}
			}
			//步骤2，返回请求命令
			else if ((uCommand==POS_REQUEST) && 0xAAAA == uMagic )
			{	
					GetInfoCmd(SendHead.uProVer);  
				
				return 0;
			}
		}
		
	}
	return -1;
}


/**
 * @brief	获取POS boot等信息
 * @param[out] pszPosInfo boot信息
 * @param[out] pnLen boot信息长度
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNLDDownload::GetPOSInfo(char* pszPosInfo,int* pnLen)
{
	char szRecvBuff[1024*2] = {0};
	char szSendBuff[20] = 	{0};
	char szDataBuff[128] = {0};
	U32 unResendCount = 0;		//重发次数
	U32 unHeadLen = 0;
	U32 unSendLen = 0;
	U16 usTmpCmd = 0;			//接收的命令字
	int nRet = 0;
	int nResendTimes = 10;
	string strResult;

	MSG_HEAD msgHeadRecv;
	MSG_HEAD msgHeadSend;
	FILE *fp = NULL;
	char *pInfo = NULL;

	unHeadLen = sizeof(MSG_HEAD);
	memset(&msgHeadSend,0,unHeadLen);
	memset(&msgHeadRecv,0,unHeadLen);
	memset(szSendBuff,0,sizeof(szSendBuff));
	msgHeadSend.uMagic		=	0xAAAA;		
	msgHeadSend.uSurport	=   1;
	msgHeadSend.uCommand	=	0x0300;		
	msgHeadSend.uProVer		=	0x0001;		
	msgHeadSend.uTotalNum	=	0x0001;		
	msgHeadSend.uSeq		=	0x0001;		
	msgHeadSend.uFieldCount=	0;			
	msgHeadSend.uDataLen	=	0;			
	memcpy(szSendBuff,&msgHeadSend,unHeadLen);
	unSendLen = unHeadLen;
	m_objUsbSerial.FlushPort();
	nRet = m_objUsbSerial.SendBuffData(szSendBuff,unSendLen);

	while (nRet != unSendLen )
	{
		if(nResendTimes-- <= 0 )
		{
			CFunCommon::DlSprintf("Failed to send data: %s.\n",strerror(errno));
			return false;
		}
		usleep(1000*50);
		m_objUsbSerial.FlushPort();
		nRet = m_objUsbSerial.SendBuffData((char*) &szSendBuff,unSendLen);
	}
	while(1)
	{
		memset(szRecvBuff,0,sizeof(szRecvBuff));
		nRet = m_objUsbSerial.ReadBuffData(szRecvBuff,unHeadLen,NLD_TIME_OUT);
		if(nRet != unHeadLen)
		{
			CFunCommon::DlSprintf("Failed to read data.\n");
			return false;
		}
		memcpy(&msgHeadRecv,szRecvBuff,unHeadLen);

		if (0xCCCC != msgHeadRecv.uMagic  || msgHeadRecv.uCommand != msgHeadSend.uCommand)
		{
			if (unResendCount > 3)
			{
				CFunCommon::DlSprintf("Failed to get pos info, Exceeding times.\n");
				return false;
			}
			m_objUsbSerial.FlushPort();
			m_objUsbSerial.SendBuffData(szSendBuff,unSendLen);
			unResendCount++;
		}
		else
		{
		
			if (0 != msgHeadRecv.uDataLen )
			{
				memset(szRecvBuff,0,sizeof(szRecvBuff));
				nRet = m_objUsbSerial.ReadBuffData(szRecvBuff,msgHeadRecv.uDataLen,NLD_TIME_OUT);
				
				if(nRet != msgHeadRecv.uDataLen)
				{
					CFunCommon::DlSprintf("Failed to recv boot info:%s\n",strerror(errno));
					return false;
				}
				if (0 == szRecvBuff[0] && 0 == szRecvBuff[1])
				{
					memset(szDataBuff,0,sizeof(szDataBuff));
					strncpy(szDataBuff,szRecvBuff+2,msgHeadRecv.uDataLen-3);
					strResult += szDataBuff;
					strResult += ",";	
				}
				
			}
			usTmpCmd = msgHeadRecv.uCommand+1;
			if ((usTmpCmd)<=m_ArrInfoCmd[msgHeadRecv.uProVer][m_ArrInfoCount[msgHeadRecv.uProVer]-1])
			{
				memset(&msgHeadSend,0,unHeadLen);
				memset(szSendBuff,0,sizeof(szSendBuff));
				
				msgHeadSend.uMagic		=	0xAAAA;		
				msgHeadSend.uSurport	=   1;
				msgHeadSend.uCommand	=	usTmpCmd;	
				msgHeadSend.uProVer		=	0x0001;		
				msgHeadSend.uTotalNum	=	0x0001;		
				msgHeadSend.uSeq		=	0x0001;		
				msgHeadSend.uFieldCount=	0;			
				msgHeadSend.uDataLen	=	0;			
				
				memcpy(szSendBuff,&msgHeadSend,unHeadLen);
				unSendLen = 0;
				unSendLen = unHeadLen;
				m_objUsbSerial.FlushPort();
				m_objUsbSerial.SendBuffData(szSendBuff,unSendLen);
			}
			else
			{				
							
				*pnLen = strResult.length();
				memcpy(pszPosInfo,strResult.c_str(),*pnLen);
				break;
			}
		}
	}
	
	return true;
}


/**
 * @brief	获取应用通讯交互
 * @param[out] nAppCount 返回的应用数量
 * @param[out] pszAppList 返回的应用列表
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNLDDownload::GetAppProcess(int* nAppCount,char* pszAppList)
{
	char szRecvBuff[1024*16];
	char szSendBuff[30];
	U16 usResult = 0;
	int nRet = 0;
	int nHeadLen = sizeof(MSG_HEAD);
	MSG_HEAD RecHead;
	MSG_HEAD SendHead;
	
	memset(&SendHead,0,sizeof(MSG_HEAD));
	memset(szSendBuff,0,sizeof(szSendBuff));
	
	SendHead.uMagic		=	0xAAAA;		/*报文魔术字*/
	SendHead.uSurport	=   1;
	SendHead.uCommand	=	POS_GETAPP; /*握手命令字*/
	SendHead.uProVer	=	0x0001;		/*协议版本*/
	SendHead.uTotalNum	=	0x0001;		/*报文总数*/
	SendHead.uSeq		=	0x0001;		/*当前包序列号*/
	SendHead.uFieldCount=	0;			/*buff中包含几个TLV构成*/
	SendHead.uDataLen	=	0;			/*buff中的长度*/
	
	memcpy(szSendBuff,&SendHead, nHeadLen);
	m_objUsbSerial.FlushPort();
	m_objUsbSerial.SendBuffData(szSendBuff, nHeadLen);
	

	memset(szRecvBuff,0x00,sizeof(szRecvBuff));
	nRet=m_objUsbSerial.ReadBuffData(szRecvBuff,nHeadLen,5000);
	if (nRet == nHeadLen)
	{
		memcpy(&RecHead,szRecvBuff,nHeadLen);
		nRet = m_objUsbSerial.ReadBuffData(szRecvBuff,RecHead.uDataLen,1000);
		
		if (nRet != RecHead.uDataLen)
		{
			m_objUsbSerial.SendBuffData(szSendBuff, nHeadLen);
				return false;
		}
		else
		{
			szRecvBuff[RecHead.uDataLen-1]='\0';
			*nAppCount=RecHead.uFieldCount;
			if (RecHead.uFieldCount != 0)
				memcpy(pszAppList,szRecvBuff+2,RecHead.uDataLen-3);

			return true;
		}
	}
	return true;
}

/**
 * @brief	清空POS应用信息
 * @return
 * @li true		成功
 * @li false	失败
*/
bool CNLDDownload::ClearApp(void)
{
	char szRecvBuff[30] = {0};
	char szSendBuff[30] = {0};
	U16 usResult = 0;
	int nRet = 0;
	int nSendTimes = 0;
	int nHeadLen = sizeof(MSG_HEAD);

	MSG_HEAD msgHeadRecv;
	MSG_HEAD msgHeadSend;

	memset(&msgHeadSend,0,sizeof(MSG_HEAD));
	memset(szSendBuff,0,30);		
	msgHeadSend.uMagic		=	0xAAAA;		
	msgHeadSend.uSurport	=   1;
	msgHeadSend.uCommand	=	POS_DELAPP_ALL; 
	msgHeadSend.uProVer	=	0x0001;		
	msgHeadSend.uTotalNum	=	0x0001;		
	msgHeadSend.uSeq		=	0x0001;		
	msgHeadSend.uFieldCount=	0;			
	msgHeadSend.uDataLen	=	0;			
				
	memcpy(szSendBuff,&msgHeadSend, nHeadLen);
	m_objUsbSerial.FlushPort();
	m_objUsbSerial.SendBuffData(szSendBuff, nHeadLen);
	while(1)
	{
		memset(szRecvBuff,0x00,sizeof(szRecvBuff));
		nRet = m_objUsbSerial.ReadBuffData(szRecvBuff,nHeadLen,NLD_TIME_OUT);
		if (nRet == nHeadLen)
		{
			memcpy(&msgHeadRecv,szRecvBuff,nHeadLen);
			if (0 != msgHeadRecv.uDataLen)
			{
				nRet = m_objUsbSerial.ReadBuffData(szRecvBuff,msgHeadRecv.uDataLen,NLD_TIME_OUT);
				if (nRet == 2)
					memcpy(&usResult,szRecvBuff,2);
				else
					usResult = 0xff;
			}
			if (0xff == usResult)
			{
				m_objUsbSerial.SendBuffData(szSendBuff, nHeadLen);
				nSendTimes++;

				if (nSendTimes > 3) 
					return false;
			}
			else
				return true;	
		}
	}

	return true;
}



/**
 * @brief	从队列获取下一个要下载文件
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNLDDownload::GetNextFile(void)
{
	if (!m_queFiletype.empty())
	{
		m_queFiletype.pop();
	}
	if (!m_queFilepath.empty())
	{
		m_queFilepath.pop();
	}
	if (m_queFilepath.empty()) 
	{	
        return false;		
	} 
    return true;
}


/**
 * @brief	下载应用
 * @param[in] pszDownFileList 需要下载文件列表
 * @param[in] nMaxSend 每帧最大大小 。 限定最大可以发送为NEW_MAX_BUFF_LEN 100k，当前使用都是NEW_FRAME_BUFF_LEN 24k
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNLDDownload::DownloadApp(const char* pszDownFileList,int nMaxSend)
{
	
	U8  ucChkSum = 0;
	U16	usResult = 0;	
	int nTotalpkg = 0;	//总的包数
	int	nCurrpkg = 1;	//当前发包序号
	int	nReSendTimes = 0;
	int	i = 0;
	int	nPos = 0;
	int	nRet = 0;
	int	nHeadLen = sizeof(MSG_HEAD);
	int	nFileLen = 0;		
	int	nSendLen = 0;		//发送buff长度
	int	nFileReadLen = 0;	//文件读取长度	
	int nMaxBuffSend=0;
	int nProgress =0;
	float fProgress;

	char szSendBuff[NEW_MAX_BUFF_LEN] = {0};	
	char szFileName[512] = {0};  //被下载文件完整路径名 
	char szRecvBuff[30]	= {0};	
	MSG_HEAD msgHeadSend;			
	MSG_HEAD msgHeadRecv;		

	FILE *fp = NULL;
	string strFileName;
	string strTmp;
	string strTotal;
	

	strTotal = pszDownFileList;
	//从文件列表解析出每个文件全路径文件名，文件类型
	while (strTotal != "")
	{	
		nPos = strTotal.find('|');
		if(nPos != string::npos)
		{
			strTmp = strTotal.substr(0,nPos );
		}			
		else
		{
			strTmp = strTotal;
		}			
		strTotal = strTotal.substr(nPos+1);		
		nPos = strTmp.find('*');
		if(nPos != string::npos)
		{
			strFileName = strTmp.substr(0,nPos );
		}		
		else
		{
			strFileName = strTmp;
		}			
		m_queFilepath.push(strFileName);
		strFileName = strTmp.substr(nPos+1);		
		nPos = atoi(strFileName.c_str());
		m_queFiletype.push(nPos);
	}	
	
	strFileName = m_queFilepath.front();
	snprintf(szFileName,sizeof(szFileName),"%s",strFileName.c_str());
	
	//CFunCommon::DlSprintf("szFileName=%s\n",szFileName);
	fp = fopen(szFileName,"rb");
	if (NULL == fp )
	{
		CFunCommon::DlSprintf("Failed to open %s.\n",szFileName);
		return false;
	}
	fseek(fp,0,SEEK_END);
	nFileLen = ftell(fp);

	nMaxBuffSend = nMaxSend < NEW_MAX_BUFF_LEN?nMaxSend:NEW_MAX_BUFF_LEN;
	

	nFileReadLen = nMaxBuffSend-nHeadLen-1;
	
	if (nFileLen > nFileReadLen)
	{								
		nTotalpkg = nFileLen/nFileReadLen;
		nCurrpkg = nFileLen%nFileReadLen;
		if (0 != nCurrpkg )
			nTotalpkg++;
	}
	else
	{
		nFileReadLen = nFileLen;
		nTotalpkg = 1;
	}
	
	nPos = nFileReadLen;
	msgHeadSend.uMagic 		=	0xAAAA;
	msgHeadSend.uSurport	=	1;
	msgHeadSend.uProVer		=	0x0001;
	nCurrpkg				=	1;
	msgHeadSend.uTotalNum	=	nTotalpkg;		
	msgHeadSend.uSeq		=	nCurrpkg;		
	msgHeadSend.uFieldCount	=	1;			
	
	memset(szSendBuff,0,sizeof(szSendBuff));						
	fseek(fp,0,SEEK_SET);
	fread(szSendBuff+nHeadLen,nFileReadLen,1,fp);
	fclose(fp);
	ucChkSum = 0;
	for (i=nHeadLen; i<nHeadLen+nFileReadLen; i++)
	{
		ucChkSum += (U8)szSendBuff[i];
	}
	nSendLen = 0;
	msgHeadSend.uDataLen = nFileReadLen+1;			
	szSendBuff[nHeadLen+nFileReadLen] =	ucChkSum;
	nSendLen = nHeadLen+nFileReadLen+1;			
	msgHeadSend.uCommand = POS_DOWNAPP;		
	memcpy(szSendBuff,&msgHeadSend,nHeadLen);						
	
	m_objUsbSerial.FlushPort();
	nRet = m_objUsbSerial.SendBuffData(szSendBuff, nSendLen);	


	//CFunCommon::DlSprintf("Downloding %s\n",szFileName);

	nReSendTimes = 0;	

	while(1)
	{
		memset(szRecvBuff,0x00,sizeof(szRecvBuff));
		nRet = m_objUsbSerial.ReadBuffData(szRecvBuff,nHeadLen,NLD_TIME_OUT);
		
		if (nRet != nHeadLen)
		{
			if (nReSendTimes > 3)
			{
				return false;
			}
			m_objUsbSerial.SendBuffData(szSendBuff, nSendLen);
			nReSendTimes++;
		}
		else
		{
			memcpy(&msgHeadRecv,szRecvBuff,sizeof(MSG_HEAD));
			memset(szRecvBuff,0x00,sizeof(szRecvBuff));
			m_objUsbSerial.ReadBuffData(szRecvBuff,msgHeadRecv.uDataLen,NLD_TIME_OUT);
			memcpy(&usResult,szRecvBuff,msgHeadRecv.uDataLen);		
		
			//收包判断结果，结果出错，重发
			if (0xff == usResult)
			{				
				if (nReSendTimes > 3)
					return false;

				m_objUsbSerial.SendBuffData(szSendBuff, nSendLen);
				nReSendTimes++;
			}
			else
			{				
				if (msgHeadRecv.uTotalNum != msgHeadRecv.uSeq)
				{
					//继续读取下一包
					fp = fopen(szFileName,"rb");
					fseek(fp,nPos,SEEK_SET);
					
					if ((nFileLen-nPos) > (nMaxBuffSend-nHeadLen-1))
					{
						nFileReadLen = nMaxBuffSend-nHeadLen-1;
					}
					else
					{
						nFileReadLen = nFileLen-nPos;
					}
					
					memset(&msgHeadSend,0,nHeadLen);
					memset(szSendBuff,0,NEW_MAX_BUFF_LEN);
					
					fread(szSendBuff+nHeadLen,nFileReadLen,1,fp);
					fclose(fp);
					nPos = nPos+nFileReadLen;
					nCurrpkg++;
					msgHeadSend.uMagic		=	0xAAAA;		
					msgHeadSend.uSurport	=   0x0001;
					msgHeadSend.uCommand	=	POS_DOWNAPP; 				
					msgHeadSend.uProVer	=	1;		
					msgHeadSend.uTotalNum	=	nTotalpkg;		
					msgHeadSend.uSeq		=	nCurrpkg;	
					msgHeadSend.uFieldCount=	1;			/*buff包含几个TLV构成*/
					msgHeadSend.uDataLen	=	nFileReadLen+1;			
					
					memcpy(szSendBuff,&msgHeadSend,nHeadLen);
					ucChkSum = 0;
					for (i=nHeadLen; i<nFileReadLen+nHeadLen; i++)
					{
						ucChkSum += (unsigned char)szSendBuff[i];
					}
					
					szSendBuff[nHeadLen+nFileReadLen] = ucChkSum;
					
					nSendLen=nHeadLen+nFileReadLen+1;

					m_objUsbSerial.FlushPort();
					nRet = m_objUsbSerial.SendBuffData(szSendBuff, nSendLen);	
					fProgress = 0.1f*nPos*1000/(nFileLen);
					nProgress = fProgress;
				
					(*gCallBack)(szFileName,nProgress);
				//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
				}
				else
				{
					nFileLen = 0;
					nFileReadLen = 0;
				//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);
					(*gCallBack)(szFileName,100);
					if (GetNextFile())
					{
						
						//下发下一个文件
						nPos = 0;
						strFileName = m_queFilepath.front();
						memset(szFileName, 0x00, sizeof(szFileName));
						memcpy(szFileName, strFileName.c_str(), strFileName.length());
					
						fp = fopen(szFileName,"rb");
						fseek(fp,0,SEEK_END);
						nFileLen = ftell(fp);
					//	CFunCommon::DlSprintf("Downloding %s\n",szFileName);
						nMaxBuffSend = nMaxSend<NEW_MAX_BUFF_LEN?nMaxSend:NEW_MAX_BUFF_LEN;
						nFileReadLen = nMaxBuffSend-nHeadLen-1;
						
						if (nFileLen > nFileReadLen)
						{								
							nTotalpkg = nFileLen/nFileReadLen;
							nCurrpkg = nFileLen%nFileReadLen;
							
							if (0 != nCurrpkg)
								nTotalpkg++;
						}
						else
						{
							nFileReadLen = nFileLen;
							nTotalpkg = 1;
						}
						
						nPos+=nFileReadLen;							
						nCurrpkg=1;
						msgHeadSend.uMagic		=	0xAAAA;		
						msgHeadSend.uSurport	=   0x0001;
						msgHeadSend.uProVer		=   0x0001;
						msgHeadSend.uTotalNum	=	nTotalpkg;		
						msgHeadSend.uSeq		=	nCurrpkg;		
						msgHeadSend.uFieldCount=	1;				
						msgHeadSend.uCommand	=	POS_DOWNAPP;	
						
						memset(szSendBuff,0,sizeof(szSendBuff));							
						fseek(fp,0,SEEK_SET);
						fread(szSendBuff+nHeadLen,nFileReadLen,1,fp);
						fclose(fp);
						
						ucChkSum = 0;
						for (i=nHeadLen; i<nHeadLen+nFileReadLen; i++)
						{
							ucChkSum += (unsigned char)szSendBuff[i];
						}
						msgHeadSend.uDataLen = nFileReadLen+1;			
						szSendBuff[nHeadLen+nFileReadLen] = ucChkSum;
						nSendLen = nHeadLen+nFileReadLen+1;
						memcpy(szSendBuff,&msgHeadSend,nHeadLen);
						
						
						m_objUsbSerial.FlushPort();
						m_objUsbSerial.SendBuffData(szSendBuff, nSendLen);	

						fProgress = 0.1f*nPos*1000/(nFileLen);
						nProgress = fProgress;
					
						(*gCallBack)(szFileName,nProgress);
					//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
						(*gCallBack)(szFileName,nProgress);
					}
					else
					{						
						break;
					}
				}
				
			}  
			
		}
	}

	memset(&msgHeadSend,0,sizeof(MSG_HEAD));
	memset(szSendBuff,0,sizeof(szSendBuff));
	
	msgHeadSend.uMagic		=	0xAAAA;		
	msgHeadSend.uSurport	=   1;
	msgHeadSend.uCommand	=	POS_INSTAPP; 
	msgHeadSend.uProVer		=	0x0001;		
	msgHeadSend.uTotalNum	=	0x0001;		
	msgHeadSend.uSeq		=	0x0001;		
	msgHeadSend.uFieldCount=	0;			
	msgHeadSend.uDataLen	=	0;			
	
	(*gCallBack)("Installing app, please wait",-1);

	memcpy(szSendBuff,&msgHeadSend, nHeadLen);
	nSendLen = nHeadLen;
	m_objUsbSerial.FlushPort();
	m_objUsbSerial.SendBuffData(szSendBuff, nSendLen);	
	nReSendTimes = 0;
	while(1)
	{
		memset(szRecvBuff,0x00,sizeof(szRecvBuff));
		nRet=m_objUsbSerial.ReadBuffData(szRecvBuff,nHeadLen,NLD_TIME_OUT*3);	
		if (nRet != nHeadLen)
		{
			return false;
		}
		else
		{
			memcpy(&msgHeadRecv,szRecvBuff,sizeof(MSG_HEAD));
	
			m_objUsbSerial.ReadBuffData(szRecvBuff,msgHeadRecv.uDataLen,NLD_TIME_OUT);
			memcpy(&usResult,szRecvBuff,msgHeadRecv.uDataLen);		
			
			
			if (0xff == usResult)
			{				
				if (nReSendTimes > 3)
				{
					return false;
				}				
				m_objUsbSerial.FlushPort();
				m_objUsbSerial.SendBuffData(szSendBuff, nSendLen);
				nReSendTimes++;
			}
			else
			{
				return true;
			}
		}
	}

	return true;
}


/**
 * @brief	结束下载通讯流程
 * @return void
*/
void CNLDDownload::DownloadEnd(void)
{
	char szSendBuff[30] = {0};
	MSG_HEAD msgHead;
	
	int nHeadLen = sizeof(MSG_HEAD);
	
	memset(&msgHead,0,sizeof(MSG_HEAD));
	memset(szSendBuff,0,sizeof(szSendBuff));			
	msgHead.uMagic		=	0xAAAA;		
	msgHead.uSurport	=   1;
	msgHead.uCommand	=	POS_END; 
	msgHead.uProVer		=	0x0001;		
	msgHead.uTotalNum	=	0x0001;		
	msgHead.uSeq		=	0x0001;		
	msgHead.uFieldCount=	0;			
	msgHead.uDataLen	=	0;			
				
	memcpy(szSendBuff,&msgHead, nHeadLen);
	m_objUsbSerial.FlushPort();
	m_objUsbSerial.SendBuffData(szSendBuff, nHeadLen);

	return;
}




/**
 * @brief	NLD下载主控流程
 * @param[in] pszFileList 下发文件列表
 * @param[in] pszDev 设备号
 * @return
 * @li 0	成功
 * @li <0 	失败
*/
int CNLDDownload::DownloadProcess(const char *pszFileList,const char *pszDev)
{
	char szPosInfo[1024] = {0};
	int nInfoLen = 0;
	int nRet = 0;
	char szDevicePath[256] = {0};

	chdir(g_szAppPath);
	sprintf(szDevicePath,"/dev/%s",pszDev);

//	CFunCommon::DlSprintf("Device:%s\n",szDevicePath);
	if(!m_objUsbSerial.OpenPort(szDevicePath))
	{
		CFunCommon::DlSprintf("Failed to open  %s.\n",szDevicePath,strerror(errno));
		return -1;
	}
	nRet = ShakeHand();	
	if (-1 == nRet)
	{
		CFunCommon::DlSprintf("Failed to shakehand:%s.\n",strerror(errno) );
		return -2;	
	}
	if(!GetPOSInfo(szPosInfo,&nInfoLen))
	{
		CFunCommon::DlSprintf("Failed to GetPosInfo.\n");
		return -3;	
	}
	CFunCommon::DlSprintf("%s\n",szPosInfo);
	if(m_bClear)
	{
		if(!ClearApp())
		{
			return -4;
		}
		
	}

	if (DownloadApp(pszFileList,NEW_FRAME_BUFF_LEN))
	{
		DownloadEnd();
		CFunCommon::DlSprintf("%s\n",DOWN_SUCC);
		return 0;
	}
	CFunCommon::DlSprintf("Failed to Download.\n" );	
	return -5;
	
}


/**
 * @brief 	解析并打印app list
 * @param[out] strFileList    返回的应用列表信息
 * @return void
*/
void CNLDDownload::ParseAppList(string strFileList)
{
	string strApplist;
	string strTmp,strTmpName,strTmpVer,strTmpPacket;

	int i = 0,nPos = 0,nSubPos = 0;
	int nTag = 0; //是否最后一个数据

	strApplist = strFileList;
	
	while (1)
	{
		nPos = strApplist.find('|');
		if (-1 == nPos )
		{
			if (1 == nTag)
			{
				return;
			}
			nTag = 1;
		}	
		strTmp = strApplist.substr(0,nPos);
		if(strTmp.find('*') != -1)
		{		
			nSubPos = strTmp.find('*');
			// app name
			strTmpName = strTmp.substr(0,nSubPos);
			strTmp = strTmp.substr(nSubPos+1);
			//ver
			nSubPos=strTmp.find('*');
			strTmpVer=strTmp.substr(0,nSubPos);
			strTmp = strTmp.substr(nSubPos+1);
			//pack name
			nSubPos=strTmp.find('*');
			strTmp = strTmp.substr(nSubPos+1);
			//高端平台最后一个字段数据改为包名
			if (strTmp.length() >1)
			{
				strTmpPacket = strTmp;
			}
			else{
				if (strTmp=="1")
						strTmpPacket="Main NLD";
				else
						strTmpPacket="Program NLD";
			}
			CFunCommon::DlSprintf("%d*%s*%s*%s\n",i+1,strTmpName.c_str(),strTmpVer.c_str(),strTmpPacket.c_str());
			
		}
		strApplist = strApplist.substr(nPos+1);
		i++;
	}
	return;
}

/**
 * @brief	获取设备应用列表
 * @param[in] pszDev 设备号
 * @return	void
*/
int CNLDDownload::GetAppList(const char *pszDev)
{
	
	
	int nAppCount = 0;
	int nRet = 0;
	char szDevicePath[256] = {0};
	char szApp[1024*16] = {0};

	chdir(g_szAppPath);
	sprintf(szDevicePath,"/dev/%s",pszDev);
	//CFunCommon::DlSprintf("Device:%s\n",szDevicePath);
	if(!m_objUsbSerial.OpenPort(szDevicePath))
	{
		CFunCommon::DlSprintf("Failed to open  %s.\n",szDevicePath,strerror(errno));
		return -1 ;
	}

	nRet = ShakeHand();	
	if (-1 == nRet)
	{
		CFunCommon::DlSprintf("Failed to shakehand:%s.\n",strerror(errno) );
		return -1;	
	}
	if(!GetAppProcess(&nAppCount,szApp))
	{
		CFunCommon::DlSprintf("Failed to get app list:%s.\n",strerror(errno) );
		return -1;
	}
	DownloadEnd();
	ParseAppList(szApp);
	return 0;
		//	CFunCommon::DlSprintf("nAppCount:%d,szApp:%s.\n",nAppCount,szApp);
	//	CFunCommon::DlSprintf("%s.\n",DOWN_SUCC);
}

/**
 * @brief	删除指定应用
 * @param[in] pszAppList 需要删除的一个月名称
 * @param[in] pszDev usb-serail号
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNLDDownload::DelAppList(const char* pszAppList,const char* pszDev)
{
	int nAppCount = 0;
	int nRet = 0;
	char szDevicePath[256] = {0};


	chdir(g_szAppPath);
	sprintf(szDevicePath,"/dev/%s",pszDev);
	//CFunCommon::DlSprintf("Device:%s\n",szDevicePath);
	if(!m_objUsbSerial.OpenPort(szDevicePath))
	{
		CFunCommon::DlSprintf("Failed to open  %s.\n",szDevicePath,strerror(errno));
		return false;
	}

	nRet = ShakeHand();	
	if (-1 == nRet)
	{
		CFunCommon::DlSprintf("Failed to shakehand:%s.\n",strerror(errno) );
		return false;	
	}

	char szTmp[1024*16] = {0};
	strcpy(szTmp,pszAppList);
	nAppCount = CFunCommon::ReplaceAll(szTmp,'|','*');
	nAppCount++;
//	CFunCommon::DlSprintf("nAppCount = %d,pszAppList=%s\n",nAppCount,pszAppList);
	if(!DelAppProcess(nAppCount,pszAppList))
	{
		CFunCommon::DlSprintf("Failed to del app list:%s.\n",strerror(errno) );
		return false;
	}
	DownloadEnd();
	CFunCommon::DlSprintf("Successful.\n");
	return true;
}

/**
 * @brief	删除指定应用 通讯
 * @param[in] nAppCount 需要删除的应用数量
 * @param[in] pszAppList 要删除的文件列表名
 * @return
 * @li true 成功
 * @li false 失败
*/
bool CNLDDownload::DelAppProcess(int nAppCount,const char* pszAppList)
{
	char szRecvBuff[1024*4] = {0};
	char szSendBuff[1024*4] = {0};
	unsigned char checksum = 0;
	U16 usStrLen = 0,usResult = 0;
	int nRet = 0;
	int nHeadLen = sizeof(MSG_HEAD);
	MSG_HEAD RecHead;
	MSG_HEAD SendHead;

	usStrLen=strlen(pszAppList);

	memset(&SendHead,0,sizeof(MSG_HEAD));
	memset(szSendBuff,0,sizeof(szSendBuff));
	
	SendHead.uMagic		=	0xAAAA;		/*报文魔术字*/
	SendHead.uSurport	=   1;			/**/
	SendHead.uCommand	=	POS_DELAPP_PART; /*握手命令字*/
	SendHead.uProVer	=	0x0001;		/*协议版本*/
	SendHead.uTotalNum	=	0x0001;		/*报文总数*/
	SendHead.uSeq		=	0x0001;		/*当前包序列号*/
	SendHead.uFieldCount=	nAppCount;			/*buff中包含几个TLV构成*/
	SendHead.uDataLen	=	usStrLen+1;			/*buff中的长度*/
	
	memcpy(szSendBuff,&SendHead, nHeadLen);
	memcpy(szSendBuff+nHeadLen,pszAppList,usStrLen);

	checksum = 0;
	for (int i = nHeadLen;i<nHeadLen+usStrLen;i++)
	{
		checksum += (unsigned char)szSendBuff[i];
	}
	szSendBuff[nHeadLen+usStrLen]=checksum;

	m_objUsbSerial.FlushPort();
	m_objUsbSerial.SendBuffData(szSendBuff, nHeadLen+usStrLen+1);
	


	memset(szRecvBuff,0x00,sizeof(szRecvBuff));
	nRet = m_objUsbSerial.ReadBuffData(szRecvBuff,nHeadLen,3000);
	if (nRet == nHeadLen)
	{
		memcpy(&RecHead,szRecvBuff,nHeadLen);
		nRet = m_objUsbSerial.ReadBuffData(szRecvBuff,RecHead.uDataLen,20);
		memcpy(&usResult,szRecvBuff,2);
	//	CFunCommon::DlSprintf("nResult = %d\n",nResult);
		if (0xff == usResult)
		{
			m_objUsbSerial.SendBuffData(szSendBuff, nHeadLen+usStrLen+1);
			return false;
		}
		else
		{
			return true;	
		}
			
	}

	return false;
}


