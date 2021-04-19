// #include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

// #include <utils/Log.h>
#include <android/log.h>

#define _DEBUG 0
#ifdef _DEBUG
#define LOGI(...) __android_log_print(ANDROID_LOG_VERBOSE,"sdtp",__VA_ARGS__)
// ANDROID_LOG_DEBUG
#else
#define LOGI(...) 	;
#endif

#define K21PORT "/dev/ttyHSL3"

#define		ERROR_FW_FORMAT			(-3)
#define		ERROR_SWITCH_BOOT		(-4)
#define		ERROR_BOOT_MODE_OPEN	(-5)
#define		ERROR_BOOT_MODE_TEST	(-6)
#define		ERROR_BOOT_FW_OPEN		(-7)
#define		ERROR_BOOT_FW_SIGN		(-8)
#define		ERROR_BOOT_FW_SEND		(-9)
#define		ERROR_BOOT_FW_VERIFY	(-10)

#define uint8_t  unsigned char
#define uint32_t  unsigned int
#define uint16_t	unsigned short
// #define NULL	0

//#define MAX_FRAME_SIZE	(1024*4)
#define MAX_FRAME_SIZE	(1024*16)
#define FRAME_LEN 	(2+2+MAX_FRAME_SIZE)


#define SDTP_ABORT              -9999

typedef uint8_t seq_nr_t;       /* Frame Seq type */
typedef uint8_t frame_type_t;   /* Frame type type */

/** Frame structure, not including FCS  */
typedef struct {
    frame_type_t type;      /* Frame type */
    seq_nr_t seq_nr;        /* Frame seq */
    uint8_t data[1];        /* Frame data */
}frame_t;

typedef struct _sdtp_t{
    int frame_size;             /* Maximum Frame Size */
    int timeout_msec;           /* Timeout between two bytes, ms */
    int retry;                  /* Frame Retry Times */
    seq_nr_t start;             /* Current Frame Seq */
    int syn_req;                /* require seq_nr to sync */
    int no_fcs;                 /* FCS unnecessary */
    frame_t * in;               /* Buffer for Receiving */
    frame_t * out;              /* Buffer for Sending */
    frame_t * acknak;           /* Buffer for ACK/NAK */
    int (*_put)(int fd, unsigned char * buf, int len);        /* Send Data: len bytes */
    int (*_get)(int fd, int timeout);   /* Receive Data: one byte */
    int fdCom;	/* 通讯口对应的文件描述符 */
}sdtp_t;


#define MAX_SEQ_NR 255  /* Frame Seq, 255 max */

#define FSTX 0x7E       /* Frame Start Character */
#define FDLE 0x7D       /* Escape Character */

#define FDATA   0x02    /* Data Frame */
#define FACK    0x01    /* ACK Frame */
#define FNAK    0x40    /* NAK Frame */
#define FSYN    0x04    /* Frame Seq Syn Advice, from host, slave do not */

#define IS_FACK(frame)          ((frame)->type & FACK)
#define IS_FNAK(frame)          ((frame)->type & FNAK)
#define IS_FDATA(frame)         ((frame)->type & FDATA)

#define FRAME_HEAD 2
#define FRAME_HOLD (FRAME_HEAD + 2)

#define IS_PREVIOUS_SEQ(a, b) (((uint8_t)((a) - 1)) == (b))


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

unsigned short crc16( unsigned char *buffer, int len)
{
	unsigned short crc = 0;

	if (buffer == NULL) return crc;

	while (len--) {
		crc = crc16_byte(crc, *buffer++);
	}
	return crc;
}

int _reccomm(int fd, int len, char *outbuf, int timeout)
{
    struct timeval tv;
    fd_set readfds;
    int nfds = -1;
    int flag = 0;
    int readcnt=0;
    int ret = 0;

    struct  timeval    tv1;
    struct  timeval    tv2;
    struct  timeval    tv3;
    struct  timezone   tz;
    gettimeofday(&tv1,&tz);

    int iii = 0;

	LOGI("--------receive with len:%d,timeout:%d\n",len,timeout);
    while(1){
    	if (!flag){
    		tv.tv_sec=timeout/1000;
			tv.tv_usec=(timeout%1000)*1000;
    	}else{
    		tv.tv_sec = 0;
			tv.tv_usec = 1000;
    	}

	    FD_ZERO(&readfds);
	   	FD_SET(fd, &readfds);

	   	nfds=select(fd+1, &readfds, NULL, NULL, &tv);

	   	if (nfds < 0) {
			if (EINTR == errno) {

				continue;
	        }
			LOGI("--------receive err 1\n");
	        return -1;
	    }else if (nfds > 0) {
	    	if(readcnt == 0){
	    		gettimeofday(&tv2,&tz);
	    	}
	    	ret = read( fd, outbuf + readcnt, len-readcnt);
	        if (FD_ISSET(fd, &readfds)) {
	        	if (ret < 0) {
	        		LOGI("1------------%ld, %ld,%ld, %ld", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec);
					LOGI("--------receive err 2\n");
	        		return -1;
	        	} else if (0 == ret) {
	        		LOGI("2------------%ld, %ld,%ld, %ld", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec);
					LOGI("--------1 receive with len:%d\n",len);
	            	return len;
	        	} else if (ret < (len-readcnt)) {
	        		if(readcnt == 0){
	        			gettimeofday(&tv3,&tz);
	        		}
	        		iii++;
					readcnt += ret;
					flag = 1;
	                continue;
	            }else{
	            	LOGI("3------------%ld, %ld,%ld, %ld", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec);
					LOGI("--------2 receive with len:%d\n",len);
	            	return len;
	            }
	        }
	    } else {
	        if (0 == readcnt) {
	        	LOGI("4------------%ld, %ld,%ld, %ld", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec);
				LOGI("--------receive err 3\n");
	        	return -1;
	        }else{
	        	LOGI("5------------%ld, %ld,%ld, %ld, %ld, %ld, %d", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec, tv3.tv_sec, tv3.tv_usec, iii);
				LOGI("--------3 receive with len:%d\n",readcnt);
	        	return readcnt;
	        }
	    }
    }
}

unsigned char dataBuffer[MAX_FRAME_SIZE*2];
int dataBufferLen=0;
int dataBufferOffset=0;
int framRecFlag=0;	//0，表示用户重新发起读一帧数据；1，原数据缓冲区存在buf，先读数据缓冲区数据；2，数据buf空，进行过实际物理串口读取

int getcTimeout(int fd, int timeout)
{
	int len;
	unsigned char c;

	if (dataBufferOffset>=dataBufferLen){
		if (framRecFlag==2){//串口已经读过，不再读取
			LOGI("framRecFlag==2");
			return -1;
		}else if (framRecFlag==0){//数据缓冲在上一帧已经读取完成，按用户要求时间设置超时
			len = _reccomm(fd, sizeof(dataBuffer), dataBuffer, timeout);
			LOGI("framRecFlag==0, len = %d", len);
		}else if (framRecFlag==1){//上一帧读取完成后，有遗留数据，只判断10ms内是否还能收到数据
			len = _reccomm(fd, sizeof(dataBuffer), dataBuffer, 10);
			LOGI("framRecFlag==1, len = %d", len);
		}

		if (len>0){
			dataBufferOffset = 0;
			dataBufferLen = len;
		}else{
			dataBufferOffset = 0;
			dataBufferLen = 0;
			return -1;
		}

		framRecFlag = 2;
	}

	if (framRecFlag == 0){
		framRecFlag = 1;
	}

	c = dataBuffer[dataBufferOffset];
	dataBufferOffset ++;

//	LOGI("===== c =====%d", c);
	return c;
}

#if 0
int getcTimeout(int comm_fd, int timeout_ms)
{
    uint8_t ch;
    do
    {
    	if (read(comm_fd, &ch, 1 ) == 1 ) return ch;
    	usleep(1000);
    }while(--timeout_ms >= 0);
    return -1;
}
#endif

int writeK21(int fd,  unsigned char* buf , int len)
{
	return write(fd,buf,len);
}
static int packet_a_frame(sdtp_t * pro, frame_type_t type, uint8_t * data, int datalen)
{
    uint8_t * pframe;
    uint16_t crc;
    if (pro->syn_req) {
        type |= FSYN;
    }
    if (type & FDATA) {
        pro->out->type = type;
        pro->out->seq_nr = pro->start;
        memcpy(pro->out->data, data, datalen);
        pframe = (uint8_t *)(pro->out);
    }
    else { /* ACK or NAK */
        datalen = 0;
        pro->acknak->type = type;
        pro->acknak->seq_nr = pro->start;
        pframe = (uint8_t *)(pro->acknak);
    }
    if (pro->no_fcs) crc = 0x0000;
    else crc = crc16(pframe, datalen + FRAME_HEAD);
    *(pframe + datalen + FRAME_HEAD) = crc & 0x00FF;
    *(pframe + datalen + FRAME_HEAD + 1) = (crc >> 8) & 0x00FF;
    return (datalen + FRAME_HOLD);
}

static int recv_a_frame(sdtp_t * pro, int timeout)
{
    unsigned char * pdata = (unsigned char *)pro->in;
    unsigned char recvdata;
    int flag = 0, ret, len = 0, escape = 0;

    framRecFlag = 0;

    /**
     * flag = 0 we haven't met FSTX, =1 frame end
     * escape = 0 normal character, = next one needs escaping
     */
    do {
        if ((ret = pro->_get(pro->fdCom, timeout)) < 0) {
  			 LOGI("recv_a_frame  : timeout");
        	break;
        }
//        LOGI("+++++++ ret = %d", ret);
        recvdata = ret;
        if (recvdata == FSTX) {  /* Frame Start Character */
            if (flag == 1) { /* frame end  第二个表示帧结束了 */
                ret = len;
//                LOGI("222222222222222222222222222");
                /* if frame too short, it is invalid */
                break;
            }
            if (flag == 2) { /* last frame data error */
   			 LOGI("recv_a_frame  : last frame data error");
                flag = 0; /* we want first FSTX again */
                continue;
            }
//            LOGI(" 11111111111111111111111111 ");
            flag = 1; /* see first FSTX 第一个7e表示帧开始了 */
        }
        else if (flag == 1) { /* frame data processing */
            if (recvdata == FDLE) {  // 第一个7d表示转义
                escape = 1;
                continue;
            }
            if (escape == 1) {	// 转义
                recvdata = (uint8_t)((0x7D & 0xF0) | (recvdata & 0x0F));
                escape = 0;
            }
            if(len > (pro->frame_size + FRAME_HOLD))/*over flow, abandon it*/ {
                len = 0;
       		 LOGI("recv_a_frame  : over flow, abandon it");
                pdata = (unsigned char *)pro->in;
                flag = 2; /* until new FSTX */
            }
            else {
                *pdata ++ = recvdata;
                len ++;
            }


        }
        else  if (flag == 0) { // 没有帧头
 			 LOGI("recv_a_frame  : 没有帧头");
            flag = 2; /* until new first FSTX */
        }
    }while (1);
    return ret;
}

static int send_a_frame(sdtp_t * pro, frame_type_t type, int hold)
{
    uint8_t * psend = (type & FDATA) ? (unsigned char *)(pro->out) :
            (unsigned char *)(pro->acknak);

#if 0		//一个字节一个字节发送
    if (pro->_put(FSTX) < 0) return -1;
    while (hold) {
        if (*psend == FSTX) {
            if (pro->_put(FDLE) < 0) return -1;
            if (pro->_put(0x5E) < 0) return -1;
        }
        else if (*psend == FDLE) {
            if (pro->_put(FDLE) < 0) return -1;
            if (pro->_put(0x5D) < 0) return -1;
        }
        else {
            if (pro->_put(*psend) < 0) return -1;
        }
        psend ++;
        hold --;
    }
    if (pro->_put(FSTX) < 0) return -1;
#else
	char _buf[1024*8];
	register char *p = _buf;
	int i;

	*p++ = FSTX;
	for(i=0; i<hold; i++) {
		register unsigned char ch = psend[i];
		if (ch == FSTX) {
			*p++ = FDLE;
			*p++ = 0x5E; //m_es1;
		}
		else if (ch == FDLE) {
			*p++ = FDLE; // m_FDLE;
			*p++ = 0x5D;	//m_es2;
		}
		else {
			*p++ = ch;
		}
	}
	*p++ = FSTX;	// m_FSTX;
	pro->_put(pro->fdCom, (unsigned char *)_buf, p-_buf);	// WriteComm((unsigned char *)_buf, p-_buf);
#endif
    return 0;
}


static int frame_fcs_check(uint8_t * frame, int len)
{
    uint16_t crc_req, crc_cal;
    len -= 2;
    crc_cal = crc16(frame, len);
    crc_req = (*(frame + len)) | ((*(frame + len + 1)) << 8);
    return (crc_cal == crc_req);
}
/*与nlsdtp库重复定义 与nlsdtp库重复定义 */
int sdtp_send(sdtp_t * arq, uint8_t * data, int datalen)
{
    int len, retry, relen, sendlen, totallen = datalen;
    unsigned char * pdata = data;

    if (arq == NULL || data == NULL || datalen <= 0) return 0;
    sendlen = arq->frame_size;

    while (datalen >= 0) {
        retry = arq->retry;
        sendlen = datalen > arq->frame_size ? arq->frame_size : datalen;
        len = packet_a_frame(arq, FDATA, pdata, sendlen);
        do {
       	 LOGI("sdtp_send  DATA  :%d sn-%d ",len,arq->start);
        	if (send_a_frame(arq, FDATA, len) < 0) {
                return -1;
            }
recv:
            if ((relen = recv_a_frame(arq, arq->timeout_msec)) <= 0) {
                continue;
            }
            if (arq->no_fcs || frame_fcs_check((uint8_t *)(arq->in), relen)) {
                if (arq->in->seq_nr == arq->start) {
                    if (IS_FACK(arq->in)) {
                    	 LOGI("sdtp_send  _ACK  : sn-%d ",arq->start);
                        break;
                    }
                    if (IS_FNAK(arq->in)) {
                   	 LOGI("sdtp_send _NACK  :%d sn-%d ",len,arq->start);
                        continue;
                    }
                    goto recv;
                }
                else if (IS_FDATA(arq->in) &&
                    (IS_PREVIOUS_SEQ(arq->start, arq->in->seq_nr))) {
                	LOGI("sdtp_send _FDATA  :%d  sn-%d re-%d",len,arq->start, arq->in->seq_nr);
                    arq->start --;
                    len = packet_a_frame(arq, FACK, NULL, 0);
                    arq->start ++;
                    if (send_a_frame(arq, FACK, len) < 0) {
                        return -4;
                    }
                    goto recv;
                }
                else {
                    goto recv;
                }
            }

        }while (--retry > 0);
        if (retry == 0) {
            return -2;
        }
        arq->start ++;
        pdata += sendlen;
        datalen -= arq->frame_size;
    }
    return totallen;
}

int sdtp_recv(sdtp_t * arq, uint8_t * buff, int bufflen, int timeout)
{
    int relen, sndlen, len = 0, dlen;
    seq_nr_t old;
    if (arq == NULL) return 0;

    while (1) {
        if ((relen = recv_a_frame(arq, timeout)) <= 0) {
            return relen;
        }
        if (arq->no_fcs || frame_fcs_check((uint8_t *)(arq->in), relen)) { // 校验正确
            if (IS_FDATA(arq->in)) {// 是数据包
            	LOGI("sdtp_recv  FDATA  :%d  sn-%d insn-%d",relen,arq->start, arq->in->seq_nr);
                if (arq->in->type & FSYN) arq->start = arq->in->seq_nr;  // 数据包+同步位
                if (arq->in->seq_nr == arq->start) { // 帧号对应
                    sndlen = packet_a_frame(arq, FACK, NULL, 0); // ACK之
                	LOGI("sdtp_recv  s_ACK size%d",sndlen);
                    if (send_a_frame(arq, FACK, sndlen) < 0) {
                        return -1;
                    }
                    arq->start ++;
                    relen -= FRAME_HOLD;
                    if (bufflen) {
                        dlen = relen > bufflen ? bufflen : relen;
                        memcpy(buff + len, arq->in->data, dlen); // 拷贝数据到buff
                        bufflen -= dlen;
                    }
                    len += relen;
                    if (relen < arq->frame_size) {
                        return len;
                    }
                }
                else if (IS_PREVIOUS_SEQ(arq->start, arq->in->seq_nr)) { // 收到的是前一个帧
                	LOGI("sdtp_recv  s_NAK IS_PREVIOUS_SEQ   :%d  sn-%d insn-%d",relen,arq->start, arq->in->seq_nr);
                	arq->start --;
                    sndlen = packet_a_frame(arq, FACK, NULL, 0); //组ACK包,对应帧号是前一个
                    arq->start ++; // 恢复目前SN号
                    if (send_a_frame(arq, FACK, sndlen) < 0) {
                        return -4;
                    }
                }

            }
        }
        else { // CRC出错了
            old = arq->start;
            arq->start = arq->in->seq_nr;	// 用收到的SN
            sndlen = packet_a_frame(arq, FNAK, NULL, 0); // 组NAK包
            arq->start = old; // 恢复等待的SN
        	LOGI("sdtp_recv  s_NACK size%d",sndlen);
            if (send_a_frame(arq, FNAK, sndlen) < 0) {
                return -2;
            }
        }
    }
    return 0;
}

// 检测固件文件格式.如果TAG为(256证书+312签名),返回568;如果是(256签名)返回256,否则返回-1
int getFWtagLen(int fdFw)
{
	int  position;
	unsigned int len,datalen;
	unsigned char num;
	int i,j;
	int tlen[2] = {256, (256+312)};

	position = lseek(fdFw,0,SEEK_END);	//	fl.Seek(13, CFile::begin);
	LOGI( "固件文件长度%d",position);

	if ( position <= (256+16+4) ) // 最小
	{
		return -1;
	}
	len = position; // len为有效文件总长度

	lseek(fdFw,13,SEEK_SET);	//
	read(fdFw,&num, 1);	// 第13位是num, 文件块数

	// 先尝试TAG为(256字节签名)格式
	for (j= 0; j < 2; j ++)
	{
		lseek(fdFw,16,SEEK_SET);	// ~到16位
		// LOGI( "尝试TAG长度%d",tlen[j]);
		for (i = 0; i < num; i ++)
		{
			position = lseek(fdFw,tlen[j],SEEK_CUR); //跳过  TAG区,大小 tlen[j]
			if( (position +4)  >=len ) break;

			read(fdFw,&datalen, 4);	//  datalen在这里,即正式固件长, 4B
			if( (position +datalen)  >len ) break;

			position = lseek(fdFw,datalen,SEEK_CUR); //跳过 数据区  大小datalen

		}
		if(( i == num )&&(position == len))
		{	// 格式吻合
			LOGI( "TAG长度%d",tlen[j]);
			return tlen[j];
		}
	}

			LOGI( "文件格式不合");
			return -1;

}

/**********************************************************************
* 函数名称： k21_IOCtrl
***********************************************************************/
#define BOOTUP	((int)0)
#define UPDATE	((int)1)
int k21_IOCtrl(int mode, const char* flag)
{
	int fd = -1;
	// fd = open("/sys/class/paymodule_k21/bootup", O_RDWR );

	switch(mode)
	{
		case BOOTUP:
			fd = open("/sys/class/paymodule_k21/bootup", O_RDWR );
			break;

		case UPDATE:
			fd = open("/sys/class/paymodule_k21/update", O_RDWR );
			break;

	}
	if(fd > 0)
	{
		write(fd,flag,1);
		close(fd);
		return 0;
	}

	return (-1);
}

int dump_ram(unsigned char* p,int len)
{
	int i,j,k;
	j = len%16;

	for(i=0,k=0; i< len/16  ; i++)
	{
		LOGI("l%d: %x %x %X %x, %x %x %X %x, %x %x %X %x, %x %x %X %x",i+1,p[k],p[k+1],p[k+2],p[k+3],p[k+4],p[k+5],p[k+6],p[k+7],p[k+8],p[k+9],p[k+10],p[k+11],p[k+12],p[k+13],p[k+14],p[k+15]);
		k += 16;
	}

	if(j != 0)
	{
	LOGI("tail:");
	LOGI("l%d: %x %x %X %x, %x %x %X %x, %x %x %X %x, %x %x %X %x",i+1,p[k],p[k+1],p[k+2],p[k+3],p[k+4],p[k+5],p[k+6],p[k+7],p[k+8],p[k+9],p[k+10],p[k+11],p[k+12],p[k+13],p[k+14],p[k+15]);
	}

	return len;

}

int
sdtp_updateApp(int fdFw, int fdJNI, int mode)
{

	int iRet=0;

	int iLenWr,iLenRd,iLenRdOk,i, j;
	int hlen;


  	unsigned char cmd, buff[MAX_FRAME_SIZE];
  	char  *pbuf;

  	sdtp_t sdtp;
  	char inbuf[FRAME_LEN],outbuf[FRAME_LEN],ackbuf[FRAME_LEN];
    /* SDTP Parameters Setup */
    sdtp.frame_size = MAX_FRAME_SIZE; // master->frame_size;
    sdtp.no_fcs = 0; //需要crc   master->no_fcs;
    sdtp.retry = 5; // 3;
    sdtp.start = 0;
    sdtp.syn_req = 0;
    sdtp.timeout_msec = 1000;	//150;
    sdtp._get = getcTimeout;
    sdtp._put = writeK21; // write;
    sdtp.in = (frame_t *)inbuf;
    sdtp.out = (frame_t *)outbuf;
    sdtp.acknak = (frame_t *)ackbuf;
    sdtp.fdCom = fdJNI;		// K21对应端口

	LOGI("into MAPP download mode%d! fd %d ",mode,fdJNI);

	hlen = getFWtagLen(fdFw); // 通过判断Tag长度判断固件的格式合法性
	if( hlen < 0)
	{ //  固件格式错
			iRet = (ERROR_FW_FORMAT);	 goto end;
	}

	if((mode == 100)||(mode == 120))
	{
	 // 先下电,再发".",再上电,期间不停发送'.',持续5秒
	 ioctl(fdJNI,0X540b,2);//  ,TCFLSH,TCIOFLUSH); // 清空串口数据
	 k21_IOCtrl(BOOTUP,"0");
	 k21_IOCtrl(UPDATE,"0");
	 for(i=0, j=500, iLenWr=0; i<8001; i+=j)	// j表示间隔的ms数
	 {
		if (i > 500) iLenWr += write(fdJNI,"..........",1);
		usleep(j*1000);	// 等待100ms，
		iLenRd =  read(fdJNI, buff, 1);
		LOGI("read %d = %c",iLenRd,buff[0]);
		if((iLenRd > 0)||(buff[0] == '.')) break;

		if (i == 500)	{k21_IOCtrl(BOOTUP,"1");}
		if ((i % 1000)==0) LOGI("wait reboot... %ds...send %d",i/1000,iLenWr);
	 }
		if (mode == 120) return (-11);
	}
	ioctl(fdJNI,0X540b,2);//  ,TCFLSH,TCIOFLUSH); // 清空串口数据



	cmd = 0;	// ~发版本查询命令
	if (sdtp_send(&sdtp, &cmd, 1) < 0) {
		LOGI("数据发送失败");
		iRet = (ERROR_BOOT_MODE_TEST);		goto end2;//  goto end;// goto end2;
	}
	if (sdtp_recv(&sdtp, buff, sizeof (buff), 1000) < 0) {
		LOGI("数据接收失败");
		iRet = (ERROR_BOOT_MODE_TEST);		goto end2;//  goto end;// goto end2;
	}
	if (buff[0] != 0) {
		sprintf((char *)buff, "错误码:[%02X]", buff[0]);
		LOGI((char *)buff, 0);
		iRet = (ERROR_BOOT_MODE_TEST);		goto end2;//  goto end;// goto end2;
	}
	else
		LOGI("master版本号 %s", (char *)buff + 1);

	pbuf = (char *)buff + 1;
	LOGI("设备检测 ... ... 已连接");

	unsigned int len, rlen, tlen;
	unsigned char num;
	/* hlen的计算移到函数开始,提早判断免得误进入下载模式
	int hlen;
	// hlen = 312 + 256; 	// 第一次发包: 证书+签名
	hlen = getFWtagLen(fdFw);// 256;
	if( hlen < 0){
		iRet = (-8);	 goto end;
	}*/

	lseek(fdFw,13,SEEK_SET);	//	fl.Seek(13, CFile::begin);
	read(fdFw,&num, 1);	// fl.Read(&num, 1);	// ~第13位是num, 文件数
	lseek(fdFw,2,SEEK_CUR);	// fl.Seek(2, CFile::current); //~到14+2=16位?
	for (i = 0; i < num; i ++)
	{
		LOGI("等待终端处理 数据块%d...", i);
		buff[0] = 1;	// 表示数据~
		iLenRd = read(fdFw,buff + 1, hlen);	// fl.Read(buff + 1, hlen); // 读256字节
		if (iLenRd != hlen) {
			LOGI( "固件文件读取错误");
			iRet = (ERROR_BOOT_FW_OPEN);	goto end2;//  goto end;
		}
		read(fdFw,&len, 4);	// fl.Read(&len, 4); // len在这里,即正式固件长?
		// dump_ram(buff+1, hlen);
		if (sdtp_send(&sdtp, buff, hlen + 1) < 0) { // 发出这257的数据
			LOGI( "签名数据发送失败");
		}
		if (sdtp_recv(&sdtp, buff, sizeof (buff), 5000) < 0) {
			LOGI( "数据接收失败");
		}
		if (buff[0] != 0) {
			LOGI( "证书和签名解析错误");
			iRet = (ERROR_BOOT_FW_SIGN);	 goto end2;//  goto end;
		}
		else
		{
			LOGI( "证书和签名校验成功, 数据块%d大小%d",i, len);
			tlen = len;
			while (len) {
				usleep(100000);	// 等待100ms，
				ioctl(fdJNI,0X540b,2);//  ,TCFLSH,TCIOFLUSH); // 清空串口数据
				rlen = len > sizeof (buff) ? sizeof (buff) : len;
				iLenRd = read(fdFw,buff,rlen); // 读正式数据,按最大帧
				if (iLenRd != rlen) {
					LOGI( "固件文件读取错误 1");
					iRet = (ERROR_BOOT_FW_OPEN);	 goto end2;//  goto end;
				}
				if ((iLenWr = sdtp_send(&sdtp, buff, rlen) )< 0) { // 发出本帧数据~
					LOGI( "数据发送失败,总%d,发送%d,实际发出%d,剩余%d",tlen,rlen,iLenWr,len);
					iRet = (ERROR_BOOT_FW_SEND);	 goto end2;//  goto end;
				}
				if ((iLenRd = sdtp_recv(&sdtp, buff, sizeof (buff), 10000) )< 0) { // 接收应答~
					LOGI( "数据接收失败,总%d,发送%d,实际发出%d,剩余%d,读结果%d",tlen,rlen,iLenWr,len,iLenRd);
					iRet = (ERROR_BOOT_FW_SEND);	goto end2;//  goto end;
				}
				if (buff[0] != 0) { //读本帧结果
					LOGI( "数据下载处理错误,总%d,发送%d,剩余%d",tlen,rlen,len);
					iRet = (ERROR_BOOT_FW_SEND);	goto end2;//  goto end;
				}
				len -= rlen;
				LOGI( "下载进度: +%d:%d%%", rlen, (tlen - len) * 100 / tlen);
			}
			LOGI( "校验...");
			if (sdtp_recv(&sdtp, buff, sizeof (buff), 5000) < 0) { // 再读什么-升级结果
				LOGI( "下载结果接收失败");
				iRet = (ERROR_BOOT_FW_VERIFY);	goto end2;//  goto end;
			}
			if (buff[0] != 0) {
				LOGI("失败:%d", buff[0]); // tip.Format("失败:%d", buff[0]);
				// LOGI( tip, 1);
				iRet = (ERROR_BOOT_FW_VERIFY);	goto end2;//   goto end;
			}
			else LOGI( "成功,请移除设备");
			/* by cz : 移到循环外
			buff[0] = 0x03;	// 重启命令 -每下载一块正文都得重启一次!
			LOGI( "重启设备...");
			if (sdtp_send(&sdtp, buff, 1) < 0) {
				goto end2;
			*/
			}// 证书和签名校验通过

	} // for (i = 0; i < num; i ++)

//	if(fdJNI_acm0 > 0) {
//		close(fdJNI_acm0);
//	}
end2:
	k21_IOCtrl(BOOTUP,"0");
	usleep(500000);	// 等待500ms，
	k21_IOCtrl(UPDATE,"0");
	usleep(500000);	// 等待500ms，
	ioctl(fdJNI,0X540b,2);//  ,TCFLSH,TCIOFLUSH); // 清空串口数据
	k21_IOCtrl(BOOTUP,"1");
	usleep(500000);	// 等待500ms，
	ioctl(fdJNI,0X540b,2);// TCFLSH,TCIOFLUSH); // 清空串口数据
end:

	usleep(500000);	// 等待K21稳定，500ms，

	return iRet;

}

int inUpdateFile = 0;
int
sdtp_updateFile(char *posInfo, int posInfoLen, char *pdir)
{
	LOGI("sdtp_updateFile %s", posInfo);

	inUpdateFile = 1;
	int comm_fd;	// PC端串口
    int file_fd;	// 保存文件句柄
    int shakeHandSuccess = 0; //握手成功与否判断；0：未握手 1：已握手

    int len, rlen;

    unsigned char cmd, buff[MAX_FRAME_SIZE];
    char  *pbuf = buff;
//    char shakeHand[1] = {5}; // 4k
    char shakeHand[1] = {6}; //16k

    comm_fd = open("/dev/ttyGS0", O_NONBLOCK | O_RDWR);
    sdtp_t sdtp;
    char inbuf[FRAME_LEN],outbuf[FRAME_LEN],ackbuf[FRAME_LEN];
    /* SDTP Parameters Setup */
    sdtp.frame_size = MAX_FRAME_SIZE; // master->frame_size;
    sdtp.no_fcs = 0; //需要crc   master->no_fcs;
    sdtp.retry = 3; // 3;
    sdtp.start = 0;
    sdtp.syn_req = 0;
    sdtp.timeout_msec = 150;   //150;
    sdtp._get = getcTimeout; /* TODO */
    sdtp._put = writeK21;   /* TODO */
    sdtp.in = (frame_t *)inbuf;
    sdtp.out = (frame_t *)outbuf;
    sdtp.acknak = (frame_t *)ackbuf;
    sdtp.fdCom = comm_fd;     // PC端对应端口

//    LOGI("comm_fd %d ",comm_fd);

    /* TODO:Get 0x02 */
    char c;
    cmd = 0; /* 发送0x00 开始传输 */
    int ret = -1;

    while(inUpdateFile) { /* 0x02 + len(4 bytes) + name */

    	// 握手
    	if(shakeHandSuccess == 0){
    		sdtp_send(&sdtp, shakeHand, 1);
    	}

        /* TODO: Get a charactor:0x02 */
        ret = sdtp_recv(&sdtp, pbuf, sdtp.frame_size, 500);
        if(ret>0){
            LOGI("11111111111 comm_fd %d , ret = %d", comm_fd, ret);
            LOGI("pbuf: %s" ,pbuf);
            int i =0;
            for(i = 0; i < ret; i++){
            	LOGI("pbuf: %x" ,pbuf[i]);
            }
            LOGI("pbuf: %x" ,pbuf[0]);
            c = pbuf[0];
        	if(c == 0x00){ // 获取版本
        		LOGI("获取版本");
        		shakeHandSuccess = 1;
        		sdtp_send(&sdtp, posInfo, posInfoLen);
        	}else if(c == 0x02){ // 下载文件
        		LOGI("下载文件");
        		shakeHandSuccess = 1;
            	LOGI("22222222222");
                memcpy(&rlen, pbuf + 1, 4); /* 总长度  */

                // 获取下载的文件的 文件名
                char nameBuf[ret - 5];
                char *pnameBuf;
                strcpy(nameBuf, pbuf+5);
                pnameBuf = nameBuf;
                LOGI("pnameBuf %s", pnameBuf);
                LOGI("pdir %s", pdir);
                char *ppath = malloc(strlen(pdir)+strlen(pnameBuf));
                strcpy(ppath, pdir);
                strcat(ppath, pnameBuf);
                LOGI("ppath %s", ppath);

                file_fd = open(ppath, O_RDWR|O_CREAT );/* 创建文件 */
                LOGI("file_fd %d ",file_fd);
                sdtp_send(&sdtp, &cmd, 1); /* 发送0x00 开始传输 */
                LOGI("3333333");
                LOGI("len %d ",rlen);
                while (rlen) {
                    len = rlen > MAX_FRAME_SIZE ?
                    		MAX_FRAME_SIZE : rlen;
                    if (sdtp_recv(&sdtp, pbuf, len, 5000) != len) {
                    	LOGI("len %d ",len);
                    	LOGI("pbuf: %s" ,pbuf);
                        return -1;
                    }
                    LOGI("44444444");
                    if (write(file_fd, pbuf, len) != len) {
                        *pbuf = 0x04;
                        inUpdateFile = 0;
                        close(comm_fd);
                        return -2;
                    }
                    LOGI("55555555");
                    *pbuf = 0x00;
                    sdtp_send(&sdtp, pbuf, 1);
                    rlen -= len;
                    LOGI("rlen %d ",rlen);
                }
                LOGI("rlen = 0 , end while");
                *pbuf = 0x00;
                sdtp_send(&sdtp, pbuf, 1);
//                break;
        	} else if(c == 0x03){
        		LOGI("get reboot cmd");
        		inUpdateFile = 0;
        	}
        }
    }
    LOGI("end........");
    close(comm_fd);
    return 0;
}

int
sdtp_endUpdateFile(void){
	inUpdateFile = 0;
}
