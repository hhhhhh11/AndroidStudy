// #include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

// #include <utils/Log.h>
#include <android/log.h>
#include <time.h>
#include "ndk.h"

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

unsigned char dataBuffer[MAX_FRAME_SIZE*2];
int needRead = 1;
int position = 0;
int totalLength = 0;

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

int getcTimeout(int comm_fd, int timeout_ms)
{
    uint8_t ch;

    do
    {
    	if (read(comm_fd, &ch, 1 ) == 1 ) return ch;
    	usleep(1000);
    }while(--timeout_ms>0);
    return -1;
}

int writeK21(int fd,  unsigned char* buf , int len)
{
	return write(fd,buf,len);
}

/**
 * 打包一帧
 * @param pro
 * @param type  帧类型
 * @param data  数据
 * @param datalen 数据长度
 * @return
 */
static int packet_a_frame(sdtp_t * pro, frame_type_t type, uint8_t * data, int datalen)
{
    uint8_t * pframe;
    uint16_t crc;
    if (pro->syn_req) {
        type |= FSYN;
    }
    if (type & FDATA) {//如果是数据帧
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

	LOGI("_reccomm-1 --------receive with len:%d,timeout:%d\n",len,timeout);
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
        LOGI("_reccomm-1 read--------receive err 1 nfds=%d\n",nfds);
	   	if (nfds < 0) {
			if (EINTR == errno) {

				continue;
	        }
	        return -1;
	    }else if (nfds > 0) {
	    	if(readcnt == 0){
	    		gettimeofday(&tv2,&tz);
	    	}

	    	ret = read( fd, outbuf + readcnt, len-readcnt);
            LOGI("_reccomm-1 read--------receive err 1 ret=%d\n",ret);
	        if (FD_ISSET(fd, &readfds)) {
	        	if (ret < 0) {
	        		return -1;
	        	} else if (0 == ret) {
					return -1;
	        	} else if (ret < (len-readcnt)) {
	        		if(readcnt == 0){
	        			gettimeofday(&tv3,&tz);
	        		}
	        		iii++;
					readcnt += ret;
					flag = 1;
                    if (ret < 4095){
                        return readcnt;
                    }
	                continue;
	            }else{
	            	return len;
	            }
	        }
	    } else {
	        if (0 == readcnt) {
	        	return -1;
	        }else{
	        	return readcnt;
	        }
	    }
    }
}
#if 0
int _reccomm(int fd, int len, char *outbuf, int timeout)
{
    struct timeval tv1, tv2, tv3;
    int cur_time;
    fd_set readfds;
    int nfds = -1;
    int flag = 0;
    int readcnt=0;
    int curtime = 0;
    int ret = 0;
    int ii = 0;

	gettimeofday(&tv1, NULL);

	do{
		if (read( fd, outbuf, 1)==1){
			gettimeofday(&tv2, NULL);
			readcnt ++;
			break;
		}
		usleep(1000);
//		gettimeofday(&tv2, NULL);
//		curtime = (tv2.tv_sec-tv1.tv_sec)*1000 + (tv2.tv_usec-tv1.tv_usec);
	}while(--timeout>=0);

	if (readcnt<=0){
		return -1;
	}

	while(1){
		ret=read(fd, outbuf+readcnt, len-readcnt);
		if (ret <= 0){
			if (!flag){
				usleep(1000);
				flag ++;
			}else{
				gettimeofday(&tv3, NULL);
				LOGI("6------------%ld, %ld,%ld, %ld, %ld, %ld, %d, %d", tv1.tv_sec, tv1.tv_usec, tv2.tv_sec, tv2.tv_usec, tv3.tv_sec, tv3.tv_usec, flag, ii);
				return readcnt;
			}
		}else{
			ii++;
			readcnt += ret;
		}
	}

	return readcnt;
}
#endif

static int recv_a_frame(sdtp_t * pro, int timeout)
{
    unsigned char * pdata = (unsigned char *)pro->in;
    unsigned char recvdata;
    int flag = 0, ret, len = 0, escape = 0;

    struct  timeval    tv;
    struct  timezone   tz;
    gettimeofday(&tv,&tz);
    int jj = 0;

//    unsigned char *buffer = NULL;
    int length = 0;
    int count = 0;

//    buffer = malloc(MAX_FRAME_SIZE*2);
    if(needRead == 0){ // needRead == 0, 说明不需要再读取数据包，已在缓存dataBuffer中
    	LOGI("already has the data !!!!!!!!!!!!! position = %d, totalLength = %d", position, totalLength);
    	count = position;
    	length = totalLength;

    	while(*(dataBuffer+(totalLength-1)) != FSTX){
    		length = _reccomm(pro->fdCom,MAX_FRAME_SIZE*2-totalLength,dataBuffer+totalLength,timeout);
    		totalLength = totalLength + length;
    		length = totalLength;
    		LOGI("*(dataBuffer+totalLength) != FSTX, totalLength = %d", totalLength);
    	}
    }else{
        LOGI("_reccomm STAT--------------------");
        length =  _reccomm(pro->fdCom,MAX_FRAME_SIZE*2,dataBuffer,timeout);
        LOGI("_reccomm  END----------------%d", length);
        if(length < 0){
    	   return -1;
       }

        while(*(dataBuffer+(length-1)) != FSTX){
            length += _reccomm(pro->fdCom,MAX_FRAME_SIZE*2-length,dataBuffer+length,timeout);
            LOGI("*(dataBuffer+totalLength2222222222222) != FSTX, length = %d", length);
        }
    }


    /**
     * flag = 0 we haven't met FSTX, =1 frame end
     * escape = 0 normal character, = next one needs escaping
     */
    LOGI("----- recv_a_frame START, %d, %ld, %ld", timeout, tv.tv_sec, tv.tv_usec);
    ret = *(dataBuffer+count);
    do {
//        if ((ret = pro->_get(pro->fdCom, timeout)) < 0) {
//  			 LOGI("recv_a_frame  : timeout");
//        	break;
//        }
//        if(jj == 0) {
//        	gettimeofday(&tv,&tz);
//        	jj++;
//        }
        recvdata = *(dataBuffer+count);
        count++;
        if (recvdata == FSTX) {  /* Frame Start Character */
            if (flag == 1) { /* frame end  第二个表示帧结束了 */
                ret = len;
                /* if frame too short, it is invalid */
                LOGI("recv_a_frame end, ret = %d, count = %d", ret, count);
                if(count == length){
                	needRead = 1;
                	position = 0;
                	totalLength = 0;
                }else{
                	needRead = 0;
                	position = count;
                	totalLength = length;
                }
                flag = 2;
                break;
            }
            if (flag == 2) { /* last frame data error */
   			 LOGI("recv_a_frame  : last frame data error");
                flag = 0; /* we want first FSTX again */
                continue;
            }
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
    }while (count < length);

    LOGI("---FLAG = %d ---------", flag);
    if(flag != 2){
    	needRead = 1;
    	position = 0;
    	totalLength = 0;
    }
    LOGI("----- recv_a_frame END-----------%ld, %ld", tv.tv_sec, tv.tv_usec);
    return ret;
}

/**
 * 发送一帧，对SYN进行转义处理，对串口写入
 * @param pro
 * @param type
 * @param hold
 * @return
 */
static int send_a_frame(sdtp_t * pro, frame_type_t type, int hold)
{
    uint8_t * psend = (type & FDATA) ? (unsigned char *)(pro->out) :
            (unsigned char *)(pro->acknak);

#if 0		//一个字节一个字节发送
    unsigned char fstx;
    unsigned char fdle;
    unsigned char e;
    unsigned char d;
    fstx = FSTX;
    fdle = FDLE;
    e = 0x5E;
    d = 0x5D;
    if (pro->_put(pro->fdCom, &fstx, 1) < 0) return -1;
    while (hold) {
        if (*psend == FSTX) {
            if (pro->_put(pro->fdCom, &fdle, 1) < 0) return -1;
            if (pro->_put(pro->fdCom, &e, 1) < 0) return -1;
        }
        else if (*psend == FDLE) {
            if (pro->_put(pro->fdCom, &fdle, 1) < 0) return -1;
            if (pro->_put(pro->fdCom, &d, 1) < 0) return -1;
        }
        else {
            if (pro->_put(pro->fdCom, psend, 1) < 0) return -1;
        }
        psend ++;
        hold --;
    }
    if (pro->_put(pro->fdCom, &fstx, 1) < 0) return -1;
#else
	char _buf[MAX_FRAME_SIZE*2];
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

    while (datalen >= 0) {//数据全发完
        retry = arq->retry;
        sendlen = datalen > arq->frame_size ? arq->frame_size : datalen;
        len = packet_a_frame(arq, FDATA, pdata, sendlen);
        do {//重试
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

/**
 *
 * @param arq
 * @param buff
 * @param bufflen
 * @param timeout
 * @return
 */
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

/**********************************************************************
* 函数名称： k21_IOCtrl
***********************************************************************/
#define BOOTUP	((int)0)
#define UPDATE	((int)1)



/**********************************************************************
* 函数名称： set_com_config
* 功能描述： 设置串口参数等
* 接口描述
*   输入参数：  int fd - 串口对应文件描述符
*		  					int baud_rate - 波特率
*		  					int data_bits - 数据长
*		  					char parity    - 校验位 N或n或S或s=无校验 E或e=偶校验 O或o=奇校验
*		  					int stop_bits - 停止位
*		  					char ir_en	  - 红外通讯防止反射串扰,即自发自收防护功能 I或i或Y或y=开启 , N或n=关闭
*		  					char block_en - 是否开启读写阻塞  B或b或Y或y=阻塞 N或n=非阻塞

*   返回参数： int  - 			0 表示成功, 非0失败,-1=fd错 -2=设置串口参数出错 -3=设置阻塞失败

*   影响的参数：

* 补充说明 :
***********************************************************************/
int set_com_config(int fd,int baud_rate, int data_bits, char parity, int stop_bits,char ir_en,char block_en)
{
	struct termios new_cfg,old_cfg;
	int speed,flag;

	/*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
	if  (tcgetattr(fd, &old_cfg)  !=  0)
	{
		LOGI("tcgetattr error");
		return -1;
	}

	/*设置字符大小*/
	new_cfg = old_cfg;
	cfmakeraw(&new_cfg);
	new_cfg.c_cflag &= ~CSIZE;

  	/*设置波特率*/
  	switch (baud_rate)
  	{
  		case 2400:
		{
			speed = B2400;
		}
		break;

  		case 4800:
		{
			speed = B4800;
		}
		break;

  		case 9600:
		{
			speed = B9600;
		}
		break;

		case 19200:
		{
			speed = B19200;
		}
		break;

  		case 38400:
		{
			speed = B38400;
		}
		break;

  		case 57600:
		{
			speed = B57600;
		}
		break;

  		case 230400:
		{
			speed = B230400;
		}
		break;

  		case 460800:
		{
			speed = B460800;
		}
		break;

   		case 921600:
		{
			speed = B921600;
		}
		break;


	 	case 1500000:
		{
			speed = B1500000;
		}
		break;

	 	case 3000000:
		{
			speed = B3000000;
		}
		break;

	 	case 4000000:
		{
			speed = B4000000;
		}
		break;



		default:
		LOGI("Baud not support,force to 115200!!");

		case 115200:
		{
			speed = B115200;
		}
		break;
  	}
	flag = cfsetispeed(&new_cfg, speed);
	LOGI("setiBaud %d",flag);

	flag = cfsetospeed(&new_cfg, speed);
	LOGI("setoBaud %d , %d ",flag,baud_rate);

	/*设置数据位*/
	switch (data_bits)
	{
		case 7:
		{
			new_cfg.c_cflag |= CS7;
		}
		break;

		default:
		LOGI("data_bits not support,force to 8!!");

		case 8:
		{
			new_cfg.c_cflag |= CS8;
		}
		break;
  	}

  	/*设置奇偶校验位*/
  	switch (parity)
  	{
		default:
		LOGI("parity not support,force to null!!");
		case 'n':
		case 'N':
		{
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_iflag &= ~INPCK;
		}
		break;

        case 'o':
		case 'O':
        {
            new_cfg.c_cflag |= (PARODD | PARENB);
            new_cfg.c_iflag |= INPCK;
        }
        break;

		case 'e':
        case 'E':
		{
			new_cfg.c_cflag |= PARENB;
			new_cfg.c_cflag &= ~PARODD;
			new_cfg.c_iflag |= INPCK;
        }
		break;

        case 's':  /*as no parity*/
		case 'S':
        {
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_cflag &= ~CSTOPB;
		}
		break;
	}

	/*设置停止位*/
	switch (stop_bits)
	{
		default:
		LOGI("stop_bits not support,force to 1!!");

		case 1:
		{
			new_cfg.c_cflag &=  ~CSTOPB;
		}
		break;

		case 2:
		{
			new_cfg.c_cflag |= CSTOPB;
		}
	}

	/*设置等待时间和最小接收字符*/
	new_cfg.c_cc[VTIME]  = 0;
	new_cfg.c_cc[VMIN] = 1;

	/* 设置红外自发自收防护功能 @@@2011/09/16 */
  	switch (ir_en)
  	{
  		// 红外自发自收防护功能开启
  		case 'Y':
   		case 'y':
 		case 'I':
 		case 'i':
  			new_cfg.c_iflag |= IMAXBEL;
  			break;

  	  	// 红外自发自收防护功能关闭
 		default:
  		case 'N':
   		case 'n':
  			new_cfg.c_iflag &= ~IMAXBEL;
  			break;

  	}

	/*处理未接收字符*/
	tcflush(fd, TCIFLUSH);

	/*激活新配置*/
	if((tcsetattr(fd, TCSANOW, &new_cfg)) != 0)
	{
		perror("tcsetattr");
		return -2;
	}

	/*处理阻塞状态设置*/
	flag = fcntl(fd,F_GETFL,0);
	if((block_en == 'Y')||(block_en == 'y')||(block_en == 'B')||(block_en == 'b'))
	{
		 // 设置串口为阻塞状态
		flag &= ~O_NONBLOCK;

	}
	else
	{ 	// 设置串口为非阻塞状态
	      flag |= O_NONBLOCK;
	}
	if (fcntl(fd, F_SETFL, flag) < 0)	// (fcntl(fd, F_SETFL, 0) < 0)
	{
		LOGI("fcntl F_SETFL\n");
		return -3;
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
int inUpdateFile = 0;
int
sdtp_updateFile(char *posInfo, int posInfoLen, char *pdir ,char *appInfo , int appLen, char *ptime)
{
	LOGI("sdtp_updateFile %s", posInfo);
	inUpdateFile = 1;
	int comm_fd;	// PC端串口
    int file_fd;	// 保存文件句柄
    int shakeHandSuccess = 0; //握手成功与否判断；0：未握手 1：已握手

    int len, rlen;
    int ret = -1;

    unsigned char cmd, buff[MAX_FRAME_SIZE];
    char  *pbuf = buff;
    char shakeHand[1] = {6};

    comm_fd = open("/dev/ttyGS0", O_NONBLOCK | O_RDWR);
//    set_com_config(comm_fd, 115200, 8, 'N', 1, 'N', 'N');

    sdtp_t sdtp;
    char inbuf[FRAME_LEN],outbuf[FRAME_LEN],ackbuf[FRAME_LEN];
    /* SDTP Parameters Setup */
    sdtp.frame_size = MAX_FRAME_SIZE; // master->frame_size;
    sdtp.no_fcs = 0; //需要crc   master->no_fcs;
    sdtp.retry = 3; // 3;
    sdtp.start = 0;
    sdtp.syn_req = 0;
    sdtp.timeout_msec = 500;   //150;
    sdtp._get = getcTimeout; /* TODO */
    sdtp._put = writeK21;   /* TODO */
    sdtp.in = (frame_t *)inbuf;
    sdtp.out = (frame_t *)outbuf;
    sdtp.acknak = (frame_t *)ackbuf;
    sdtp.fdCom = comm_fd;     // PC端对应端口

    LOGI("comm_fd %d ",comm_fd);
    if (comm_fd < 0){
        return -33;
    }
    /* TODO:Get 0x02 */
    char c;
    cmd = 0; /* 发送0x00 开始传输 */
    int downType = 0;

    while(inUpdateFile) { /* 0x02 + len(4 bytes) + name */

    	// 握手
    	if(shakeHandSuccess == 0){
    		LOGI("shake hand");
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
            c = pbuf[0];
            LOGI("===========================================================================C-pbuf: %x" ,c);
        	if(c == 0x00){ // 获取版本
        		LOGI("获取版本");
        		shakeHandSuccess = 1;
        		sdtp_send(&sdtp, posInfo, posInfoLen);
        	}else if(c == 0x02 || c == 0x07 || c == 0x05){ // 下载文件(02,05--下载checkKey xml且不是第一台,07--下载checkKey mxl且是第一台)
        		LOGI("下载文件");
        		shakeHandSuccess = 1;
            	LOGI("22222222222");
                memcpy(&rlen, pbuf + 1, 4); /* 总长度  */
                if(c == 0x02){
                    downType = 0;
                }else if(c == 0x05){
                    downType = 5;
                }else if(c == 0x07){
                    downType = 7;
                }
                // 获取下载的文件的 文件名
                char nameBuf[ret - 5];
                char *pnameBuf;
                strcpy(nameBuf, pbuf+5);
                pnameBuf = nameBuf;
                LOGI("pnameBuf %s", pnameBuf);
                LOGI("pdir %s", pdir);
                char *ppath = malloc(strlen(pdir)+1+strlen(pnameBuf)+1);
                if(ppath == NULL){
                	close(comm_fd);
                	return -1;
                }
                strcpy(ppath, pdir);
                strcat(ppath, pnameBuf);
                LOGI("ppath %s", ppath);

                file_fd = open(ppath, O_RDWR|O_CREAT ,777);/* 创建文件 */
                LOGI("file_fd %d ",file_fd);
                if(file_fd < 0){
                	close(comm_fd);
                	free(ppath);
                	return -1;
                }
                sdtp_send(&sdtp, &cmd, 1); /* 发送0x00 开始传输 */
                LOGI("3333333");
                LOGI("rlen %d ",rlen);
                while (rlen) {
                    len = rlen > MAX_FRAME_SIZE ?
                    		MAX_FRAME_SIZE : rlen;
                    LOGI("len %d ",len);
                    if (sdtp_recv(&sdtp, pbuf, len, 10000) != len) {
                    	LOGI("len %d ",len);
                    	LOGI("pbuf: %s" ,pbuf);
                    	free(ppath);
                    	close(file_fd);
                    	close(comm_fd);
                        return -1;
                    }
                    LOGI("44444444");
                    if (write(file_fd, pbuf, len) != len) {
                        *pbuf = 0x04;
                        inUpdateFile = 0;
                        free(ppath);
                        close(file_fd);
                        close(comm_fd);
                        return -2;
                    }
                    LOGI("55555555");
                    *pbuf = 0x00;
                    sdtp_send(&sdtp, pbuf, 1);
                    rlen -= len;
                    LOGI("rlen %d ",rlen);
                }
                free(ppath);
                close(file_fd);
                LOGI("rlen = 0 , end while");
                *pbuf = 0x00;
                sdtp_send(&sdtp, pbuf, 1);
//                break;
        	} else if(c == 0x03){
        		LOGI("get reboot cmd");
                memcpy(ptime,pbuf+1,4);
                inUpdateFile = 0;
        		close(comm_fd);
                LOGI("downTye: %d",downType);
        		if(downType == 0){
        		    return 0;
        		}else if(downType == 5){
                     // int retval=NDK_SysSetPosTime(*tm_t);
                    // LOGI("===========================================================================retval : %d", retval);
        		    return 5;
                }else if(downType == 7){
                    // int retval=NDK_SysSetPosTime(*tm_t);
                    // LOGI("===========================================================================retval : %d", retval);
                    return 7;
                }
        	} else if(c == 0x01){// OTA包不匹配
        		inUpdateFile = 0;
        		close(comm_fd);
        		return -3;
        	} else if(c == 0x04){ // 清除应用
        		LOGI("need clear apks .......");
        		inUpdateFile = 0;
        		close(comm_fd);
        		return 4;
        	} else if(c == 0x11){
        	    LOGI("获取应用");
                sdtp_send(&sdtp, appInfo, appLen);
                inUpdateFile = 0;
                close(comm_fd);
                if (appLen == 1)
                {
                    return -5;
                }
                return 1;
            }else if(c == 0x08){ //安装应用无法取消
                inUpdateFile = 0;
                close(comm_fd);
                return 8;
            }else if(c == (0x04 | 0x08)){
                 inUpdateFile = 0;
                close(comm_fd);
                return 12;
            }
        }
    }
    if(shakeHandSuccess == 0){
    	close(comm_fd);
    	return -4;
    }

    LOGI("end........");
    close(comm_fd);
    return 0;
}

int
sdtp_endUpdateFile(void){
	inUpdateFile = 0;
}


void sdtp_endUninstallApp(){
    int isUninstalling = 0;
}
void sdtp_getAppInfo(){
    int inGetAppInfo = 0;
}


int inGetAppInfo = 0;
int isUninstalling = 0;
