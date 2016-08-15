#include "nl_send.h"

static int lock = 0;								  //c��û��bool���ͣ����ﶨ���int��, nl_send_to_others������lock
static int lock_of_himac = 0;						  //nl_send_to_himac������lock

nl_buff_pool_t  *nl_buf_pool;						  //��̬�������himac�Ľ��ջ��棬������Ϊnl_buff_num��ѭ��ʹ�ã���������pkt��
int nl_buff_num = 11;
static int nl_buff_timeout = 5;

U16 mtu[MAX_NODE_CNT];

extern int speed_level;

int combine_send_pkt(nl_package_t * pkt, int length)
{
	int H, SN;
	H = get_H(pkt);
	SN = get_SN(pkt);
	/*************************************************��������ֵİ���ֱ��ת��******************************************************/
	if(H == 1 && SN == 0)
	{
//2.26		printf("sigle pkt\n");
		mmsg_t * snd_buf;
		snd_buf = (mmsg_t *)malloc(sizeof(mmsg_t));
		if (snd_buf == NULL)
			EPT(stderr, "!!! malloc error when deal sigle pkt\n");
		memset(snd_buf, 0, sizeof(snd_buf));

		set_nl2other_mtype(snd_buf, pkt);
		
//2.26		EPT(stderr, "***snd_buf->mtype = %ld\n", snd_buf->mtype);

		snd_buf->node = get_src_addr(pkt);
		
		memcpy(snd_buf->data, pkt->data, length - 8);

//3.24 added by wanghao
		U8 Cos = get_CoS(pkt);
		
		nl_send_to_others(snd_buf, length - 8);
		free(snd_buf);
		snd_buf = NULL;
		return 0;
	}
	/*************************************************ԤԼ���߻�ȡpkt������******************************************************/
	nl_buff_t * pkt_buf;
	int pool_id ;
	//4.18 through seq and src determine key!
	//changed by wanghao on 5.10
	U8 seq, src;
	src = get_src_addr(pkt);
	seq = get_SEQ(pkt);
	
	int key = seq ^ src;
	while(pool_id = manage_nl_buf(key, src, seq) == -1)
	{
		EPT(stderr, "order a nl_buf error!! maybe full used!!\n");
		sleep(1);
	}
	pkt_buf =&(nl_buf_pool[pool_id].nl_buf);
	/******************************************************����pkt��************************************************************/
	int SH, SSN;
	SH = get_SH(pkt);
	SSN = get_SSN(pkt);
	
	printf("ready recombine data:%s\n",pkt->data);
	
	if (H == 1)											//��һ���ֶεİ�
	{
		if( SH == 1 && SSN == 0 )						//δ�ٷֶ�
		{
			pkt_buf->number = SN + 1;
			pkt_buf->count = 1;
			pkt_buf->len[0] = length;
			memcpy(pkt_buf->package[0], pkt, length);					//����Խ��û�����⣬��Ϊ����Ŀռ仹���ڽ���buff��ֵΪ0
																		//4.18 no use! waste	
		}
		else if(SH == 1)								//�ٷֶ� -> ��һ���ٷֶεİ�
		{
			pkt_buf->SSN_NUM = SSN + 1;
			pkt_buf->number = SN + 1 + SSN;
			pkt_buf->count = 1;
			pkt_buf->len[0] = length;
			memcpy(pkt_buf->package[0], pkt, length);
			printf("pkt_buf->package[0]:%s\n",pkt_buf->package[0]);
		}
		else											//�ٷֶ� -> ֮����ٷֶεİ�
		{
			pkt_buf->count++;
			pkt_buf->len[SSN] = length;
			memcpy(pkt_buf->package[SSN], pkt, length);	
			printf("pkt_buf->package[%d]:%s\n",SSN,pkt_buf->package[SSN]);
		}
	}
	else												//���ֶεİ�
	{
		if( SH == 1 && SSN == 0 )						//δ�ٷֶ�
		{
			pkt_buf->count++;
			pkt_buf->len[SN*(pkt_buf->SSN_NUM)] = length;
			memcpy(pkt_buf->package[SN*(pkt_buf->SSN_NUM)], pkt, length);
			printf("pkt_buf->package[%d]:%s\n",SN*(pkt_buf->SSN_NUM),pkt_buf->package[SN*(pkt_buf->SSN_NUM)]);
		}
		else if(SH == 1)								//�ٷֶ� -> ��һ���ٷֶεİ�
		{
			pkt_buf->number += SSN;
			pkt_buf->count ++;
			pkt_buf->len[SN*(pkt_buf->SSN_NUM)] = length;
			memcpy(pkt_buf->package[SN*(pkt_buf->SSN_NUM)], pkt, length);
			printf("pkt_buf->package[%d]:%s\n",SN*(pkt_buf->SSN_NUM),pkt_buf->package[SN*(pkt_buf->SSN_NUM)]);
		}
		else											//�ٷֶ� -> ֮����ٷֶεİ�
		{
			pkt_buf->count++;
			pkt_buf->len[SN*(pkt_buf->SSN_NUM) + SSN] = length;
			memcpy(pkt_buf->package[SN*(pkt_buf->SSN_NUM) + SSN], pkt, length);	
			printf("pkt_buf->package[%d]:%s\n",SN*(pkt_buf->SSN_NUM)+ SSN,pkt_buf->package[SN*(pkt_buf->SSN_NUM)+ SSN]);
		}
	}
	
	printf("check:%s\n",pkt_buf->package[0]);
	
	//if (pkt_buf->number == SN && pkt_buf->count == SN)
	//if(pkt_buf->number == SN)
	if (pkt_buf->number == pkt_buf->count)
	{
		int i,len = 0;
		U16 data_len;
		nl_package_t *tmp_ptr;
		mmsg_t * snd_buf;
		snd_buf = (mmsg_t *)malloc(sizeof(mmsg_t));
		if (snd_buf == NULL)
			return;
		
		//added by wanghao on 4.18
		set_nl2other_mtype(snd_buf,pkt);
		
		snd_buf->node = get_src_addr(pkt);
		
		char * ptr = snd_buf->data;
		for (i = 0; i < pkt_buf->number; i ++)
		{
			tmp_ptr = (nl_package_t*)(pkt_buf->package[i]);
			data_len = pkt_buf->len[i] - 8;
			
			printf("package[%d] 's len:%d\n",i, data_len);
			int j;
			printf("\n\n");
			for(j = 0; j<data_len;j++)
				printf("%c",tmp_ptr->data[j]);
			printf("\n\n");
			
			memcpy(ptr, tmp_ptr->data, data_len);

			ptr += data_len;
			len += data_len;
		}

		nl_send_to_others(snd_buf,len);
		free(snd_buf);
		snd_buf = NULL;
	}
	return 0;
}

//����snd_msg��ͬ���������ͷ��͵���Ӧ�Ľ���
int nl_send_to_others(mmsg_t *snd_msg, U16 length)
{
	while(lock)					//��֤ͳһʱ��ֻ�иú���ֻ��һ���������
	{
		sleep(1);
	}
	lock = 1;

	int qid;

	switch(snd_msg->mtype)
	{
		case MMSG_REF_DATA:
		case MMSG_RIP_DATA:
		case MMSG_RIPC_DATA:
			qid = vi_qid;
			break;
		case MMSG_URP_DATA:
			qid = rp_qid;
			break;
    /*
	????????
		case MMSG_FT_DATA:
            qid = rp_qid;
			snd_msg->mtype = MMSG_FT_DATA;
			break;
	???????
	*/
		case MMSG_MRP_DATA:
			qid = ma_qid;
			break;
		case MMSG_SMS_TEST:
			printf("rcv sms from %d:\n",snd_msg->node);
			printf("%s\n",snd_msg->data);
			return 0;
		default:
			printf("default qid = -1\n"); 
			qid = -1;
			break;
	}
	
//2.26	EPT(stdout, "~~~mtype:%ld qid:%d\n", snd_msg->mtype, qid);
	
	while(msgsnd(qid, snd_msg, length + sizeof(MADR), 0) < 0)
	{
		if (errno == EINTR)
			continue;
		else
			{
				EPT(stdout, "%s:------snd to himac wrong------------\n", qinfs[re_qin].pname);
				return -1;
			}
	}

//2.26	printf("nl,send\n");

	lock = 0;
	return 0;
}

//����Ϣ���ݾ�����װ�ֽⷢ�͵�himac��
int nl_send_to_himac(mmsg_t *msg, int len)
{
//	EPT(stderr, "#hm_qid:%d\n",hm_qid);
	while(lock_of_himac)									//��֤ͬһʱ��ֻ�иú���ֻ��һ���������
	{
		EPT(stderr, "##while sleep~~~\n");
		usleep(20);
	}
	lock_of_himac = 1;

	mmsg_t * snd_buf;
	snd_buf = (mmsg_t *)malloc(sizeof(mmsg_t));
	
	if (snd_buf == NULL)
		EPT(stderr, "!!! malloc error when deal sigle pkt\n");
	memset(snd_buf, 0, sizeof(snd_buf));

	//һ�����������ͣ�MMSG_IP_DATA��MMSG_RP_FT_DATA��MMSG_RPM,MMSG_MAODV,  MMSG_MP_DATA
	
	switch(msg->mtype)
	{
		case MMSG_IP_DATA:
		case MMSG_EF_DATA:
		case MMSG_IPC_DATA:
		case MMSG_RPM:
		case MMSG_MRPM:
			snd_buf->mtype = MMSG_MP_DATA;
			break;
		case MMSG_RP_FT_DATA:
			snd_buf->mtype = MMSG_FT_DATA;
			break;
		case MMSG_FT_REQ:
			snd_buf->mtype = MMSG_FT_REQ;
			break;
		case MMSG_MTU_DATA:
			snd_buf->mtype = MMSG_MTU_DATA;
		default:
			printf("!!!nl_msg->mtype_default:%ld\n",msg->mtype); 
			break;
	}

	EPT(stderr,"# NL_snd_buf->mtype:%ld\n",snd_buf->mtype);
	
	char *ptr = NULL;
	int left,n;
	int count = 1;

	int flag = 0;
	int size_for_snd;

	nl_package_t * pkt;
	pkt = (nl_package_t *)(snd_buf->data);					//��pkt����snd_buf��data����
	//wanghao4 on 2.29
	init_package_head(pkt,msg);			//��ʼ��ͷ��		ԭ����init_package_head(pkt,snd_buf);	god damn it!!!!

	//4.18 node set CoS while sending to HM
	//snd_buf->node = msg->node;
	snd_buf->node = get_CoS(pkt);
	
	int rval;
	for(rval = 0; rval < MAX_NODE_CNT; rval ++)
		printf("2mtu[%d]=%d\n",rval,mtu[rval]);
	
	U16 max_data_length = mtu[get_rcv_addr(pkt) - 1] - 16;	//ȥ��8�ֽ�MACͷ��4�ֽ�CRC�����л�����Ҫ��4�ֽ�ͷ��
	EPT(stderr,"! FOR TEST: sent to %d 's mtu :%d\n", get_rcv_addr(pkt), max_data_length);
	
	//int length = ((mmhd_t *)msg->data)->len;
	//��Ϣ����data�ĳ���
	left = len;
	ptr = (char *)(msg->data);
	while(left > 0)
	{
//		EPT(stderr, "##hm_qid:%d\n",hm_qid);
		if (left > max_data_length)		//MAX_PACKAGE_DATA_LENGTH		496
		{

			n = max_data_length;
		}
		else
		{

			n = left;
		}
		

//4.18		set_data_length(pkt, n);

		if (left == len)				//����ĵ�һ��pkt
		{

//2.26		printf("this is the pkt : 1\n");
			
			set_H(pkt, 1);
			if(len > max_data_length)
			{
				set_SN(pkt, len/max_data_length);//�����յĳ���С��pkt����,��Ĭ��SN = 0
			}
		}
		else
		{
//2.26		    printf("this is the pkt : %d\n",count);
			set_H(pkt, 0);
			set_SN(pkt,count++);
		}
		
		memcpy(pkt->data, ptr, n);

		ptr += n;
		left -= n;
        //printf("pkt data : %s\n",pkt->data + sizeof(mmhd_t));
		
		int size_for_snd = sizeof(MADR) + 8 + n;
		EPT(stderr, "send to himac %d bytes\n", size_for_snd);
		
		while(msgsnd(hm_qid, snd_buf, size_for_snd, 0) < 0)
		{
			if (errno == EINTR)
				continue;
			else
				{
					EPT(stdout, "%s:------snd to himac wrong------------\n", qinfs[re_qin].pname);
					break;
				}
		}
		
	//	EPT(stderr, "###hm_qid:%d\n",hm_qid);

		/*nl_package_t * pkt_1 = (nl_package_t *)snd_buf->data;
		combine_send_pkt(pkt_1);

		printf("here2\n");*/

	}

	free(snd_buf);
	snd_buf = NULL;
	lock_of_himac = 0;
	return 0;
}



//��HighMAC�ϴ�����Ҫ�ٷֶε����ݽ����ٷֶΣ��ٴ���HighMAC
void nl_reseg_to_himac(mmsg_t *msg, int len)
{
	
	while(lock_of_himac)									//��֤ͬһʱ��ֻ�иú���ֻ��һ���������
	{
		EPT(stderr, "##while sleep~~~\n");
		usleep(20);
	}
	lock_of_himac = 1;
	
	mmsg_t * snd_buf;
	snd_buf = (mmsg_t *)malloc(sizeof(mmsg_t));
	
	if (snd_buf == NULL)
		EPT(stderr, "!!! malloc error when deal sigle pkt\n");
	memset(snd_buf, 0, sizeof(snd_buf));
	
	snd_buf->mtype = MMSG_MP_DATA;
		
	char *ptr = NULL;
	int left,n;
	int count = 1;

	int flag = 0;
	int size_for_snd;

	nl_package_t * pkt;
	pkt = (nl_package_t *)(snd_buf->data);					//��pkt����snd_buf��data����
	
	init_reseg_package_head(pkt,msg);			//��ʼ��ͷ��	

	//4.18 node set CoS while sending to HM
	//snd_buf->node = msg->node;
	snd_buf->node = get_CoS(pkt);
	
	U16 data_length_limit = mtu[get_rcv_addr(pkt) - 1] - 16;	//ȥ��8�ֽ�MACͷ��4�ֽ�CRC�����л�����Ҫ��4�ֽ�ͷ��
	EPT(stderr,"! FOR TEST: sent to %d 's mtu :%d\n", get_rcv_addr(pkt), data_length_limit);
	
	if(data_length_limit <= len)		//�������жϸ����ó����ģ�or������Ҫ�ٷֶεġ��ٷֶΣ�ֱ�Ӱ���СMTU����
	{
		//int length = ((mmhd_t *)msg->data)->len;
		//��Ϣ����data�ĳ���
		left = len - 8 ;				//left: data���ȣ�����MAC֡ͷ������
		ptr = (char *)(&(msg->data[8]));
		
		U8 H = get_H(pkt);
		U8 SN = get_SN(pkt);
		
		int flag = 0;
		if( (1 == H) && (0 == SN))
		{
			flag = 1;					//��ʾ�����ݰ�δ�ֶΣ������ٷֶ�ʱ����H��SN����������SH��SSN
		}
		
		while(left > 0)
		{
	//		EPT(stderr, "##hm_qid:%d\n",hm_qid);
			if (left > MIN_MTU_DATA_LEN)	
			{
				n = MIN_MTU_DATA_LEN;
			}
			else
			{
				n = left;
			}

			if (left == (len-8) )				//����ĵ�һ��pkt
			{

	//2.26		printf("this is the pkt : 1\n");
				if(flag)
					set_H(pkt, 1);
				else
					set_SH(pkt, 1);
				if(left > MIN_MTU_DATA_LEN)
				{
					if(flag)
						set_SN(pkt, left/MIN_MTU_DATA_LEN);
					else	
						set_SSN(pkt,left/MIN_MTU_DATA_LEN);//�����յĳ���С��pkt����,��Ĭ��SN = 0
				}
			}
			else
			{
	//2.26		    printf("this is the pkt : %d\n",count);
				if(flag)
				{
					set_H(pkt, 0);
					set_SN(pkt,count++);					
				}
				else
				{
					set_SH(pkt, 0);
					set_SSN(pkt,count++);					
				}
			}
			
			memcpy(pkt->data, ptr, n);

			ptr += n;
			left -= n;
			//printf("pkt data : %s\n",pkt->data + sizeof(mmhd_t));
			
			int size_for_snd = sizeof(MADR) + 8 + n;
			EPT(stderr, "   send reseg package to himac %d bytes\n", size_for_snd);
			
			while(msgsnd(hm_qid, snd_buf, size_for_snd, 0) < 0)
			{
				if (errno == EINTR)
					continue;
				else
					{
						EPT(stdout, "%s:------snd to himac wrong------------\n", qinfs[re_qin].pname);
						break;
					}
			}
		}
	}
	else
	{
		EPT(stderr,"!!! ERROR, data_length_limit(%d) > data_len(%d)\n", data_length_limit, len);
	}

	free(snd_buf);
	lock_of_himac = 0;
	snd_buf = NULL;
}

int manage_nl_buf(int key, U8 src, U8 seq)				//����nl_buff������key, src, seq�������Ӧ��buff_pool���
{
	time_t 	ctime;
	ctime = time(NULL);
	int id = key % 11;
	int i;
	
	if(nl_buf_pool[id].flag == 1 && nl_buf_pool[id].seq == seq && nl_buf_pool[id].src == src)		//��ͨ��key%11���ٲ���
	{
		nl_buf_pool[id].time = ctime;
		EPT(stderr,"  Recombine1 find id:%d\n", id);
		return id;
	}
	else															//������ҵĲ��Ƕ�Ӧ��seq��src��ϣ�����ѯ����
	{
		for(i = 0;i < nl_buff_num; i++)
		{
			if(nl_buf_pool[i].flag == 1 && nl_buf_pool[i].seq == seq && nl_buf_pool[i].src == src)
			{
				nl_buf_pool[i].time = ctime;
				EPT(stderr,"  Recombine2 find id:%d\n", i);
				return i;
			}
		}
	}

	if(nl_buf_pool[id].flag == 0)									//�����û�ҵ������������ģ����ȳ��Էŵ�key%11��
	{
		memset(&nl_buf_pool[id], 0, sizeof(nl_buf_pool));
		nl_buf_pool[id].seq = seq;
		nl_buf_pool[id].src = src;
		nl_buf_pool[id].flag = 1;
		nl_buf_pool[id].time = ctime;
		EPT(stderr,"  Recombine3 set id:%d\n", id);
		return id;
	}

	for(i = 0;i < nl_buff_num; i++)									//key%11��ռ�����0��ʼ��һ�����Էŵ�
	{
		if(ctime - nl_buf_pool[i].time > nl_buff_timeout)
			nl_buf_pool[i].flag = 0;
		if(nl_buf_pool[i].flag == 0)								//����ҵ����л�������ղ�ԤԼ�������
		{
			memset(&nl_buf_pool[i], 0, sizeof(nl_buf_pool));
			nl_buf_pool[i].seq = seq;
			nl_buf_pool[i].src = src;
			nl_buf_pool[i].flag = 1;
			nl_buf_pool[i].time = ctime;
			return i;
		}
	}

	return -1;
}

void set_nl2other_mtype(mmsg_t *snd_buf, nl_package_t *pkt)
{
	U8 type = get_TYPE(pkt);
	ASSERT(type == 0);
	U8 subType = get_SubT(pkt);
	switch(subType)
	{
		case 0:
			snd_buf->mtype = MMSG_URP_DATA;
			break;
		case 1:
			snd_buf->mtype = MMSG_REF_DATA;
			break;
		case 2:
			snd_buf->mtype = MMSG_RIP_DATA;
			break;
		case 4:
			snd_buf->mtype = MMSG_MRP_DATA;
			break;
		case 5:
			snd_buf->mtype = MMSG_RIPC_DATA;
		default:
			EPT(stderr,"no subType match,subType:%d", subType);
			break;
	}
	//5.13 EPT(stderr," NL to IP mtype:%ld",snd_buf->mtype);
}

int send_mtu2Hm()
{
	sleep(1); 	//in case HighMAC not work yet
	
	mmsg_t * snd_buf;
	snd_buf = (mmsg_t *)malloc(sizeof(mmsg_t));
	
	if (snd_buf == NULL)
	{
		EPT(stderr, "!!! malloc error when deal sigle pkt\n");
		return 0;
	}
	memset(snd_buf, 0, sizeof(snd_buf));
	
	/*if file doesn't exist or have no content, use the only one MIN_MTU_GRADE to set MTU*/
	if(0 == speed_level)    	
	{
		speed_level = 1;
		mtu_grade[0] = MIN_MTU_GRADE;			//����HighMAC��LowMAC��Ϊ�ܳ���including MAC֡ͷ��CRC+4�ֽڣ�
	}
	
	snd_buf->mtype = MMSG_MTU_DATA;
	snd_buf->node = speed_level;
	memcpy(snd_buf->data, mtu_grade, speed_level*2);
	
	while(msgsnd(hm_qid, snd_buf, speed_level*2 + 1, 0) < 0)
	{
		if (errno == EINTR)
			continue;
		else
			{
				EPT(stdout, "!!!%s:------snd mtu_grade to himac wrong------------\n", qinfs[re_qin].pname);
				free(snd_buf);
				return 0;
			}
	}
	
	free(snd_buf);
	return 1;
	
}
