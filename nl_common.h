#ifndef _NL_COMMON_H
#define _NL_COMMON_H

#include "mr_common.h"
#include <sys/shm.h>

typedef struct _nl_tshare_t {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	int qr_run;
	int gi_run;
} nl_tshare_t;

extern nl_tshare_t  share;
extern MADR SRC_ADDR;

extern int speed_level;
extern U16 mtu_grade[MAX_SPEED_LEVEL];
extern U16 mtu[MAX_NODE_CNT];

//LowMAC在两板间发前边加4字节，后边CRC4字节，MAC帧头8字节。
//总限制512，so 512-4-4-8 = 496
#define MAX_PACKAGE_DATA_LENGTH		496

#define MIN_MTU_GRADE 128

int shm_init();
int maodv_shm_init();

int init_ip_hash();
int init_nl_hash();

int find_filter(U8 pt, U16 port);
U8 find_Cos(U8 pt, U16 port);

int init_mtu();

fwt_t * shm_fwt;
m_table_shm * p_mt_shm;


#endif

