#include "arq.h"
#include "function.h"
#include "common.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <sys/time.h>
#include <errno.h>
using namespace std;



extern char g_szMessage[MSGSUM][100];
extern PFCALLBACK gCallBack ;


/**
 * @brief crc16校验数据表
*/
static const unsigned short crc16_table[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};


#define crc16_byte(crc, data) \
((crc >> 8) ^ crc16_table[(crc ^ data) & 0xff])

/**
 * @brief crc16校验
*/
unsigned short crc16(unsigned short crc, unsigned char *buffer, int len)
{
	if (buffer == NULL) return crc;
	while (len--) {
		crc = crc16_byte(crc, *buffer++);
	}
	return crc;
}




/**
 * @brief	重载构造函数
 * @param[in] bClear  	是否清空
 * @param[in] nRetry  	重发次数
*/
CARQ::CARQ(bool bClear,int nRetry)
{
	
	m_acknak = NULL;
	m_ucFSTX = FSTX;
	m_ucFDLE = FDLE;
	m_ucEs1 = 0x5E;
	m_ucEs2 = 0x5D;
	m_fd = -1;

	m_bClearApp = bClear;
	m_nRetry = nRetry;
	
	m_start = 0;
	m_syn = 1;
	m_in = NULL;
	m_out = NULL;

}

/**
 * @brief	默认析构函数
*/
CARQ::~CARQ()
{
	if (m_fd>0)
  	{
		close(m_fd);
  	}	
	if (m_in)
		delete m_in;

	if (m_out)
		delete m_out;

	if (m_acknak)
		delete m_acknak;
}



/**
 * @brief	清空usb-serail缓冲区
 * @return 	void
*/
void CARQ::FlushPort(void)
{
	tcflush(m_fd,TCIOFLUSH);
}

/**
 * @brief	关闭usb-serail
 * @return	void
*/
void CARQ::ClosePort(void)
{
	close(m_fd);	
	tcsetattr(m_fd,TCSANOW,&oldtio); 
}


/**
 * @brief	打开usb-serail设备
 * @param[in] pszDev  设备号
 * @return
 * @li  true		成功
 * @li  false		失败
 */
bool CARQ::OpenPort(const char* pszDev)
{
  	if (m_fd>0)
  	{
		close(m_fd);
  	}	
	if((m_fd=open(pszDev,O_RDWR | O_NOCTTY|O_NONBLOCK)) < 0)  
	{  
		CFunCommon::DlSprintf("Failed to opening %s.\n",pszDev);
		return false;       
	}
	fcntl(m_fd, F_SETFL, 0); //设置为阻塞模式
	tcgetattr(m_fd,&oldtio);
	bzero(&newtio,sizeof(newtio));
 
	newtio.c_cflag |= CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag |= CS8; 
	newtio.c_cflag &= ~PARENB;
	newtio.c_cflag &= ~CSTOPB; 
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  //Input,设定缓冲区有数据就返
	newtio.c_oflag &= ~OPOST;   //Output
	newtio.c_oflag &= ~(ONLCR | OCRNL);

	cfsetispeed(&newtio,B115200);
	cfsetospeed(&newtio,B115200); 
	newtio.c_iflag = IGNPAR;
	newtio.c_iflag &= ~ (IXON | IXOFF | IXANY |INLCR | ICRNL | IGNCR);
	newtio.c_cc[VMIN]=0;  
	newtio.c_cc[VTIME]=0;
	tcflush(m_fd,TCIFLUSH);
	tcsetattr(m_fd,TCSANOW,&newtio);
	return true;
  #if 0
  int position = maestroGetPosition(m_fd, 0);
  CFunCommon::DlSprintf("Current position is %d.\n", position); 
  int target = (position < 6000) ? 7000 : 5000;
  CFunCommon::DlSprintf("Setting target to %d (%d us).\n", target, target/4);
  maestroSetTarget(m_fd, 0, target);

  close(m_fd);
#endif

}



/**
 * @brief 初始化收发缓冲区
 * @param[in] nFlag:SENDBUFFER_16K/SENDBUFFER_4K
 * @return 	void
 */
void CARQ::InitBuffer(int nFlag)
{
	if (nFlag == SENDBUFFER_16K)
	{
		m_nFrameMaxSize = 1024*16;
		m_nFrameLen = FRAME_HOLD + m_nFrameMaxSize;
	}
	else
	{
		m_nFrameMaxSize = 1024*4;
		m_nFrameLen = FRAME_HOLD + m_nFrameMaxSize;
	}

	m_in = (frame_t *) new unsigned char [m_nFrameLen];
	m_out = (frame_t *) new unsigned char [m_nFrameLen];
	m_acknak = (frame_t *) new unsigned char [m_nFrameLen];
}


/**
 * @brief	scrop平台 发送数据帧
 * @param[in] fr    数据帧结构体
 * @param[in] nLen  数据长度
 * @return
 * @li 0 	成功
 * @li -1, 	失败
 */
int CARQ::SendFrame(frame_t* fr, int nLen)
{
	unsigned char* pszSend = (unsigned char* )fr;
	char szBuff[1024*64+1] = {0};
	register char *p = szBuff;
	*p++ = m_ucFSTX;
	
	for(int i=0; i<nLen; i++) 
	{
		register unsigned char ch = pszSend[i];
		if (ch == FSTX) 
		{
			*p++ = m_ucFDLE;
			*p++ = m_ucEs1;
		}
		else if (ch == FDLE) 
		{
			*p++ = m_ucFDLE;
			*p++ = m_ucEs2;
		}
		else 
		{
			*p++ = ch;
		}
	}
	
	*p++ = m_ucFSTX;

	//android 16k发送，write只能发送5120，采用分包发送.  scrop和cruxplus之前没分包，一次4k通讯正常，现全部改为分包发送
	int nTotalLen =  p-szBuff;  //总发送长度
	int nSendLen = 0;			//已发送
	int nPreLen = 1024;	//分包，一次每次发送 1k
	int nFlags = 0;
	fcntl(m_fd,F_SETFL,0);
	while(nTotalLen > 0)
	{
		int nFrameLen = 0;
		nFrameLen = nTotalLen>nPreLen?nPreLen:nTotalLen;
		int nRet = write(m_fd, (unsigned char *)szBuff+nSendLen, nFrameLen);
		if(nRet != nFrameLen)
		{
			CFunCommon::DlSprintf("Failed to write,errinfo:%s\n",strerror(errno));
			return -1;
		}
		// for (int  i = 0; i < nFrameLen; i++)
		// {
		// 	CFunCommon::DlSprintf("%02x ",(unsigned char)szBuff[nSendLen+i]);
		// }
		
		nSendLen += nRet;
		nTotalLen -= nRet;	
		//usleep(1000*10);  //非阻塞时使用
	}
		nFlags = fcntl(m_fd,F_GETFL,0);
		nFlags |= O_NONBLOCK;
		fcntl(m_fd,F_SETFL,nFlags);

	return 0;
}


/**
 * @brief	scrop平台 接收数据帧
 * @param[in] nTimeout    接收超时时间
 * @return 实际接收的数据长度
 */
int CARQ::RecvFrame(int nTimeout)
{
	unsigned char * pdata = (unsigned char *)m_in;
    unsigned char ucRecvdata;
    int flag = 0, nRet = -2, len = 0, escape = 0;
    /**
     * flag = 0 we haven't met FSTX, =1 frame end
     * escape = 0 normal character, = next one needs escaping
     */
  // CFunCommon::DlSprintf("recv:");
    do {
		if ((nRet = ReadByteWithTimeout(nTimeout)) < 0) {
			//	CFunCommon::DlSprintf("recv timeout\n");
                    break;
            }	
			ucRecvdata = nRet;
            if (ucRecvdata == FSTX) {
                    if (flag == 1) { /* frame end */
                            nRet = len;
                            /* if frame too short, it is invalid */
                            break;
                    }
                    if (flag == 2) { /* last frame data error */
                            flag = 0; /* we want first FSTX again */
                            continue;
                    }
                    flag = 1; /* see first FSTX */
            }
            else if (flag == 1) { /* frame data processing */
                    if (ucRecvdata == FDLE) {
                            escape = 1;
                            continue;
                    }
                    if (escape == 1) {
                            ucRecvdata = (unsigned char)((0x7D & 0xF0) | (ucRecvdata & 0x0F));
                            escape = 0;
                    }
                    if(len > (m_nFrameLen))/*over flow, abandon it*/ {
                            len = 0;
                            pdata = (unsigned char *)m_in;
                            flag = 2; /* until new FSTX */
                    }
                    else {
                            *pdata ++ = ucRecvdata;
                            len ++;
                    }

            }
            else  if (flag == 0) {
                    flag = 2; /* until new first FSTX */
            }
    }while (1);
    return nRet;
}




/**
 * @brief	scrop平台下载的结束帧
 * @param[in] pszData   待发送数据
 * @param[in] nLen  发送数据长度
 * @return
 * @li 0	成功 
 * @li -1	失败
 */
int CARQ::ARQSendMposEnd(unsigned char* pszData, int nLen)
{
	int nPackLen = 0;
	int nSendlen = 0 ;
	unsigned char * pdata = pszData;
	if (NULL == pszData  || nLen <= 0) 
		return 0;
	nSendlen = m_nFrameMaxSize;
	
	nSendlen = nLen > m_nFrameMaxSize ? m_nFrameMaxSize : nLen;
	nPackLen = PacketFrame(m_out, FDATA, pdata, nSendlen);	

	if(SendFrame(m_out, nPackLen) < 0 )
	{
		return -1;
	}	
	else
	{
		return 0;
	}	
}


/**
 * @brief	scrop平台 数据帧组包
 * @param[out] fr   返回组包后数据
 * @param[in] type  数据类型
 * @param[in] pszData  被封包数据
 * @param[in] nDatalen  被封包数据长度
 * @return	封包后数据长度 
 */
int CARQ::PacketFrame(frame_t* fr, frame_type_t type, unsigned char* pszData, int nDatalen)
{
	unsigned char * pframe = (unsigned char *)(fr);
	unsigned short crc;


	if (m_syn) {
		type |= FSYN;
		m_syn = 0;
	}
	fr->type = type;
	
	fr->seq_nr = m_start;

//	CFunCommon::DlSprintf("\nfr->type = %02x,fr->seq_nr = %02x\n",fr->type,fr->seq_nr);

	if (pszData && nDatalen) {
		memcpy(fr->data, pszData, nDatalen);
	}
	else {
		nDatalen = 0;
	}
	crc = crc16(0, pframe, nDatalen + FRAME_HEAD);
	*(pframe + nDatalen + FRAME_HEAD) = crc & 0x00FF;
	*(pframe + nDatalen + FRAME_HEAD + 1) = (crc >> 8) & 0x00FF;
	return (nDatalen + FRAME_HOLD);
}




/**
 * @brief	scrop平台发送数据接口
 * @param[in] pszData  待发送数据
 * @param[in] nDataLen  数据长度
 * @return 实际发送数据的长度
 
 */
int CARQ::ARQSend(unsigned char * pszData, int nDataLen)
{
	int nSendLen = 0; //发送的数据长度
	int nRetry = 0;	//重发次数
	int nPackLen = 0; //封包后的要发送数据长度	
	int nRecvLen = 0; 	//接收到数据长度
	int  nTotallen = nDataLen;	//实际发送的数据有效长度
	unsigned char * pdata = pszData;

	if (pszData == NULL || nDataLen <= 0) 
		return 0;

	nSendLen = m_nFrameMaxSize;
	while ((nDataLen >= 0)) 
	{

		nRetry = m_nRetry;
		nSendLen = nDataLen > m_nFrameMaxSize ? m_nFrameMaxSize : nDataLen;
		memset(m_out,0,sizeof(m_out));
		#if 0
		for (int n = 0; n < nSendLen;n++)
		{
			
			CFunCommon::DlSprintf("%02x ", pdata[n]);
		
		}
		#endif
		nPackLen = PacketFrame(m_out, FDATA, pdata, nSendLen);
	#if 0
		CFunCommon::DlSprintf("\n---------------------------------------\n");
		for (int n = 0; n < nPackLen;n++)
		{
		
			CFunCommon::DlSprintf("%02x ", m_out->data[n]);
			
		
		}
		CFunCommon::DlSprintf("\n-------------%d:%d------------------\n",nSendLen, nPackLen);
	#endif
		do {
			if (SendFrame(m_out, nPackLen) < 0) 
			{
				CFunCommon::DlSprintf("Failed to SendFrame.\n");
				return -2;
			}			
recv:		
		//	CFunCommon::DlSprintf("begin recv\n");
			if ((nRecvLen = RecvFrame(500)) <= 0) 
			{
				//CFunCommon::DlSprintf("continue\n");
				continue;
			}
			//CFunCommon::DlSprintf("\n");
			if (CheckFcsFrame((unsigned char *)(m_in), nRecvLen)) {
				if (m_in->seq_nr == m_start) {
					if (IS_FACK(m_in)) {
						break;
					}
					if (IS_FNAK(m_in)) {
						continue;
					}
					goto recv;
				}
				else if (IS_FDATA(m_in) &&
					(IS_PREVIOUS_SEQ(m_start, m_in->seq_nr))) {
					m_start --;
					nPackLen = PacketFrame(m_acknak, FACK, NULL, 0);
					m_start ++;
					if (SendFrame(m_acknak, nPackLen) < 0) {
						return -4;
					}
					goto recv;
				}
				else {
					goto recv;
				}
			}
			else {
			}	
		}while ((--nRetry > 0));

		if (0 == nRetry) 
		{
			CFunCommon::DlSprintf("Failed: exceed retry times\n");
			return -3;
		}
		m_start ++;
		pdata += nSendLen;
		nDataLen -= m_nFrameMaxSize;
	}
    return nTotallen;
}

/**
 * @brief	读usb-serial一个字节数据
 * @param[in] nLimit  超时时间ms
 * @return  读取得1byte数据
 */
int CARQ::ReadByteWithTimeout(int nLimit)
{
	int nHaveRead = 0;
	int nRead = 0;
	unsigned char cc;
	for(int i=0; i<nLimit; i++)
	{	
		nRead = 0;
		nRead = read(m_fd,&cc,1);
		if(1 == nRead)
 	 	{
			//CFunCommon::DlSprintf("cc = %02x\n",cc);
			return cc ;		
 		}
		else if (0 == nRead)
		{
			usleep(1000);
		}
		else if (nRead < 0)
		{
			return -1;
		}			
	}
	return -1;
	
}


/**
 * @brief	scrop平台校验接收的数据
 * @param[in] frame  待校验的数据
 * @param[in] nLen  	  数据长度
 * @return  
 * @li 1	成功
 * @li 0	失败
 */
int CARQ::CheckFcsFrame(unsigned char* frame, int nLen)
{
	unsigned short crc_req, crc_cal;
	nLen -= 2;
	crc_cal = crc16(0, frame, nLen);
	crc_req = (*(frame + nLen)) | ((*(frame + nLen + 1)) << 8);
	return (crc_cal == crc_req);
}


/**
 * @brief	scrop平台 读指定长度数据
 * @param[out] pszOut  读出的数据
 * @param[in] nDataLen  需要读取的数据长度
 * @param[in] nTimeout  读取超时时间
 * @return 实际读取的数据大小，<0读取失败
 */
int CARQ::ARQRecv(unsigned char * pszOut, int nDataLen, int nTimeout)
{
	int  nLen = 0, nDlen = 0;
	int nRecvLen = 0; //读取的数据长度
	int nPacken = 0; //封包后数据长度
	seq_nr_t old;

	while (1) 
	{
		if ((nRecvLen = RecvFrame(nTimeout)) <= 0) 
		{
			return nRecvLen;
		}

		if (CheckFcsFrame((unsigned char *)(m_in), nRecvLen)) 
		{
			if (IS_FDATA(m_in)) 
			{
				if (m_in->type & FSYN) m_start = m_in->seq_nr;

				if (m_in->seq_nr == m_start) 
				{
					nPacken = PacketFrame(m_acknak, FACK, NULL, 0);

					if (SendFrame(m_acknak, nPacken) < 0) 
					{
						return -4;
					}

					m_start ++;
					nRecvLen -= FRAME_HOLD;

					if (nDataLen) 
					{
						nDlen = nRecvLen > nDataLen ? nDataLen : nRecvLen;
						memcpy(pszOut + nLen, m_in->data, nDlen);
						nDataLen -= nDlen;
					}

					nLen += nRecvLen;

					if (nRecvLen < m_nFrameMaxSize) 
					{
						return nLen;
					}
				}
				else if (IS_PREVIOUS_SEQ(m_start, m_in->seq_nr)) 
				{
					m_start --;
					nPacken = PacketFrame(m_acknak, FACK, NULL, 0);
					m_start ++;

					if (SendFrame(m_acknak, nPacken) < 0) 
					{
						return -4;
					}
				}
				
			}
		}
		else 
		{
			old = m_start;
			m_start = m_in->seq_nr;
			nPacken = PacketFrame(m_acknak, FNAK, NULL, 0);
			m_start = old;

			if (SendFrame(m_acknak, nPacken) < 0) 
			{
				return -2;
			}
		}
	}

	return 0;
}






/**
 * @brief	scrop平台 固件下载
 * @param[in] pszFileList  下载的固件文件列表
 * @return
 * @li  0 	下载成功
 * @li < 0	下载失败
 */
int CARQ::Scrop_DownLoad_Firm(const char* pszFileList)
{
	char szTotal[4096] = {0};						
	char szDownFullPath[MAX_DOWNFILE_NUM][MAX_PATH] = {0} ;
	char szDownFileName[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char szTmp[4096] = {0};
	int szFileType[MAX_DOWNFILE_NUM],			//文件类型
		nTotalFileCount = 0,		//文件数量
		nDownFileLen = 0,			//下载的文件长�??
		nLen = 0,
		nPos = 0,
		nReadLen = 0,
		nCrtLen = 0,
		i,j,a,nOffset,nRet,nProgress=0;

	char szBuff[1024*64 + 1] = {0};
	float fProgress=0.0f;
	unsigned char ucFileNum;
	unsigned char cmd ;

	char szT[256] = {0};
	FILE *f1 = NULL;


	memset(szTotal,0,sizeof(szTotal));
	memset(szBuff,0,sizeof(szBuff));
	
	strcpy(szBuff,pszFileList);
	nDownFileLen=strlen(szBuff);
	nTotalFileCount=0;
	nOffset=0;

	//解析文件列表，取出各�??文件全路径与文件名，文件数量
	for (i=0;i<nDownFileLen;i++)  
	{
		if (szBuff[i]=='*')
		{
			szBuff[i]='\0';
			strcpy(szDownFullPath[nTotalFileCount],szBuff+nOffset);
			szBuff[i]='*';

			for (j=strlen(szDownFullPath[nTotalFileCount])-1;j>0;j--)
			{
				if (szDownFullPath[nTotalFileCount][j]=='/')
				{
					strcpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1);//拷贝文件名，不包�??�??�??
					break;
				}
			}

			for (j=i;;j++)
			{
				if (szBuff[j]=='|')
					break;
			}

			memset(szTmp,0,sizeof(szTmp));
			memcpy(szTmp,szBuff+i+1,j-i-1);
			szFileType[nTotalFileCount]=atoi(szTmp);

			nOffset=j+1;			
			nTotalFileCount++;
		}
	}


	//下载
	//CFunCommon::DlSprintf("nTotalFileCount = %d,szFileType[]=%d,szDownFullPath=%s\n",nTotalFileCount,szFileType[0],szDownFullPath[0]);
	for (i=0; i<nTotalFileCount; i++)
	{		
		
		if ((0 == szFileType[i] )|| (1 == szFileType[i]) || (2 == szFileType[i]))  
		{

			//CFunCommon::DlSprintf("Downloading %s\n",szDownFullPath[i]);
		
			//NLP Down
			if ((f1 = fopen(szDownFullPath[i], "rb")) == NULL) 
			{
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -2;
			}
			memset(szTotal,0,sizeof(szTotal));

			if (szFileType[i]==1) 
				nLen = 456 + 256;
			else if (szFileType[i]==0)
				nLen = 256;		
			fseek(f1, 13,SEEK_SET );
			fread(&ucFileNum, 1, 1, f1);
			memset(szTmp,0,sizeof(szTmp));
			fread(szTmp, 1, 2, f1);
	
			for (j = 0; j < ucFileNum; j ++) 
			{
				if (szFileType[i]==2)
				{
					szTotal[0] = 7;//x509证书
					fread(szTotal+1, 1, 4, f1);
					memcpy(&nCrtLen,szTotal+1,4);
					fread(szTotal+5, 1, nCrtLen+256, f1);
					nLen=nCrtLen+256+4;					
					fread(&nDownFileLen, 1, 4, f1);
				}
				else
				{
				
					
					szTotal[0] = 1;
					fread(szTotal + 1, 1, nLen, f1);
					fread(&nDownFileLen, 1, 4, f1);
				}

				nPos=0;
			
			//	FlushPort();
				
				nRet=ARQSend((unsigned char *)szTotal, nLen + 1);
				
				if ( nRet < 0) 
				{
					CFunCommon::DlSprintf("Failed:DOWN_ERR_SENDERROR\n ");  
					nRet=DOWN_ERR_SENDERROR;
					fclose(f1);
					return -1;
				}
				
				nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);		
				if ( nRet < 0 || 0 != szTotal[0]) 
				{				
					if(nRet < 0)
						CFunCommon::DlSprintf("Failed:recv timeout");
					else
						CFunCommon::DlSprintf("Failed:ErrorInfo:%s\n ",g_szMessage[szTotal[0] + 86] );
					fclose(f1);
					return -1;
				}
				else 
				{				
					nPos=0;
					nReadLen = nDownFileLen;
					while (nDownFileLen) 
					{
						nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;
						memset(szTotal,0,sizeof(szTotal));
						a=fread(szTotal,1,nReadLen, f1);
						if(a != nReadLen)
						{
							//CFunCommon::DlSprintf("read ret:%d,pos:%d,error:%s\n",a,ftell(f1),strerror(errno));
						}
							
						FlushPort();
						nRet=ARQSend((unsigned char *)szTotal, nReadLen);
						if ( nRet< 0) 
						{
							CFunCommon::DlSprintf("Failed:DOWN_ERR_SENDERROR error\n ");
							fclose(f1);
							return -1;
						}

						nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);
						if (nRet < 0 || 0 != szTotal[0] ) 
						{			
							if(nRet < 0)
								CFunCommon::DlSprintf("Failed:recv timeout");
							else
							CFunCommon::DlSprintf("Failed:ErrorInfo:%s\n ",g_szMessage[szTotal[0] + 86] );
					
							fclose(f1);
							return -1;
						}

						nDownFileLen -= nReadLen;
						nPos+=nReadLen;
						fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
						nProgress=fProgress;
						(*gCallBack)(szDownFullPath[i],nProgress);
					//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
						usleep(10);
					}	
				//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);
					(*gCallBack)(szDownFullPath[i],100);
					nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) ;
					if (nRet < 0 || 0 != szTotal[0] ) 
					{			
							if(nRet < 0)
								CFunCommon::DlSprintf("Failed:recv timeout");
							else
								CFunCommon::DlSprintf("Failed:ErrorInfo:%s\n ",g_szMessage[szTotal[0] + 86] );
							
							fclose(f1);
							return -3;
					}
				}
			}
			fclose(f1);
		}
		else if (8 == szFileType[i])
		{
		//	CFunCommon::DlSprintf("Downloading %s\n",szDownFullPath[i]);
		    if (NULL == (f1 = fopen(szDownFullPath[i], "rb")) ) 
			{
				
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -1;
			}

			memset(szTotal,0,sizeof(szTotal));
			szTotal[0] = 2;
			fseek(f1, 0L, SEEK_END);  
    		nDownFileLen = ftell(f1); 
			fseek(f1, 0L, SEEK_SET);  
			memcpy(szTotal + 1, &nDownFileLen, 4);
		
			strcpy((char *)szTotal + 5, szDownFileName[i]);
			nLen = strlen((char *)szTotal + 5) + 1 + 4 + 1;
			if (ARQSend((unsigned char *)szTotal, nLen) < 0) 
			{
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_SENDERROR]);
				fclose(f1);
				return -1;
			}

			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
			{
		
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_RECVERROR]);
				fclose(f1);
				return -1;
			}
			if (szTotal[0] != 0) 
			{
			
				a = szTotal[0] + 86;
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[a]);
				fclose(f1);
				return -2;
			}

			nPos=0;
			nReadLen = nDownFileLen;
			while (nDownFileLen) 
			{
				nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;
				fread(szTotal, 1, nReadLen, f1);
				if (ARQSend((unsigned char *)szTotal, nReadLen) < 0) 
				{
					CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_SENDERROR]);
					fclose(f1);
					return -4;					
				}

				if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
				{
					CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_RECVERROR]);
					fclose(f1);
					return -1;					
				}

				if (szTotal[0] != 0) 
				{
					a = szTotal[0] + 86;
					CFunCommon::DlSprintf("Failed:%s",g_szMessage[a]);
					fclose(f1);
					return -1;
				}

				nDownFileLen -= nReadLen;
				nPos += nReadLen;
				//20160322�??改进度条显示不完整的�??�??
				fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
				nProgress=fProgress;

			//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
			(*gCallBack)(szDownFullPath[i],nProgress);
				usleep(30);
				
			}
		//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);	
		(*gCallBack)(szDownFullPath[i],100);
			//CFunCommon::DlSprintf("Checking Data ......\n");
		
			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
			{
			
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_RECVERROR]);
				fclose(f1);
				return -1;
			}

			if (szTotal[0] != 0) 
			{
		
				a = szTotal[0] + 86;
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[a]);
				fclose(f1);
				return -1;
			}
			else 
			{
				fclose(f1);
			}
		}
	}
	
	memset(szTotal,0,sizeof(szTotal));
	szTotal[0] = 3;

	time_t unix_tm;
	time( &unix_tm );
	unix_tm = unix_tm + 8*3600; 
    
	memcpy(szTotal + 1, &unix_tm, sizeof (unix_tm));
	if (ARQSendMposEnd((unsigned char *)szTotal, 1 + sizeof (unix_tm)) < 0) 
	{
			return -4;
	}
	return 0;
}


/**
 * @brief	scrop平台 应用下载
 * @param[in] pszFileList  下载的应用文件列表
 * @return
 * @li 0 成功
 * @li <0 失败
 */
int CARQ::Scrop_DownLoad_APP(const char* pszFileList)
{
	char szTotal[4096] = {0};						
	char szDownFullPath[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char szDownFileName[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char szTmp[4096] = {0};
	int szFileType[MAX_DOWNFILE_NUM],			//文件类型
		nTotalFileCount = 0,						//文件数量
		nDownFileLen = 0,			
		nLen = 0,
		nPos = 0,
		nReadLen =0 ,
		nCrtLen = 0,
		i = 0,j = 0 ,a = 0,nOffset = 0,nRet = 0,nProgress=0;

	char szBuff[1024*64 + 1] = {0};
	float fProgress=0.0f;
	unsigned char nFileNum;
	unsigned char cmd ;

	char szT[256]={0};
	FILE *f1 = NULL;


	memset(szTotal,0,sizeof(szTotal));
	memset(szBuff,0,sizeof(szBuff));
	
	strcpy(szBuff,pszFileList);
	nDownFileLen=strlen(szBuff);
	nTotalFileCount=0;
	nOffset=0;
	//m_start = 0;

	//清空应用
	if (m_bClearApp)
	{
		cmd = cmd_scrop_clearapp;
		FlushPort();
		if (ARQSend(&cmd, 1) < 0) 
		{
			CFunCommon::DlSprintf("Failed to send1 \n");
			return -4;		
		}	
		nRet=ARQRecv((unsigned char *)szTmp, sizeof (szTmp), 1000*5);
		if ( nRet < 0 || 0 != szTmp[0]) 
		{			
			CFunCommon::DlSprintf("Failed:icmd_clear app error.\n");
			return -1;
		}
	}

	//解析文件列表，取出各�??文件全路径与文件名，文件数量
	for (i=0;i<nDownFileLen;i++)  
	{
		if (szBuff[i]=='*')
		{
			szBuff[i]='\0';
			strcpy(szDownFullPath[nTotalFileCount],szBuff+nOffset);
			szBuff[i]='*';

			for (j=strlen(szDownFullPath[nTotalFileCount])-1;j>0;j--)
			{
				if (szDownFullPath[nTotalFileCount][j]=='/')
				{
					strcpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1);//拷贝文件名，不包�??�??�??
					break;
				}
			}

			for (j=i;;j++)
			{
				if (szBuff[j]=='|')
					break;
			}

			memset(szTmp,0,sizeof(szTmp));
			memcpy(szTmp,szBuff+i+1,j-i-1);
			szFileType[nTotalFileCount]=atoi(szTmp);

			nOffset=j+1;			
			nTotalFileCount++;
		}
	}


	//下载
	//CFunCommon::DlSprintf("nTotalFileCount = %d,szFileType[]=%d,szDownFullPath=%s\n",nTotalFileCount,szFileType[0],szDownFullPath[0]);
	for (i=0; i<nTotalFileCount; i++)
	{		
		
		if ((0 == szFileType[i] )|| (1 == szFileType[i]) || (2 == szFileType[i]))  
		{

		//	CFunCommon::DlSprintf("Downloading %s\n",szDownFullPath[i]);
		
			//NLP Down
			if ((f1 = fopen(szDownFullPath[i], "rb")) == NULL) 
			{
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -2;
			}
			memset(szTotal,0,sizeof(szTotal));

			if (szFileType[i]==1) 
				nLen = 456 + 256;
			else if (szFileType[i]==0)
				nLen = 256;		
			fseek(f1, 13,SEEK_SET );
			fread(&nFileNum, 1, 1, f1);
			memset(szTmp,0,sizeof(szTmp));
			fread(szTmp, 1, 2, f1);
	
			for (j = 0; j < nFileNum; j ++) 
			{
				if (szFileType[i]==2)
				{
					szTotal[0] = 7;//x509证书
					fread(szTotal+1, 1, 4, f1);
					memcpy(&nCrtLen,szTotal+1,4);
					fread(szTotal+5, 1, nCrtLen+256, f1);
					nLen=nCrtLen+256+4;					
					fread(&nDownFileLen, 1, 4, f1);
				}
				else
				{
				
					
					szTotal[0] = 1;
					fread(szTotal + 1, 1, nLen, f1);
					fread(&nDownFileLen, 1, 4, f1);
				}

				nPos=0;
			

				
				nRet=ARQSend((unsigned char *)szTotal, nLen + 1);
				
				if ( nRet < 0) 
				{
					CFunCommon::DlSprintf("Failed:DOWN_ERR_SENDERROR\n ");  
					nRet=DOWN_ERR_SENDERROR;
					fclose(f1);
					return -1;
				}
				
				nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);		
				if ( nRet < 0 || 0 != szTotal[0]) 
				{				
					if(nRet < 0)
							CFunCommon::DlSprintf("Failed:recv timeout");
					else
							CFunCommon::DlSprintf("Failed:ErrorInfo:%s\n ",g_szMessage[szTotal[0] + 86] );
					fclose(f1);
					return -1;
				}
				else 
				{				
					nPos=0;
					nReadLen = nDownFileLen;
					while (nDownFileLen) 
					{
						nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;
						memset(szTotal,0,sizeof(szTotal));
						a=fread(szTotal,1,nReadLen, f1);
						if(a != nReadLen)
							//CFunCommon::DlSprintf("read ret:%d,pos:%d,error:%s\n",a,ftell(f1),strerror(errno));
						FlushPort();
						nRet=ARQSend((unsigned char *)szTotal, nReadLen);
						if ( nRet< 0) 
						{
							CFunCommon::DlSprintf("Failed:DOWN_ERR_SENDERROR error 33\n ");
							fclose(f1);
							return -1;
						}

						nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);
						if (nRet < 0 || 0 != szTotal[0] ) 
						{			
							if(nRet < 0)
								CFunCommon::DlSprintf("Failed:recv timeout");
							else
								CFunCommon::DlSprintf("Failed:ErrorInfo:%s\n ",g_szMessage[szTotal[0] + 86] );
							fclose(f1);
							return -1;
						}

						nDownFileLen -= nReadLen;
						nPos+=nReadLen;
						fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
						nProgress=fProgress;
						  
					//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
					(*gCallBack)(szDownFullPath[i],nProgress);
						usleep(10);
					}	
				//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);
				(*gCallBack)(szDownFullPath[i],100);
					
					nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) ;
					if (nRet < 0 || 0 != szTotal[0] ) 
					{			
							if(nRet < 0)
								CFunCommon::DlSprintf("Failed:recv timeout");
							else
								CFunCommon::DlSprintf("Failed:ErrorInfo:%s\n ",g_szMessage[szTotal[0] + 86] );
							
							fclose(f1);
							return -3;
					}
				}
			}
			fclose(f1);
		}
		else if (8 == szFileType[i])
		{
			CFunCommon::DlSprintf("Failed:Downloading %s\n",szDownFullPath[i]);
		    if (NULL == (f1 = fopen(szDownFullPath[i], "rb")) ) 
			{
				
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -1;
			}

			memset(szTotal,0,sizeof(szTotal));
			szTotal[0] = 2;
			fseek(f1, 0L, SEEK_END);  
    		nDownFileLen = ftell(f1); 
			fseek(f1, 0L, SEEK_SET);  
			memcpy(szTotal + 1, &nDownFileLen, 4);
		
			strcpy((char *)szTotal + 5, szDownFileName[i]);
			nLen = strlen((char *)szTotal + 5) + 1 + 4 + 1;
			if (ARQSend((unsigned char *)szTotal, nLen) < 0) 
			{
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_SENDERROR]);
				fclose(f1);
				return -1;
			}

			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
			{
		
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_RECVERROR]);
				fclose(f1);
				return -1;
			}
			if (szTotal[0] != 0) 
			{
			
				a = szTotal[0] + 86;
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[a]);
				fclose(f1);
				return -2;
			}

			nPos=0;
			nReadLen = nDownFileLen;
			while (nDownFileLen) 
			{
				nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;
				fread(szTotal, 1, nReadLen, f1);
				if (ARQSend((unsigned char *)szTotal, nReadLen) < 0) 
				{
					CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_SENDERROR]);
					fclose(f1);
					return -4;					
				}

				if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
				{
					CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_RECVERROR]);
					fclose(f1);
					return -1;					
				}

				if (szTotal[0] != 0) 
				{
					a = szTotal[0] + 86;
					CFunCommon::DlSprintf("Failed:%s",g_szMessage[a]);
					fclose(f1);
					return -1;
				}

				nDownFileLen -= nReadLen;
				nPos += nReadLen;
				//20160322�??改进度条显示不完整的�??�??
				fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
				nProgress=fProgress;

			//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
			(*gCallBack)(szDownFullPath[i],nProgress);
				usleep(30*1000);
				
			}
			//CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);	
			(*gCallBack)(szDownFullPath[i],100);
			//CFunCommon::DlSprintf("Checking Data ......\n");
		
			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
			{
			
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_RECVERROR]);
				fclose(f1);
				return -1;
			}

			if (szTotal[0] != 0) 
			{
		
				a = szTotal[0] + 86;
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[a]);
				fclose(f1);
				return -1;
			}
			else 
			{
				fclose(f1);
			}
		}
	}
	
	memset(szTotal,0,sizeof(szTotal));
	szTotal[0] = 3;

	time_t unix_tm;
	time( &unix_tm );
	unix_tm = unix_tm + 8*3600; 
    
	memcpy(szTotal + 1, &unix_tm, sizeof (unix_tm));
	if (ARQSendMposEnd((unsigned char *)szTotal, 1 + sizeof (unix_tm)) < 0) 
	{
			return -4;
	}

	return 0;
}

#if 0

//GB2312到UTF-8的转�??




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////






void CARQ::SetClearApp(BOOL clearApp)
{
	m_bClearApp = clearApp;
}








//用于16K版本的下�??
int CARQ::NLP_DownLoad(const char *pcFileList)		//传入参数下载文件列表
{
	char szTotal[4096*4],			//总的处理数组
		szDownFullPath[50][256],//下载单个文件的路�??
		szDownFileName[50][256],//下载单个文件的名称，不包�??�??�??
		szTmp[10];
	int szFileType[50],			//文件类型
		nTotalFileCount,		//文件数量
		nDownFileLen,			//下载的文件长�??
		nLen,
		nPos,
		nReadLen,
		nCrtLen,
		i,j,nOffset,nRet,nProgress=0;
	float fProgress=0.0f;
	unsigned char nFileNum;
	CFile f1;
	//解析列表

	memset(szTotal,0,sizeof(szTotal));

	strcpy(szTotal,pcFileList);
	nDownFileLen=strlen(szTotal);
	nTotalFileCount=0;
	nOffset=0;
	
	for (i=0;i<nDownFileLen;i++)  //解析出文件的全路�??,并�?�算文件的个�??
	{
		if (szTotal[i]=='*')
		{
			szTotal[i]='\0';
			strcpy(szDownFullPath[nTotalFileCount],szTotal+nOffset);
			szTotal[i]='*';
			
			for (j=strlen(szDownFullPath[nTotalFileCount])-1;j>0;j--)
			{
				if (szDownFullPath[nTotalFileCount][j]=='\\')
				{
					strcpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1);//拷贝文件名，不包�??�??�??
					break;
				}
			}
			
			for (j=i;;j++)
			{
				if (szTotal[j]=='|')
					break;
			}
			
			memset(szTmp,0,sizeof(szTmp));
			memcpy(szTmp,szTotal+i+1,j-i-1);
			szFileType[nTotalFileCount]=atoi(szTmp);
			
			nOffset=j+1;			
			nTotalFileCount++;
		}
	}
	//下载

	for (i=0;i<nTotalFileCount;i++)
	{
		if ((szFileType[i]==0) || (szFileType[i]==1) || (szFileType[i]==2))  //NLP文件的包�??证书的不同格�??
		{
			//NLP Down
			f1.Open(szDownFullPath[i],CFile::modeRead|CFile::shareDenyNone);

			memset(szTotal,0,sizeof(szTotal));
			
			if (szFileType[i]==1) 
				nLen = 312 + 256;
			else if (szFileType[i]==0)
				nLen = 256;
			
			f1.Seek(13, CFile::begin);
			f1.Read(&nFileNum, 1);
			f1.Seek(2, CFile::current);
			
			for (j = 0; j < nFileNum; j ++) 
			{
				if (szFileType[i]==2)
				{
					szTotal[0] = 7;//x509证书
					f1.Read(szTotal+1,4);
					memcpy(&nCrtLen,szTotal+1,4);
					f1.Read(szTotal+5,nCrtLen+256);
					nLen=nCrtLen+256+4;
					
					f1.Read(&nDownFileLen, 4);
				}
				else
				{
					szTotal[0] = 1;
					f1.Read(szTotal + 1, nLen);
					f1.Read(&nDownFileLen, 4);
				}

				nPos=0;

				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_MSG_DOWNING); 

				nRet=ARQSend((unsigned char *)szTotal, nLen + 1);

				if ( nRet< 0) 
				{
					nRet=DOWN_ERR_SENDERROR;
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
					f1.Close();
					return nRet;
				}
				
				nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), 5000);
				if ( nRet< 0) 
				{					
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
					f1.Close();
					return nRet;
				}
				
				if (szTotal[0] != 0) 
				{					
					PostMessageA(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_PROCESSERROR); //数据处理错�??
					f1.Close();
					return -3;
				}
				else 
				{				
					nPos=0;
					nReadLen = nDownFileLen;
					while (nDownFileLen) 
					{
						nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;
						nRet=f1.Read(szTotal,nReadLen);

						nRet=ARQSend((unsigned char *)szTotal, nReadLen);

						if ( nRet< 0) 
						{
							PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
							f1.Close();
							return nRet;
						}

						nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), 5000);
						if (nRet < 0) 
						{							
							PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
							f1.Close();
							return nRet;
						}
						
						if (szTotal[0] != 0) 
						{							
							PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_PROCESSERROR); //数据下载处理错�??
							f1.Close();
							return -3;
						}
						
						nDownFileLen -= nReadLen;
						nPos+=nReadLen;
						//20160322�??改进度条显示不完整的�??�??
						fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
						nProgress=fProgress;
						::PostMessage(m_hTermWnd, WM_PROGRESS, nThreadId, nProgress);	// 发送进度给调用窗体
					}					
					
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_CHECKING); //校验

					nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), 5000) ;
					if (nRet< 0) 
					{						
						PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
						f1.Close();
						return nRet;
					}
					
					if (szTotal[0] != 0) 
					{						
						PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_PROCESSERROR); //数据处理错�??
						f1.Close();
						return -3;
					}
					else
					{
						f1.Close();
					}
				}
			}
		}
		else if (szFileType[i]==8)
		{
			f1.Open(szDownFullPath[i],CFile::modeRead|CFile::shareDenyNone);
			memset(szTotal,0,sizeof(szTotal));

			szTotal[0] = 2;
			nDownFileLen= f1.GetLength();
			memcpy(szTotal + 1, &nDownFileLen, 4);

			int testLen = strlen(szDownFileName[i]);
			char *pName_Convert = NULL;
			pName_Convert=Gb232TUtf8(szDownFileName[i],&testLen);
			strcpy((char *)szTotal + 5, pName_Convert);
		//	strcpy((char *)szTotal + 5, szDownFileName[i]);
			nLen = strlen((char *)szTotal + 5) + 1 + 4 + 1;

			PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_MSG_DOWNING); 

			if (ARQSend((unsigned char *)szTotal, nLen) < 0) 
			{
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
				f1.Close();
				return -4;
			}

			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), 5000) < 0) 
			{
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
				f1.Close();
				return -2;
			}
			if (szTotal[0] != 0) 
			{
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_PROCESSERROR); //数据处理错�??
				f1.Close();
				return -3;
			}

			nPos=0;
			nReadLen = nDownFileLen;
			while (nDownFileLen) 
			{
				nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;

				f1.Read(szTotal, nReadLen);
				if (ARQSend((unsigned char *)szTotal, nReadLen) < 0) 
				{
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
					f1.Close();
					return -4;					
				}

				if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), 5000) < 0) 
				{
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
					f1.Close();
					return -2;					
				}

				if (szTotal[0] != 0) 
				{
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据下载处理错�??
					return -3;
				}

				nDownFileLen -= nReadLen;
				nPos+=nReadLen;
				//20160322�??改进度条显示不完整的�??�??
				fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
				nProgress=fProgress;
				::PostMessage(m_hTermWnd, WM_PROGRESS, nThreadId, nProgress);	// 发送进度给调用窗体
			}

			PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_CHECKING); //校验...

			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), 5000) < 0) 
			{
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
				f1.Close();
				return -2;
			}

			if (szTotal[0] != 0) 
			{
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_PROCESSERROR); //数据处理失败
				f1.Close();
				return -3;
			}
			else 
			{
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_MSG_SUCCESS); 
				f1.Close();
			}
		}
	}

	//若无�??下载的包（OTA），发�?01，用于与正常结束区分
	if (nTotalFileCount == 0)
	{

		unsigned char cmd = 0x01;
		if (ARQSend(&cmd, 1) < 0) 
		{
			return -4;		
		}
		else
		{
			return 0;
		}
	}

	//清空应用
	if (m_bClearApp)
	{
		unsigned char cmd = ANDROID_CLEAR;
		if (ARQSend(&cmd, 1) < 0) 
		{
			return -4;		
		}
		else
		{
			return 0;
		}
	}
	memset(szTotal,0,sizeof(szTotal));
	szTotal[0]=3;
	 time_t unix_tm;
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	FILETIME ft;
    SystemTimeToFileTime( &st, &ft );
    LONGLONG nLL;
    ULARGE_INTEGER ui;
    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;
    nLL = (ft.dwHighDateTime << 32) + ft.dwLowDateTime;
    unix_tm = (long)((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
	memcpy(szTotal + 1, &unix_tm, sizeof (unix_tm));

	if (ARQSend((unsigned char *)szTotal, 1 + sizeof (unix_tm)) < 0) 
	{
		return -4;
	}

	return 0;
}


//用于4K版本的下�??
int CARQ::NLP_DownLoad_2(const char *pcFileList)		//传入参数下载文件列表
{
	char szTotal[4096],			//总的处理数组
		szDownFullPath[50][256],//下载单个文件的路�??
		szDownFileName[50][256],//下载单个文件的名称，不包�??�??�??
		szTmp[100];
	int szFileType[50],			//文件类型
		nTotalFileCount,		//文件数量
		nDownFileLen,			//下载的文件长�??
		nLen,
		nPos,
		nReadLen,
		nCrtLen,
		i,j,nOffset,nRet,nProgress=0,a;
	float fProgress=0.0f;
	unsigned char nFileNum;
	CFile f1;
	unsigned char cmd ;
	CString msg;
	//解析列表
	BOOL bNlpDebug = FALSE;  //NLP 调试版本
	BOOL bAppInfo = FALSE;//NLP appinfo版本
	memset(szTotal,0,sizeof(szTotal));

	strcpy(szTotal,pcFileList);
	nDownFileLen=strlen(szTotal);
	nTotalFileCount=0;
	nOffset=0;
	OutputDebugString(pcFileList);
	if (m_MPosDown)
	{
		if (m_bClearApp)
		{
			 cmd = MPOS_CLEAR;
			if (ARQSend(&cmd, 1) < 0) 
			{
				OutputDebugString3("send 1");
				return -4;		
			}
			nRet=ARQRecv((unsigned char *)szTmp, sizeof (szTmp), NLP_FIRM_TIMEOUT*2);
			if ( nRet< 0) 
			{			
				
				OutputDebugString3("recv 1");
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
				return nRet;
			}
			if (szTmp[0] != 0) 
			{			
				a = szTmp[0] + 86;
				OutputDebugString3("data error 1");
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, a); //数据处理错�??
				return -3;
			}
		}
	}
	for (i=0;i<nDownFileLen;i++)  //解析出文件的全路�??,并�?�算文件的个�??
	{
		if (szTotal[i]=='*')
		{
			szTotal[i]='\0';
			strcpy(szDownFullPath[nTotalFileCount],szTotal+nOffset);
			szTotal[i]='*';
			
			for (j=strlen(szDownFullPath[nTotalFileCount])-1;j>0;j--)
			{
				if (szDownFullPath[nTotalFileCount][j]=='\\')
				{
					strcpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1);//拷贝文件名，不包�??�??�??
					break;
				}
			}
			
			for (j=i;;j++)
			{
				if (szTotal[j]=='|')
					break;
			}
			
			memset(szTmp,0,sizeof(szTmp));
			memcpy(szTmp,szTotal+i+1,j-i-1);
			szFileType[nTotalFileCount]=atoi(szTmp);
			
			nOffset=j+1;			
			nTotalFileCount++;
		}
	}
	//下载
#if 0
	cmd =0x00;
	nRet=ARQSend(&cmd, 1);
	if ( nRet< 0) 
	{
		OutputDebugString3("send 22");
		nRet=DOWN_ERR_SENDERROR;
		PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
		f1.Close();
		return nRet;
	}

	nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);
	if ( nRet< 0) 
	{			
		OutputDebugString3("recv 2");
		PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
		f1.Close();
		return nRet;
	}
	OutputDebugString3((char*)szTotal+1);
#endif
	for (i=0;i<nTotalFileCount;i++)
	{
		if ((szFileType[i]==0) || (szFileType[i]==1) || (szFileType[i]==2))  //NLP文件的包�??证书的不同格�??
		{
			//NLP Down
			f1.Open(szDownFullPath[i],CFile::modeRead|CFile::shareDenyNone);

			memset(szTotal,0,sizeof(szTotal));
			
			if (szFileType[i]==1) 
				nLen = 456 + 256;
			else if (szFileType[i]==0)
				nLen = 256;
			
			f1.Seek(13, CFile::begin);
			f1.Read(&nFileNum, 1);
			memset(szTmp,0,sizeof(szTmp));
			f1.Read(szTmp,1);
			if (m_MPosDown)
			{
				//appdebug
				if (szTmp[0] == 0x01)
				{
					bNlpDebug = TRUE;
				}
				//APPINFO
				int ttOff=0,ttLen=0;
				f1.Read(szTmp,1);
				bAppInfo = TRUE;
				if (szTmp[0] == 0x01)
				{
					szTotal[0] = 0x1a;
					//strcpy(szTotal,"APPINFO");
					ttOff=1;
					f1.Read(&ttLen,4);
					memcpy(szTotal+ttOff,&ttLen,4);
					ttOff+=4;
					f1.Read(szTmp,ttLen);
					memcpy(szTotal+ttOff,szTmp,ttLen);
					ttOff+=ttLen;
					f1.Read(&ttLen,4);
					memcpy(szTotal+ttOff,&ttLen,4);
					ttOff+=4;
					f1.Read(szTmp,ttLen);
					memcpy(szTotal+ttOff,szTmp,ttLen);
					ttOff+=ttLen;
					nRet=ARQSend((unsigned char *)szTotal,ttOff);
					if ( nRet< 0) 
					{
						OutputDebugString3("send 21");
						nRet=DOWN_ERR_SENDERROR;
						PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
						f1.Close();
						return nRet;
					}

					nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);
					if ( nRet< 0) 
					{			
						OutputDebugString3("recv 22");
						PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
						f1.Close();
						return nRet;
					}
					if (szTotal[0] != 0) 
					{		
						a = szTotal[0] + 86;
						OutputDebugString3("data error 22");
						PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, a); //数据处理错�??
						f1.Close();
						return -3;
					}
					
				}
			}
			if (!bAppInfo )
			{
				f1.Seek(1, CFile::current);
			}
			for (j = 0; j < nFileNum; j ++) 
			{
				if (szFileType[i]==2)
				{
					szTotal[0] = 7;//x509证书
					f1.Read(szTotal+1,4);
					memcpy(&nCrtLen,szTotal+1,4);
					f1.Read(szTotal+5,nCrtLen+256);
					nLen=nCrtLen+256+4;					
					f1.Read(&nDownFileLen, 4);
				}
				else
				{
					if (bNlpDebug)
						szTotal[0] = 0x0a;
					else
						szTotal[0] = 1;
					f1.Read(szTotal + 1, nLen);
					f1.Read(&nDownFileLen, 4);
				}

				nPos=0;

				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_MSG_DOWNING); 

				nRet=ARQSend((unsigned char *)szTotal, nLen + 1);

				if ( nRet< 0) 
				{
					OutputDebugString3("send 2");
					nRet=DOWN_ERR_SENDERROR;
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
					f1.Close();
					return nRet;
				}
				
				nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);
				if ( nRet< 0) 
				{			
					OutputDebugString3("recv 2");
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
					f1.Close();
					return nRet;
				}
				
				if (szTotal[0] != 0) 
				{		
					OutputDebugString3("data error 2");
					a = szTotal[0] + 86;
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, a); //数据处理错�??
					f1.Close();
					return -3;
				}
				else 
				{				
					nPos=0;
					nReadLen = nDownFileLen;
					while (nDownFileLen) 
					{
						nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;
						f1.Read(szTotal,nReadLen);

						nRet=ARQSend((unsigned char *)szTotal, nReadLen);

						if ( nRet< 0) 
						{
							OutputDebugString3("send 3");
							PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
							f1.Close();
							return nRet;
						}

						nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);
						if (nRet < 0) 
						{				
							OutputDebugString3("recv 3");
							PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
							f1.Close();
							return nRet;
						}
						
						if (szTotal[0] != 0) 
						{			
							OutputDebugString3("data error 3");
							a = szTotal[0] + 86;
							PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, a); //数据下载处理错�??
							f1.Close();
							return -3;
						}
						
						nDownFileLen -= nReadLen;
						nPos+=nReadLen;
						//20160322�??改进度条显示不完整的�??�??
						fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
						nProgress=fProgress;
						::PostMessage(m_hTermWnd, WM_PROGRESS, nThreadId, nProgress);	// 发送进度给调用窗体
					}					
					
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_CHECKING); //校验

					nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) ;
					if (nRet< 0) 
					{				
						OutputDebugString3("recv 4");
						PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
						f1.Close();
						return nRet;
					}
					
					if (szTotal[0] != 0) 
					{			
						OutputDebugString3("data error 4");
						a = szTotal[0] + 86;
						PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, a); //数据处理错�??
						f1.Close();
						return -3;
					}
					else
					{
						//PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_MSG_SUCCESS); 
						
						
					}
				}
			}
			f1.Close();
		}
		else if (szFileType[i]==8)
		{
		
			if (!f1.Open(szDownFullPath[i],CFile::modeRead|CFile::shareDenyNone))
			{
				msg.Format("open %s failed:%d",szDownFullPath[i],GetLastError());
				AfxMessageBox(msg);
				return -5;
			}
			
			memset(szTotal,0,sizeof(szTotal));

			szTotal[0] = 2;
			nDownFileLen= f1.GetLength();
			memcpy(szTotal + 1, &nDownFileLen, 4);
			int testLen = strlen(szDownFileName[i]);
			char *pName_Convert = NULL;
			pName_Convert=Gb232TUtf8(szDownFileName[i],&testLen);
			OutputDebugString(pName_Convert);
			strcpy((char *)szTotal + 5, pName_Convert);
		//	strcpy((char *)szTotal + 5, szDownFileName[i]);
			nLen = strlen((char *)szTotal + 5) + 1 + 4 + 1;

			PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_MSG_DOWNING); 

			if (ARQSend((unsigned char *)szTotal, nLen) < 0) 
			{
				OutputDebugString3("send 4");
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
				f1.Close();
				return -4;
			}

			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
			{
				OutputDebugString3("recv 5");
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
				f1.Close();
				return -2;
			}
			if (szTotal[0] != 0) 
			{
				OutputDebugString3("data error 5");
				a = szTotal[0] + 86;
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, a); //数据处理错�??
				f1.Close();
				return -3;
			}

			nPos=0;
			nReadLen = nDownFileLen;
			while (nDownFileLen) 
			{
				nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;

				f1.Read(szTotal, nReadLen);
				if (ARQSend((unsigned char *)szTotal, nReadLen) < 0) 
				{
					OutputDebugString3("send 5");
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_SENDERROR); //数据发送失�??
					f1.Close();
					return -4;					
				}

				if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
				{
					OutputDebugString3("recv 6");
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
					f1.Close();
					return -2;					
				}

				if (szTotal[0] != 0) 
				{
					OutputDebugString3("data error 6");
					a = szTotal[0] + 86;
					PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, a); //数据下载处理错�??
					return -3;
				}

				nDownFileLen -= nReadLen;
				nPos+=nReadLen;
				//20160322�??改进度条显示不完整的�??�??
				fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
				nProgress=fProgress;
				::PostMessage(m_hTermWnd, WM_PROGRESS, nThreadId, nProgress);	// 发送进度给调用窗体
			}

			PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_CHECKING); //校验...

			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
			{
				OutputDebugString3("recv 7");
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_ERR_RECVERROR); //数据接收失败
				f1.Close();
				return -2;
			}

			if (szTotal[0] != 0) 
			{
				OutputDebugString3("data error 7");
				a = szTotal[0] + 86;
				PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, a); //数据处理失败
				f1.Close();
				return -3;
			}
			else 
			{
				//PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_MSG_SUCCESS); 
				f1.Close();
			}
		}
	}

	memset(szTotal,0,sizeof(szTotal));
	szTotal[0]=3;

	 time_t unix_tm;
	SYSTEMTIME st;
	GetLocalTime(&st);
	FILETIME ft;
    SystemTimeToFileTime( &st, &ft );
    LONGLONG nLL;
    ULARGE_INTEGER ui;
    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;
    nLL = (ft.dwHighDateTime << 32) + ft.dwLowDateTime;
    unix_tm = (long)((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);	
	memcpy(szTotal + 1, &unix_tm, sizeof (unix_tm));
	
	if (m_MPosDown)
	{
		if (ARQSendMposEnd((unsigned char *)szTotal, 1 + sizeof (unix_tm)) < 0) 
		{
			return -4;
		}
	}
	else 
	{
		if (ARQSend((unsigned char *)szTotal, 1 + sizeof (unix_tm)) < 0) 
		{
			OutputDebugString3("send 6");
			return -4;
		}
			
	}
	PostMessage(m_hTermWnd, WM_ERRMSG, nThreadId, DOWN_MSG_SUCCESS); 
	return 0;
}


int CARQ::NLP_Shakehand(void)
{
	char szBuff[12];
	int nTimeOut=0;

	while (1)
	{
		if (!bActive)
			return -1;

		memset(szBuff,0,sizeof(szBuff));
		ReadData(szBuff,1);
		if ((szBuff[0]!=0x06) && (szBuff[0]!=0x05))
		{
			Sleep(100);
			nTimeOut+=100;
		//这�?�代码意义不明，若是有插拔线，会产生新的线程  ym
		//频繁的打开关闭串口，与USB监控线程冲突，在连续插拔线时经常导致程序卡�?�，设�?��?�理器异常，系统无法关机，与系统蓝屏等现�??
// 			if (nTimeOut==1000)
// 			{
// 				Disconnect();
// 				Connect();
// 				nTimeOut=0;
// 			}

			continue;
		}
		else if (szBuff[0]==0x06) //16K版本的握�??
			return SENDBUFFER_16K;
		else if (szBuff[0]==0x05)	//4K版本的握�??
			return SENDBUFFER_4K;
	}
}
int CARQ::NLP_GetApnInfo(char *clist)
{
	unsigned char cmd=0, szBuff[4096],szResult[2048];
	int i,nLen,nTotal=0,nRet=-1;
	memset(szBuff,0,sizeof(szBuff));
	memset(szResult,0,sizeof(szResult));

	PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	cmd = ANDROID_GETAPN;
	WHILE_ACTIVE
	{
		m_start=0;
		if (ARQSend(&cmd, 1) < 0) 
		{
			OutputDebugString("error11111111111111111");
			Sleep(500);
			continue;
		}

		memset(szBuff,0,sizeof(szBuff));

		if ((nLen=ARQRecv(szBuff, sizeof (szBuff), 5000)) < 0) 
		{
			Sleep(500);
			continue;
		}
		if (szBuff[0] != 0) 
		{
			return -2;
		}
		else
		{
			if (nLen>1)
			{
				memcpy(clist,&szBuff[1],nLen-1);
				return 0;
			}
			else
				return -1;
		}
	}
	return -1;
}

int CARQ::NLP_GetAppInfo(char *clist)
{
	unsigned char cmd=0, szBuff[4096],szResult[2048];
	int i,nLen,nTotal=0,nRet=-1;
	memset(szBuff,0,sizeof(szBuff));
	memset(szResult,0,sizeof(szResult));

	PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	cmd = ANDROID_GETAPP;
	WHILE_ACTIVE
	{
		m_start=0;
		if (ARQSend(&cmd, 1) < 0) 
		{
			OutputDebugString("error11111111111111111");
			Sleep(500);
			continue;
		}

		memset(szBuff,0,sizeof(szBuff));
	
		if ((nLen=ARQRecv(szBuff, sizeof (szBuff), 5000)) < 0) 
		{
			Sleep(500);
			continue;
		}
		if (szBuff[0] != 0) 
		{
			return -2;
		}
		else
		{
			if (nLen>1)
			{
				memcpy(clist,&szBuff[1],nLen-1);
				return 0;
			}
			else
				return -1;
		}
	}
	return -1;
}

int CARQ::DelAndroidAPP_PAPT(char *clist)
{
	unsigned char cmd=0, szBuff[4096],szResult[2048],total[2048];
	int i,nLen,nTotal=0,nRet=-1;
	memset(szBuff,0,sizeof(szBuff));
	memset(szResult,0,sizeof(szResult));

	PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	cmd = ANDROID_DELAPP;
	total[0]=cmd;
	memcpy(&total[1],clist,strlen(clist));
	WHILE_ACTIVE
	{
		m_start=0;
		if (ARQSend(total, 1+strlen(clist)) < 0) 
		{
			Sleep(500);
			continue;
		}

		memset(szBuff,0,sizeof(szBuff));

		if ((nLen=ARQRecv(szBuff, sizeof (szBuff), 1000)) < 0) 
		{
			Sleep(500);
			continue;
		}
		if (szBuff[0] != 0) 
		{
			return -2;
		}
		else
		{
			return 0;
		}
	}
	return -1;
}
#endif



/**
 * @brief	cruxplus 发送非bin文件数据
 * @param[in] pszData  要发送的数据
 * @param[in] nDataLen  发送数据长度
 * @return
 * @li >0 成功
 * @li <0 失败
 */
int CARQ::CRC16DataSend(unsigned char* pszData, int nDataLen)
{
	unsigned char uszBuff[2] = {0};
	unsigned char uszSend[1024 * 64 + 2] = {0};
	int nRet = 0;
	UINT16 crc = crc16(0x0000, pszData, nDataLen);
	uszBuff[0] = crc & 0x00FF;
	uszBuff[1] = (crc >> 8) & 0x00FF;
	memcpy(uszSend, pszData, nDataLen);
	memcpy(uszSend + nDataLen, uszBuff, 2);

	nRet = write(m_fd, uszSend, nDataLen + 2);
	if(nRet != nDataLen + 2)
	{
		return -1;
	}
	return nRet;
}


/**
 * @brief	cruxplus 发送bin数据
 * @param[in] ucCmd  命令字
 * @param[in] pszData  发送的数据
 * @param[in] nDataLen  发送数据的长度
 * @return
 * @li >0 成功
 * @li <0 失败
 */
int CARQ::HDLCSend(unsigned char ucCmd, unsigned char* pszData, int nDataLen)
{
	unsigned char uszBuff[2048] = {0};
	int nRet = 0;
	int nSendLen = 4;
	uszBuff[0] = 0x02;
	uszBuff[1] = ucCmd;
	uszBuff[2] = (nDataLen) & 0xFF;
	uszBuff[3] = ((nDataLen) & 0xFF00) >> 8;
	if (pszData) {
		memcpy(uszBuff + 4, pszData, nDataLen);
		nSendLen += nDataLen;
	}
	UINT16 crc = crc16(0x0000, uszBuff, nSendLen);
	uszBuff[nSendLen ++] = crc & 0x00FF;
	uszBuff[nSendLen ++] = (crc >> 8) & 0x00FF;
	
	nRet = write(m_fd, uszBuff, nSendLen);
	if(nRet != nSendLen)
	{
		//CFunCommon::DlSprintf("write error\n");
		return -1;
	}
	return nRet;
}


/**
 * @brief	cruxplus 接收数据
 * @param[in] pszData  读出的数据
 * @param[in] nTimout  超时时间
 * @param[in] ck  未使用，不知道早期定义背景
 * @return 读出数据的长度
 */
int CARQ::HDLCRecv(unsigned char* pszData, int nTimout, int ck)
{
	unsigned char uszBuff[2048] = {0}; 
	int nRet = 0;
	int nFlag = 0, nLen = 0;
	unsigned char* p = uszBuff;
	do {
		if ((nRet = ReadByteWithTimeout(nTimout)) < 0)
		{
			break;
		}
		*p = nRet;
		if (nFlag == 0) 
		{
			if (*p == 0x02) 
			{
				nFlag++;
				p++;
			}
		}
		else 
		{
			p++;
			if (nLen == 0 && (p - uszBuff >= 4)) 
			{
				nLen = (uszBuff[2] | (uszBuff[3] << 8)) + 4 + 2;
			}
			else if (nLen) 
			{
				if ((p - uszBuff) >= nLen) 
				{
					if (ck) 
					{
						UINT16 crc = crc16(0x0000, uszBuff, nLen - 2);
						if (crc != ((uszBuff[nLen - 1] << 8) | uszBuff[nLen - 2])) 
						{
							nRet = -100;
						}
					}
					pszData[0] = uszBuff[1];
					memcpy(pszData + 1, uszBuff + 4, nLen - 6);
					nRet = nLen - 5;
					break;
				}
			}
		}
	} while (1);
	return nRet;
}


/**
 * @brief	cruxplus应用下载
 * @param[in] pszFileList  下载的应用文件列表
 * @return
 * @li 0 成功
 * @li <0 失败
 */
int CARQ::CruxPlus_DownLoad_APP(const char *pszFileList)
{

	unsigned char uszTotal[1024*64+1] = {0};		
	unsigned char uszRecv[1024*64 + 1] = {0};
	unsigned char ucFileNum;
	unsigned char cmd ;
	char szBuff[1024*64 + 1] = {0};
	char szDownFullPath[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char szDownFileName[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char szTmp[256] = {0};
		
	int szFileType[MAX_DOWNFILE_NUM],	//文件类型
		nTotalFileCount = 0,			//文件数量
		nDownFileLen = 0,				//下载的文件长度
		nPos = 0,
		nReadLen = 0,
		nCrtLen = 0,
		i,j,nOffset,nRet= -1,nProgress=0,
		nPrePack = 0,nLen = 0,nTimes = 0;
	float fProgress = 0.0f;
	string strModeBuf;
	FILE *f1 = NULL;

	//解析列表
	memset(uszTotal,0,sizeof(uszTotal));
	memset(szBuff,0,sizeof(szBuff));
	
	strcpy(szBuff,pszFileList);
	nDownFileLen = strlen(szBuff);
	nTotalFileCount = 0;
	nOffset = 0;
	for (i=0;i<nDownFileLen;i++)  
	{
		if (szBuff[i]=='*')
		{
			szBuff[i]='\0';
			strcpy(szDownFullPath[nTotalFileCount],(char *)(szBuff+nOffset));
			szBuff[i]='*';
			for (j=strlen(szDownFullPath[nTotalFileCount])-1;j>0;j--)
			{
				if (szDownFullPath[nTotalFileCount][j]=='\\')
				{
					strcpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1);
					break;
				}
			}

			for (j=i;;j++)
			{
				if ('|' == szBuff[j])
					break;
			}

			memset(szTmp,0,sizeof(szTmp));
			memcpy(szTmp,szBuff+i+1,j-i-1);
			szFileType[nTotalFileCount]=atoi(szTmp);
			nOffset = j+1;			
			nTotalFileCount++;
		}
	}
	//下载
	cmd = cmd_cruxplus_boot;
	nRet = HDLCSend(cmd, NULL,0);
	if ( 6 != nRet)  
	{
		CFunCommon::DlSprintf("Failed to send cmd_cruxplus_info.\n");
		return -1;		
	}

	memset(uszTotal,0,sizeof(uszTotal));
	nRet = HDLCRecv(uszTotal,1000*5, 0);
	if ( nRet <= 0) 
	{		
		CFunCommon::DlSprintf("Failed to recv cmd_cruxplus_info.\n");
		return -1;
	}
	
	if (uszTotal[0] != cmd && uszTotal[1] != 0)
	{			
		CFunCommon::DlSprintf("Failed: data  error: %02x:%02x\n",uszTotal[0], uszTotal[1]);
		return -1;
	}
	if (strstr((char *)(uszTotal + 2), "crux+ boot") != NULL)
	{
		strModeBuf = "crux+ boot";	
	}
	else if (strstr((char *)(uszTotal + 2), "crux+ sys") != NULL || strstr((char *)(uszTotal + 2), "ME50N") != NULL || strstr((char *)(uszTotal + 2), "nl.ac35") != NULL)
	{
		strModeBuf = "crux+ sys";
		
	}
	else
	{
		CFunCommon::DlSprintf("Failed to get boot info.\n");
		return -1;	
	}
	
	
	if (m_bClearApp)
	{
		cmd = cmd_cruxplus_clearapp;
		if(HDLCSend(cmd, NULL,0) != 6)
		{
			CFunCommon::DlSprintf("Failed to send cmd_cruxplus_clearapp.\n");
			return -1;
		}
		memset(uszTotal,0,sizeof(uszTotal));
		nRet=HDLCRecv(uszTotal,1000*5, 0);
		if ( nRet <= 0) 
		{		
			CFunCommon::DlSprintf("Failed to recv cmd_cruxplus_clearapp, timeout\n");
			return -1;
		}

		if (cmd != uszTotal[0] && 0 != uszTotal[1])
		{
			CFunCommon::DlSprintf("Failed: recv data error\n");
			return -1;	
		}
	}


	for (i=0;i<nTotalFileCount;i++)
	{
		//NLP格式文件
		if ((0 == szFileType[i]) || (1 == szFileType[i]) || (2 == szFileType[i])) 
		{
			//NLP Down
			if (1 == szFileType[i]) 
				nLen = 456 + 256;
			else if (0 == szFileType[i])
				nLen = 256;
		//	CFunCommon::DlSprintf("Downloading %s\n",szDownFullPath[i]);
			if (NULL == (f1 = fopen(szDownFullPath[i], "rb"))) 
			{
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -2;
			}
			fseek(f1, 13,SEEK_SET );
			fread(&ucFileNum, 1, 1, f1);
			fseek(f1, 2,SEEK_CUR );
		
			for (j = 0; j < ucFileNum; j ++) 
			{
				memset(uszTotal,0,sizeof(uszTotal));
				fread(uszTotal, 1, nLen, f1);
		
				cmd = cmd_cruxplus_dwnnlp;
				if (strstr(szDownFullPath[i],".mh1902") != NULL) //me68, mh1902的固件用05指令
				{
					cmd = 0x05;
				}
				if (HDLCSend(cmd, uszTotal, nLen) < 0)
				{
					CFunCommon::DlSprintf("Failed to send sign data file %s\n",szDownFullPath[i]);
					fclose(f1);
					return -1;
					
				}
				memset(uszTotal,0,sizeof(uszTotal));
				nRet = HDLCRecv(uszTotal,1000*30,0);
				if (nRet <0)
				{
					CFunCommon::DlSprintf("Failed:DownFile:%s, Failed to recv sign data:%02x:%02x\n",szDownFullPath[i],uszTotal[0], uszTotal[1]);
					fclose(f1);
					return -1;
				}
				if (cmd != uszTotal[0] || 0 != uszTotal[1])
				{
					CFunCommon::DlSprintf("Failed: DownFile:%s, signture decode error:%02x:%02x.\n",szDownFullPath[i],uszTotal[0], uszTotal[1]);
					fclose(f1);
					return -1;
				}
				memcpy(&nPrePack,uszTotal + 2,4);
				fread(&nDownFileLen, 1, 4, f1);	
				nPos=0;				
				nReadLen = nDownFileLen;
				while (nDownFileLen) 
				{
						nReadLen = nDownFileLen > nPrePack ? nPrePack : nDownFileLen;
						memset(uszTotal, 0, sizeof(uszTotal));
						fread(uszTotal, 1, nReadLen, f1);	
						for (nTimes = 0; nTimes<m_nRetry; nTimes++)
						{
							nRet = CRC16DataSend(uszTotal, nReadLen);
							if (nRet != (nReadLen+2))
							{		
								CFunCommon::DlSprintf("Failed to send data.\n");						
								fclose(f1);
								return -1;
							}
							memset(uszRecv, 0, sizeof(uszRecv));
							nRet = HDLCRecv(uszRecv, 1000, 0);
							if (nRet > 0)
							{
								
								if (0x02 == uszRecv[0] && 0x0a == uszRecv[1])
								{
						
									usleep(200*1000);
								}
								else
									break;
							}
							else {								
								
							}
							usleep(50*1000);
						}
					
						
						if (2 != nRet || cmd != uszRecv[0] || 0 != uszRecv[1] )
						{									
						//	CFunCommon::DlSprintf("Failed:DownFile:%s,  data error:%02x:%02x\n",szDownFullPath[i],uszRecv[0], uszRecv[1]);
							fclose(f1);
							return -1;
						}						
						

						nDownFileLen -= nReadLen;
						nPos += nReadLen;
						fProgress = 0.1f*nPos*1000/(nDownFileLen+nPos);
						nProgress = fProgress;
					//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
						(*gCallBack)(szDownFullPath[i],nProgress);
						usleep(10);

				}				
			//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);
				(*gCallBack)(szDownFullPath[i],100);	
				memset(uszTotal,0,sizeof(uszTotal));
				nRet=HDLCRecv(uszTotal,  1000*10,0);
				if ( cmd != uszTotal[0] || 0 != uszTotal[1]) 
				{								
					CFunCommon::DlSprintf("Failed: DownFile:%s,  data error1:%02x:%02x\n",szDownFullPath[i],uszTotal[0], uszTotal[1]);
					fclose(f1);
					return -1;
				}
				else
				{
						
					usleep(10*1000);
				}
				
			}
			fclose(f1);
		}
		//非NLP文件
		else if (8 == szFileType[i])
		{

		//	CFunCommon::DlSprintf("Downloading %s\n",szDownFullPath[i]);
		    if (NULL == (f1 = fopen(szDownFullPath[i], "rb"))) 		
			{
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -5;

			}
			fread(szTmp, 1, 8, f1);
			//发送文件名
			nDownFileLen = strlen(szDownFileName[i]);
			memset(uszTotal,0,sizeof(uszTotal));
			memcpy(uszTotal,&nDownFileLen,4);
			memcpy(uszTotal + 4,szDownFileName[i],strlen(szDownFileName[i]));
		
			nLen = nDownFileLen + 4 +1;
			cmd = cmd_cruxplus_dwnfile;
			nRet = HDLCSend(cmd, uszTotal,nLen);
			if ( nRet < 0) 
			{
				CFunCommon::DlSprintf("Failed to send data.\n");						
				fclose(f1);
				return -1;
			}
			memset(uszTotal,0,sizeof(uszTotal));

			
			nRet = HDLCRecv(uszTotal, 1000*10, 0);	
			if (nRet < 0 || cmd != uszTotal[0]  || 0 != uszTotal[1] )
			{
				CFunCommon::DlSprintf("Failed:DownFile:%s,  data error nRet:%d:%02x:%02x\n",szDownFullPath[i],nRet,uszTotal[0], uszTotal[1]);
				fclose(f1);
				return -1;
			}
			memcpy(&nPrePack, uszTotal + 2, 4);
			fseek(f1, 0,SEEK_SET );
	
			nProgress=0;
		
			while (nDownFileLen) 
			{
				nReadLen = nDownFileLen > nPrePack ? nPrePack : nDownFileLen;
				fread(uszTotal, 1, nReadLen, f1);
				for (nTimes = 0; nTimes < m_nRetry; nTimes++)
				{
					nRet = CRC16DataSend(uszTotal, nReadLen);
					if (nRet != (nReadLen + 2))
					{
						CFunCommon::DlSprintf("Failed to send data3.\n");	
						fclose(f1);
						return -1;
					}
					memset(uszRecv, 0, sizeof(uszRecv));
					nRet = HDLCRecv(uszRecv, 1000, 0);
					if (nRet > 0)
					{

						if (cmd == uszRecv[0] && 0x0a == uszRecv[1])
						{
							//CFunCommon::DlSprintf("retry\n");
							continue;
						}
						else
							break;
					}
					else {
						//CFunCommon::DlSprintf("timeout retry.\n");	
					}
					usleep(200*1000);
				}

				
				if (nRet < 0||cmd != uszRecv[0] || 0 != uszRecv[1] )
				{			
					CFunCommon::DlSprintf("Failed:DownFile:%s,  data error1 nRet:%d:%02x:%02x\n",szDownFullPath[i],nRet,uszTotal[0], uszTotal[1]);
					fclose(f1);
					return -1;

				}

				nDownFileLen -= nReadLen;
				nPos+=nReadLen;

				fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
				nProgress=fProgress;

			//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
				(*gCallBack)(szDownFullPath[i],nProgress);
				usleep(10);
			}				
		//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);	
			(*gCallBack)(szDownFullPath[i],100);
		
			memset(uszTotal,0,sizeof(uszTotal));
			nRet = HDLCRecv(uszTotal, 1000*20, 0);	
			
			
			if (nRet < 0||cmd != uszTotal[0]|| 0 != uszTotal[1])
			{							
				CFunCommon::DlSprintf("Failed:DownFile:%s,  data error2 nRet:%d:%02x:%02x\n",szDownFullPath[i],nRet,uszTotal[0], uszTotal[1]);
				fclose(f1);
				return -1;
			}
			fclose(f1);
			usleep(10);	
		}
	}
	if("crux+ sys" == strModeBuf) //在命令模式下，方�??更新系统时间
	{
		memset(uszTotal,0,sizeof(uszTotal));
		//time_t unix_tm;
		time_t unix_tm;
		time( &unix_tm );
		unix_tm = unix_tm + 8*3600; 
    	
		memcpy(uszTotal, &unix_tm, sizeof (unix_tm));
		
		cmd = cmd_cruxplus_timesync;
		
		nRet = HDLCSend(cmd, uszTotal,sizeof (unix_tm));
		if ( nRet< 0) 
		{
			CFunCommon::DlSprintf("Failed to send data4.\n");	
			return -1;
		}
		memset(uszTotal,0,sizeof(uszTotal));
	
		nRet = HDLCRecv(uszTotal, 1000*10, 0);	
		

	
		if (cmd != uszTotal[0] || 0 != uszTotal[1])
		{			
			CFunCommon::DlSprintf("Failed to Synchronization time.\n");		
			return -1;
		}
	}

	cmd = cmd_cruxplus_reset;
	nRet = HDLCSend(cmd, NULL,0);
	
	if ( nRet< 0) 
	{
		CFunCommon::DlSprintf("Failed to send data5.\n");	
		return -1;
	}

	return 0;
}


/**
 * @brief	cruxplus固件下载
 * @param[in] pszFileList  下载的固件文件列表
 * @return
 * @li 0 成功
 * @li <0 失败
 */
int CARQ::CruxPlus_DownLoad_Firm(const char *pszFileList)
{

	unsigned char uszTotal[1024*64+1] = {0};		
	unsigned char uszRecv[1024*64+1] = {0};
	unsigned char ucFileNum;
	unsigned char cmd ;
	char szBuff[1024*64 + 1] = {0};
	char szDownFullPath[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char szDownFileName[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char szTmp[256] = {0};
		
	int szFileType[MAX_DOWNFILE_NUM] = {0},			//文件类型
		nTotalFileCount = 0,		//文件数量
		nDownFileLen = 0,			
		nPos = 0,
		nReadLen = 0,
		i,j,nOffset = 0,nRet= -1,nProgress=0,
		nPrePack = 0,nLen = 0,nTimes = 0;

	float fProgress=0.0f;
	string strModeBuf;
	FILE *f1 = NULL;
	
	//解析列表
	memset(uszTotal,0,sizeof(uszTotal));
	memset(szBuff,0,sizeof(szBuff));
	strcpy(szBuff,pszFileList);
	nDownFileLen = strlen(szBuff);
	nTotalFileCount = 0;
	nOffset = 0;
	for (i=0;i<nDownFileLen;i++)  
	{
		if ('*' == szBuff[i])
		{
			szBuff[i] = '\0';
			strcpy(szDownFullPath[nTotalFileCount],(char *)(szBuff+nOffset));
			szBuff[i] = '*';

			for (j=strlen(szDownFullPath[nTotalFileCount])-1;j>0;j--)
			{
				if (szDownFullPath[nTotalFileCount][j]=='\\')
				{
					strcpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1);
					break;
				}
			}
			for (j=i;;j++)
			{
				if ('|' == szBuff[j])
					break;
			}

			memset(szTmp,0,sizeof(szTmp));
			memcpy(szTmp,szBuff+i+1,j-i-1);
			szFileType[nTotalFileCount] = atoi(szTmp);
			nOffset = j+1;			
			nTotalFileCount++;
		}
	}
	//下载
	cmd = cmd_cruxplus_boot;



	if (HDLCSend(cmd, NULL,0) != 6) 
	{
		CFunCommon::DlSprintf("Failed to send cmd_cruxplus_info.\n");
		return -1;		
	}

	memset(uszTotal,0,sizeof(uszTotal));
	nRet = HDLCRecv(uszTotal,1000*5, 0);
	if ( nRet <= 0) 
	{		
		CFunCommon::DlSprintf("Failed to recv cmd_cruxplus_info.\n");
		return -1;
	}
	
	if (cmd != uszTotal[0]  && 0 != uszTotal[1] )
	{			
		CFunCommon::DlSprintf("Failed: recv data  errorcode: %02x:%02x\n",uszTotal[0], uszTotal[1]);
		return -1;
	}
	if (strstr((char *)(uszTotal + 2), "crux+ boot") != NULL)
	{
		strModeBuf = "crux+ boot";	
	}
	else if (strstr((char *)(uszTotal + 2), "crux+ sys") != NULL || strstr((char *)(uszTotal + 2), "ME50N") != NULL || strstr((char *)(uszTotal + 2), "nl.ac35") != NULL)
	{
		strModeBuf = "crux+ sys";
	}

	else
	{
		CFunCommon::DlSprintf("Failed to get boot info.\n");
		return -1;	
	}
	sprintf(szBuff,"%s",uszTotal+2);
	
	for (i=0; i<nTotalFileCount; i++)
	{
		//NLP格式文件
		if ((0 == szFileType[i]) || (1 == szFileType[i]) || (2 == szFileType[i]))  
		{
			//NLP Down
			if (1 == szFileType[i]) 
				nLen = 456 + 256;
			else if (0 == szFileType[i])
				nLen = 256;

		//	CFunCommon::DlSprintf("Downloading %s\n",szDownFullPath[i]);
			if (NULL == (f1 = fopen(szDownFullPath[i], "rb"))  ) 
			{
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -2;
			}
			fseek(f1, 13,SEEK_SET );
			fread(&ucFileNum, 1, 1, f1);
			fseek(f1, 2,SEEK_CUR );
		
			for (j = 0; j < ucFileNum; j ++) 
			{
				memset(uszTotal,0,sizeof(uszTotal));
				fread(uszTotal, 1, nLen, f1);
	
				cmd = cmd_cruxplus_dwnnlp;
				if (strstr(szDownFullPath[i],".mh1902") != NULL) //me68, mh1902的固件用05指令
				{
					cmd = 0x05;
				}
				if ((nRet = HDLCSend(cmd, (unsigned char*)uszTotal, nLen)) < 0)
				{
					CFunCommon::DlSprintf("Failed to send sign data file %s\n",szDownFullPath[i]);
					fclose(f1);
					return -1;
					
				}
				memset(uszTotal,0,sizeof(uszTotal));
				nRet = HDLCRecv(uszTotal,1000*30,0);
				if (nRet <0)
				{
					CFunCommon::DlSprintf("Failed:DownFile:%s,  recv sign error:%02x:%02x\n",szDownFullPath[i],uszTotal[0], uszTotal[1]);
					fclose(f1);
					return -1;
				}
				if (uszTotal[0] != cmd || uszTotal[1] != 0)
				{
					CFunCommon::DlSprintf("Failed:DownFile:%s, signture decode error:%02x:%02x.\n",szDownFullPath[i],uszTotal[0], uszTotal[1]);
					fclose(f1);
					return -1;
				}
				memcpy(&nPrePack,uszTotal + 2,4);
				fread(&nDownFileLen, 1, 4, f1);	
				nPos=0;				
				nReadLen = nDownFileLen;
				while (nDownFileLen) 
				{
						nReadLen = nDownFileLen > nPrePack ? nPrePack : nDownFileLen;
						memset(uszTotal, 0, sizeof(uszTotal));
						fread(uszTotal, 1, nReadLen, f1);	
						for (nTimes = 0; nTimes<m_nRetry; nTimes++)
						{
							nRet = CRC16DataSend(uszTotal, nReadLen);
							if (nRet != (nReadLen+2))
							{		
								CFunCommon::DlSprintf("Failed to send data.\n");						
								fclose(f1);
								return -1;
							}
							memset(uszRecv, 0, sizeof(uszRecv));
							nRet = HDLCRecv(uszRecv, 1000, 0);
							if (nRet > 0)
							{
								if (0x02 == uszRecv[0]  && 0x0a == uszRecv[1] )
								{
									//CFunCommon::DlSprintf("DownFile:%s,  data error:%02x:%02x\n",szDownFullPath[i],uszTotal[0], uszTotal[1]);
									usleep(200*1000);
								}
								else
									break;
							}
							else {
								//CFunCommon::DlSprintf("DownFile:%s,  data error:%02x:%02x\n",szDownFullPath[i],uszTotal[0], uszTotal[1]);
									
							}
							usleep(50*1000);
						}	
						if (2 != nRet  || cmd != uszRecv[0]  || 0 != uszRecv[1] )
						{									
							CFunCommon::DlSprintf("Failed:DownFile:%s, data error:%02x:%02x\n",szDownFullPath[i],uszRecv[0], uszRecv[1]);
							fclose(f1);
							return -1;
						}						
						nDownFileLen -= nReadLen;
						nPos += nReadLen;
			
						fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
						nProgress=fProgress;
					//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
						(*gCallBack)(szDownFullPath[i],nProgress);
						usleep(10);

				}				
			//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);	
				(*gCallBack)(szDownFullPath[i],100);
				memset(uszTotal,0,sizeof(uszTotal));
				nRet = HDLCRecv(uszTotal,  1000*10,0);
				if (uszTotal[0] != cmd || 0 != uszTotal[1])
				{								
					CFunCommon::DlSprintf("Failed:DownFile:%s, data error:%02x:%02x\n",szDownFullPath[i],uszTotal[0], uszTotal[1]);
					fclose(f1);
					return -1;
				}
				else
				{
						
					usleep(10*1000);
				}
				
			}
			fclose(f1);
		}
		//非NLP文件
		else if (8 == szFileType[i])
		{

		//	CFunCommon::DlSprintf("Downloading %s\n",szDownFullPath[i]);
		    if (NULL == (f1 = fopen(szDownFullPath[i], "rb")) ) 		
			{
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -5;

			}
			fread(szTmp, 1, 8, f1);
			//发送文件名
			nDownFileLen = strlen(szDownFileName[i]);
			memset(uszTotal,0,sizeof(uszTotal));
			memcpy(uszTotal,&nDownFileLen,4);
			memcpy(uszTotal + 4,szDownFileName[i],strlen(szDownFileName[i]));
		
			nLen = nDownFileLen + 4 +1;
			cmd = cmd_cruxplus_dwnfile;
			nRet = HDLCSend(cmd, uszTotal,nLen);
			if ( nRet< 0) 
			{
				CFunCommon::DlSprintf("Failed to send data.\n");						
				fclose(f1);
				return -1;
			}
			memset(uszTotal,0,sizeof(uszTotal));	
			nRet = HDLCRecv(uszTotal, 1000*10, 0);	
			if (nRet < 0 || cmd != uszTotal[0] || 0 != uszTotal[1])
			{
				CFunCommon::DlSprintf("Failed:DownFile:%s, data error nRet:%d:%02x:%02x\n",szDownFullPath[i],nRet,uszTotal[0], uszTotal[1]);
				fclose(f1);
				return -1;
			}
			memcpy(&nPrePack, uszTotal + 2, 4);
			fseek(f1, 0,SEEK_SET );
	
			nProgress=0;
		
			while (nDownFileLen) 
			{
				nReadLen = nDownFileLen > nPrePack ? nPrePack : nDownFileLen;
				fread(uszTotal, 1, nReadLen, f1);
				for (nTimes = 0; nTimes < m_nRetry; nTimes++)
				{
					nRet = CRC16DataSend(uszTotal, nReadLen);
					if (nRet != (nReadLen + 2))
					{
						CFunCommon::DlSprintf("Failed to send data3.\n");	
						fclose(f1);
						return -1;
					}
					memset(uszRecv, 0, sizeof(uszRecv));
					nRet = HDLCRecv(uszRecv, 1000, 0);
					if (nRet > 0)
					{
						if (uszRecv[0] == cmd && uszRecv[1] == 0x0a)
						{
						//	CFunCommon::DlSprintf("retry\n");
							continue;
						}
						else
							break;
					}
					else {
						//CFunCommon::DlSprintf("timeout retry.\n");	
					}
					usleep(200*1000);
				}

				
				if (nRet < 0||cmd != uszRecv[0] || 0 != uszRecv[1] )
				{			
					CFunCommon::DlSprintf("Failed: DownFile:%s,  data error1 nRet:%d:%02x:%02x\n",szDownFullPath[i],nRet,uszTotal[0], uszTotal[1]);
					fclose(f1);
					return -1;

				}

				nDownFileLen -= nReadLen;
				nPos+=nReadLen;
				fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
				nProgress=fProgress;

			//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
				(*gCallBack)(szDownFullPath[i],nProgress);
				usleep(10);
			}				

		//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);	
			(*gCallBack)(szDownFullPath[i],100);
			memset(uszTotal,0,sizeof(uszTotal));
			nRet = HDLCRecv(uszTotal, 1000*20, 0);	
			
			
			if (nRet < 0 || uszTotal[0] != cmd || uszTotal[1] != 0)
			{							
				CFunCommon::DlSprintf("Failed: DownFile:%s,  data error2 nRet:%d:%02x:%02x\n",szDownFullPath[i],nRet,uszTotal[0], uszTotal[1]);
				fclose(f1);
				return -1;
			}
			fclose(f1);
			usleep(10);	
		}
	}
	if(strModeBuf == "crux+ sys") //在命令模式下，方�??更新系统时间
	{
		memset(uszTotal,0,sizeof(uszTotal));
		//time_t unix_tm;
		time_t unix_tm;
		time( &unix_tm );
		unix_tm = unix_tm + 8*3600; 
    	
		memcpy(uszTotal, &unix_tm, sizeof (unix_tm));
		
		cmd = cmd_cruxplus_timesync;
		
		nRet=HDLCSend(cmd, uszTotal,sizeof (unix_tm));
		if ( nRet< 0) 
		{
			CFunCommon::DlSprintf("Failed to send data4.\n");	
			return -1;
		}
		memset(uszTotal,0,sizeof(uszTotal));
	
		nRet = HDLCRecv(uszTotal, 1000*10, 0);	
		

	
		if (cmd != uszTotal[0]  || 0 !=  uszTotal[1])
		{			
			CFunCommon::DlSprintf("Failed to Synchronization time.\n");		
			return -1;
		}

	}
	//重启
	cmd = cmd_cruxplus_reset;
	nRet=HDLCSend(cmd, NULL,0);
	
	if ( nRet< 0) 
	{
		CFunCommon::DlSprintf("Failed to send data5.\n");	
		return -1;
	}

	return 0;
}


 
 /**
 * @brief	通过轮询获取boot信息，判断设备属于cruxpluse还是scrop
 * @param[in] nType 	1、下载应用  2、下载固件
 * @return
 * @li 1：scop平台
 * @li 2: cruxplus 平台
 * @li -1：失败
*/
int CARQ::GetMposPlat(int nType)
{
	
	U8 cmd = 0x00 ;
	U8 szTotal[1024] = {0} ;
	int nRet = 0;
	int nTimes=0;
	int nMaxRetry = 5;
	while (nTimes++ < nMaxRetry)
	{	
		//cruxplus 
		cmd = cmd_cruxplus_boot;
		nRet = HDLCSend(cmd, NULL,0);
		if ( nRet != 6) 
		{		
			sleep(1);
			continue;
		}
		memset(szTotal,0,sizeof(szTotal));
		nRet=HDLCRecv(szTotal,1000, 0);
		if (nRet > 2)
		{
			if (szTotal[0] != cmd || szTotal[1] != 0)
			{
				continue;
			}
			if (1 == nType)
			{
				if (strstr((char *)(szTotal + 2), "crux+ sys") != NULL)
				{
					return mpos_plat_cruxplus;
				}

			}
			if (2 == nType)
			{			
			//	CFunCommon::DlSprintf("%s\n",(char *)(szTotal + 2));
				if (strstr((char *)(szTotal + 2), "crux+ boot") != NULL)
				{
					return mpos_plat_cruxplus;
				}	
			}
			continue;
	
		}

		//scrop
		cmd = cmd_scrop_boot;
		if (ARQSend(&cmd, 1) < 0) 
		{
			continue;	
		}
		memset(szTotal,0,sizeof(szTotal));
		nRet=ARQRecv(szTotal, sizeof (szTotal), 1000);
		if ( nRet < 0 || szTotal[0] != 0) 
		{		
			
			continue;
		}
		return mpos_plat_scrop;
		
	}
	CFunCommon::DlSprintf("Failed to get plat .\n");
	return -1;
}

/***************************************android app********************************************************/

/**
 * @brief	android设备 握手，返回通讯数据帧大小
 * @return
 * @li SENDBUFFER_16K  16K版本
 * @li SENDBUFFER_4K  4k版本
 * @li -1 握手失败 
 */
int CARQ::Android_Shakehand(void)
{
	int nRet = 0;
	int nTimes = 30; //握手超时设置为30s
	U8 cc = 0;
	while (nTimes-- > 0)
	{
		//CFunCommon::DlSprintf("nTimes:%d\n",nTimes);
		nRet=read(m_fd,&cc,1);
	//	CFunCommon::DlSprintf("nRet:%d cc:%02x ,read:%s\n",nRet,cc,strerror(errno));
		if (0 == nRet)
		{
			usleep(1000*1000);
			continue;
		}
		else if (nRet < 0)
		{
		//	CFunCommon::DlSprintf("Failed to read:%s\n",strerror(errno));
			return -1;
		}
		if (0x06 != cc && 0x05 != cc)
		{
		//	CFunCommon::DlSprintf("nTimes:%d,nRet=%02x\n",nTimes,nRet);
			continue;
		}
		else if (0x06 == cc) //16K版本的握手
		{
			return SENDBUFFER_16K;
		}
			
		else if (0x05 == cc)	//4K版本的握手
		{
			return SENDBUFFER_4K;
		}	
	}
	return -1;
}

/**
 * @brief	获取智能设备固件版本信息
 * @param[out] pszVer  设备返回版本信息
 * @return
 * @li true 成功
 * @li false 失败
 */
bool CARQ::Android_GetVersion(char* pszVer)
{
	U8 cmd=0;
	U8 uszBuff[4096] = {0};
	char szResult[2048] = {0}; 
	int nLen = 0;
	int nTotal=0;
	int nTimes = 60;

	m_start=0;
	cmd = cmd_scrop_boot;
	FlushPort();

	while (nTimes-- > 0)
	{
		if (ARQSend(&cmd, 1) < 0) 
		{
			CFunCommon::DlSprintf("Failed to get pos version info.\n");
			continue;
		
		}
		memset(uszBuff,0,sizeof(uszBuff));
		if (ARQRecv(uszBuff, sizeof (uszBuff), 1000) < 0) 
		{
			CFunCommon::DlSprintf("Failed to get pos version info1.");
			continue;
		}	
		if (0 != uszBuff[0]) 
		{
			CFunCommon::DlSprintf("Failed to get pos version info2.");
			return false;
		}
		else
		{
			for (int i=1; i<4096; i++)
			{
				if (0x80 == uszBuff[i])//解析机型
				{
					nLen = uszBuff[i+1];
					strncat(szResult, (char *)uszBuff+i+2, nLen);
					nTotal += nLen;
					strcat(szResult,"_");
					nTotal++;
				}
				else if (0x81 == uszBuff[i])//解析硬件识别码
				{
					nLen = uszBuff[i+1];
					strncat(szResult, (char *)uszBuff+i+2, nLen);
					nTotal += nLen;
					strcat(szResult,"_");
					nTotal++;
				}
				else if (0x82 == uszBuff[i])//解析软件版本
				{
					nLen = uszBuff[i+1];
					strncat(szResult, (char *)uszBuff+i+2, nLen);
					nTotal += nLen;
					strcat((char *)szResult, "_");
					nTotal++;				
				}
				else if (0x83 == uszBuff[i])//解析SN码
				{
					nLen = uszBuff[i+1];
					strncat(szResult, (char *)uszBuff+i+2, nLen);
					nTotal += nLen;
					strcat(szResult, "_");
					nTotal++;	
				}
				else if (0x84 == uszBuff[i])//解析PN码
				{
					nLen = uszBuff[i+1];
					strncat(szResult, (char *)uszBuff+i+2, nLen);
					nTotal += nLen;
					memcpy(pszVer,szResult,nTotal);
					
					(*gCallBack)(pszVer,-2);
					return true;
				}
			}
			return false;
		}
	}
	return false;
}



/**
 * @brief	获取智能设备应用列表
 * @param[out] pszAppList  设备返回应用信息
 * @return
 * @li true 成功
 * @li false 失败
 */
bool CARQ::Android_GetAppInfo(string& strAppList)
{
	unsigned char cmd=0, szBuff[4096*10] = {0};
	char szTmp[4096*10] = {0};
	int nLen;
	memset(szBuff,0,sizeof(szBuff));
	cmd = ANDROID_GETAPP;
	m_start=0;
	if (ARQSend(&cmd, 1) < 0) 
	{
		CFunCommon::DlSprintf("Failed to Get AppInfo1.\n");
		return false;
	}
	memset(szBuff,0,sizeof(szBuff));
	if ((nLen=ARQRecv(szBuff, sizeof (szBuff), 5000)) < 0) 
	{
		CFunCommon::DlSprintf("Failed to Get AppInfo2.\n");
		return false;
	}
	if (szBuff[0] != 0) 
	{
		CFunCommon::DlSprintf("Failed to Get AppInfo3.\n");
		return false;
	}
	else
	{
		if (nLen>1)
		{
			memcpy(szTmp,&szBuff[1],nLen-1);
			strAppList = szTmp;
			return true;
		}
	}
	return false;
}

/**
 * @brief	下载android应用
 * @param[in] pszFileList  待下载文件列表
 * @return
 * @li 0 成功
 * @li -1 失败
 */
int CARQ::Android_DownLoad_APP(const char* pszFileList)
{
	unsigned char szTotal[1024*16] = {0};						
	char szDownFullPath[MAX_DOWNFILE_NUM][MAX_PATH] = {0} ;
	char szDownFileName[MAX_DOWNFILE_NUM][MAX_PATH] = {0};
	char	szTmp[4096] = {0};
	int szFileType[MAX_DOWNFILE_NUM] = {0},		
		nTotalFileCount = 0,		
		nDownFileLen = 0,			
		nLen = 0,
		nPos = 0,
		nReadLen = 0,
		nCrtLen = 0,
		i,j,nOffset = 0,nRet = 0,a = 0,nProgress = 0;

	char szBuff[1024*64 + 1] = {0};
	float fProgress = 0.0f;
	unsigned char ucFileNum;
	unsigned char cmd ;

	FILE *f1 = NULL;


	memset(szTotal,0,sizeof(szTotal));
	memset(szBuff,0,sizeof(szBuff));
	
	strcpy(szBuff,pszFileList);
	nDownFileLen = strlen(szBuff);
	nTotalFileCount = 0;
	nOffset = 0;
	//m_start = 0;

	

//	CFunCommon::DlSprintf("pszFileList = %s\n",pszFileList);
	for (i=0; i<nDownFileLen; i++)  
	{
		if ('*' == szBuff[i])
		{
			szBuff[i] = '\0';
			strcpy(szDownFullPath[nTotalFileCount],szBuff+nOffset);
			szBuff[i] = '*';

			for (j=strlen(szDownFullPath[nTotalFileCount])-1; j>0; j--)
			{
				if ('/' == szDownFullPath[nTotalFileCount][j])
				{
					strcpy(szDownFileName[nTotalFileCount],szDownFullPath[nTotalFileCount]+j+1);
					break;
				}
			}

			for (j=i;;j++)
			{
				if ('|' == szBuff[j])
					break;
			}
			memset(szTmp,0,sizeof(szTmp));
			memcpy(szTmp,szBuff+i+1,j-i-1);
			szFileType[nTotalFileCount] = atoi(szTmp);
			nOffset = j+1;			
			nTotalFileCount++;
		}
	}


	//下载
	//CFunCommon::DlSprintf("nTotalFileCount = %d,szFileType[]=%d,szDownFullPath=%s\n",nTotalFileCount,szFileType[0],szDownFullPath[0]);
	for (i=0; i<nTotalFileCount; i++)
	{		

		
		if ((0 == szFileType[i] )|| (1 == szFileType[i]) || (2 == szFileType[i]))  
		{

		//	CFunCommon::DlSprintf("Downloading %s\n",szDownFullPath[i]);
		
			//NLP Down
			if (NULL == (f1 = fopen(szDownFullPath[i], "rb"))) 
			{
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -2;
			}
			memset(szTotal,0,sizeof(szTotal));

			if (1 == szFileType[i]) 
				nLen = 456 + 256;
			else if (0 == szFileType[i])
				nLen = 256;		
			fseek(f1, 13,SEEK_SET );
			fread(&ucFileNum, 1, 1, f1);
			memset(szTmp,0,sizeof(szTmp));
			fread(szTmp, 1, 2, f1);
	
			for (j = 0; j < ucFileNum; j ++) 
			{
				if (szFileType[i]==2)
				{
					szTotal[0] = 7;//x509证书
					fread(szTotal+1, 1, 4, f1);
					memcpy(&nCrtLen,szTotal+1,4);
					fread(szTotal+5, 1, nCrtLen+256, f1);
					nLen=nCrtLen+256+4;					
					fread(&nDownFileLen, 1, 4, f1);
				}
				else
				{
				
					szTotal[0] = 1;
					fread(szTotal + 1, 1, nLen, f1);
					fread(&nDownFileLen, 1, 4, f1);
				}

				nPos=0;
				nRet=ARQSend((unsigned char *)szTotal, nLen + 1);		
				if ( nRet < 0) 
				{
					CFunCommon::DlSprintf("Failed:Communication error\n ");  
					nRet=DOWN_ERR_SENDERROR;
					fclose(f1);
					return -1;
				}
				
				nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);		
				if ( nRet < 0 || 0 != szTotal[0]) 
				{				
					if(nRet < 0)
							CFunCommon::DlSprintf("Failed:recv timeout");
					else
							CFunCommon::DlSprintf("Failed: ErrorInfo:%s\n ",g_szMessage[szTotal[0] + 86] );
					fclose(f1);
					return -1;
				}
				else 
				{				
					nPos=0;
					nReadLen = nDownFileLen;
					while (nDownFileLen) 
					{
						nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;
						memset(szTotal,0,sizeof(szTotal));
						fread(szTotal,1,nReadLen, f1);
						
						FlushPort();
						nRet = ARQSend((unsigned char *)szTotal, nReadLen);
						if ( nRet< 0) 
						{
							//CFunCommon::DlSprintf("Failed:DOWN_ERR_SENDERROR error \n ");
							fclose(f1);
							return -1;
						}

						nRet = ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT);
						if (nRet < 0 || 0 != szTotal[0] ) 
						{			
							if(nRet < 0)
								CFunCommon::DlSprintf("Failed:recv timeout");
							else
								CFunCommon::DlSprintf("Failed: ErrorInfo:%s\n ",g_szMessage[szTotal[0] + 86] );
							fclose(f1);
							return -1;
						}

						nDownFileLen -= nReadLen;
						nPos+=nReadLen;
						fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
						nProgress=fProgress;
						  
					//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
						(*gCallBack)(szDownFullPath[i],nProgress);
						usleep(10);
					}	
				//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);
					(*gCallBack)(szDownFullPath[i],100);
					
					nRet=ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) ;
					if (nRet < 0 || 0 != szTotal[0] ) 
					{			
							if(nRet < 0)
								CFunCommon::DlSprintf("Failed:recv timeout");
							else
								CFunCommon::DlSprintf("Failed:ErrorInfo:%s\n ",g_szMessage[szTotal[0] + 86] );
							
							fclose(f1);
							return -3;
					}
				}
			}
			fclose(f1);
		}
		else if (8 == szFileType[i])
		{
		//	CFunCommon::DlSprintf("Downloading %s\n",szDownFullPath[i]);
			
		    if (NULL == (f1 = fopen(szDownFullPath[i], "rb")) ) 
			{
				
				CFunCommon::DlSprintf("Failed to open file %s\n",szDownFullPath[i]);
				return -1;
			}
			memset(szTotal,0,sizeof(szTotal));
			szTotal[0] = 2;
			fseek(f1, 0L, SEEK_END);  
    		nDownFileLen = ftell(f1); 
			fseek(f1, 0L, SEEK_SET);  
			memcpy(szTotal + 1, &nDownFileLen, 4);
		
			strcpy((char *)szTotal + 5, szDownFileName[i]);
			nLen = strlen((char *)szTotal + 5) + 1 + 4 + 1;
		
			if (ARQSend((unsigned char *)szTotal, nLen) < 0) 
			{
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_SENDERROR]);
				fclose(f1);
				return -1;
			}
	
			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
			{
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_RECVERROR]);
				fclose(f1);
				return -1;
			}
			if (0 != szTotal[0]) 
			{
				a = szTotal[0] + 86;
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[a]);
				fclose(f1);
				return -2;
			}
		
			nPos=0;
			nReadLen = nDownFileLen;
			while (nDownFileLen) 
			{
				nReadLen = nDownFileLen > sizeof (szTotal) ? sizeof (szTotal) : nDownFileLen;
				fread(szTotal, 1, nReadLen, f1);	
			//	CFunCommon::DlSprintf("nReadLen = %d\n",nReadLen);
				if (ARQSend((unsigned char *)szTotal, nReadLen) < 0) 
				{
					CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_SENDERROR]);
					fclose(f1);
					return -4;					
				}
				if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
				{
					CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_RECVERROR]);
					fclose(f1);
					return -1;					
				}
				if (0 != szTotal[0] ) 
				{
					a = szTotal[0] + 86;
					CFunCommon::DlSprintf("Failed:%s",g_szMessage[a]);
					fclose(f1);
					return -1;
				}
				nDownFileLen -= nReadLen;
				nPos += nReadLen;
				fProgress=0.1f*nPos*1000/(nDownFileLen+nPos);
				nProgress=fProgress;

			//	CFunCommon::DlSprintf("   --> Downloading......   %3d%%\033[1A\r\n",nProgress); 
				(*gCallBack)(szDownFullPath[i],nProgress);
				usleep(30*1000);
				
			}
		//	CFunCommon::DlSprintf("       --> Downloading......   %d%%\n",100);
			(*gCallBack)(szDownFullPath[i],100);	
			//CFunCommon::DlSprintf("Checking Data ......\n");
		
			if (ARQRecv((unsigned char *)szTotal, sizeof (szTotal), NLP_FIRM_TIMEOUT) < 0) 
			{
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[DOWN_ERR_RECVERROR]);
				fclose(f1);
				return -1;
			}

			if (0 != szTotal[0]) 
			{	
				a = szTotal[0] + 86;
				CFunCommon::DlSprintf("Failed:%s",g_szMessage[a]);
				fclose(f1);
				return -1;
			}
			else 
			{
				fclose(f1);
			}
		}
	}
	//对于pos端，发送无可以下载的包0x01，同步时间0x03,清空应用0x04都是结束下载流程，所以涉及到若无可下载的包，发送01，用于与正常结束区分
	if (0 == nTotalFileCount )
	{
		unsigned char cmd = 0x01;
		if (ARQSend(&cmd, 1) < 0) 
		{
			return -4;		
		}
		return 1;
		
	}
	
	
	//清空应用
	cmd  = 0x00;
	if (m_bClearApp)
	{
		cmd |= ANDROID_CLEAR;
//		cmd |= ANDROID_INTERRUPT;
		if (ARQSend(&cmd, 1) < 0) 
		{
			CFunCommon::DlSprintf("Failed to clear app.\n");
			return -5;		
		}
		
	}

	memset(szTotal,0,sizeof(szTotal));
	szTotal[0] = 3;
	time_t unix_tm;
	time( &unix_tm );
	unix_tm = unix_tm + 8*3600; 
	memcpy(szTotal + 1, &unix_tm, sizeof (unix_tm));
	if (ARQSendMposEnd((unsigned char *)szTotal, 1 + sizeof (unix_tm)) < 0) 
	{
		CFunCommon::DlSprintf("Failed to synchronize  time.\n");
		return -4;
	}

	return 0;
}