#ifndef SDTP_H_
#define SDTP_H_

#include <stdint.h>

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
    int (*_put)(int ch);        /* Send Data: one byte */
    int (*_get)(int timeout);   /* Receive Data: one byte */
}sdtp_t;

int sdtp_send(sdtp_t * arq, uint8_t * data, int datalen);
int sdtp_recv(sdtp_t * arq, uint8_t * buff, int bufflen, int timeout);
int sdtp_updateFile(char *posInfo, int posInfoLen, char *pdir,char *appInfo , int appLen, char *ptime);
#endif

