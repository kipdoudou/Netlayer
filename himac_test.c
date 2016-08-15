#include "mr_common.h"

typedef struct
{
		//U8 PR_TYPE_SubT;
		U8 SubT : 4;
		U8 TYPE : 2;
		U8 PR : 2;	
		U8 rcv_addr;
		U8 snd_addr;
		U8 dst_addr;
		U8 src_addr;
		//U8 SEQ_H;		
		U8 H : 1;
		U8 SEQ : 7;
		//U8 SN_TTL;
		U8 TTL : 4;
		U8 SN : 4;		
		//U8 CoS_ACK_Rev;					//ACK和REV在HIGHMAC部分处理
		//U8 Rev : 4;
		U8 SSN : 3;
		U8 SH : 1;
		U8 ACK : 1;
		U8 CoS : 3;		
		
		char data[1592];
//		unsigned int CRC;
}nl_package_t;

int main(int argc, char * argv[])
{

	mmsg_t snd_buf;
	mmsg_t rcv_buf;

	snd_buf.mtype = MMSG_HM_DATA;
	snd_buf.node = 1;

	mr_queues_init("highmac");

	char str1[9] = "111111111";
	char str2[9] = "222222222";
	char str3[5] = "33333";

	mmsg_t test_msg;
	test_msg.mtype = MMSG_SEG_DATA;
	nl_package_t * pkt = (nl_package_t *)test_msg.data;

/*
	pkt->PR = 0;
	pkt->TYPE=0;
	pkt->SubT = 1;
	pkt->rcv_addr = 1;
	pkt->snd_addr = 1;
	pkt->dst_addr= 1;
	pkt->src_addr = 1;
	pkt->SEQ = 8;
	pkt->H = 1;
	pkt->SN = 1;
	pkt->CoS = 2;
	pkt->TTL = 7;
	pkt->ACK = 0;
	pkt->SH = 1;
	pkt->SSN = 0;

	memcpy(pkt->data, str1, 9);
	while(msgsnd(nl_qid, &test_msg, 18, 0) < 0)
	{
		if (errno == EINTR)
			continue;
		else
			{
				EPT(stdout, "!!!!!!!!!!!!!!!!!------snd 1 to himac wrong------------\n");
			}
	}
	

	pkt->PR = 0;
	pkt->TYPE=0;
	pkt->SubT = 1;
	pkt->rcv_addr = 1;
	pkt->snd_addr = 1;
	pkt->dst_addr= 1;
	pkt->src_addr = 1;
	pkt->SEQ = 8;
	pkt->H = 0;
	pkt->SN = 1;
	pkt->CoS = 2;
	pkt->TTL = 7;
	pkt->ACK = 0;
	pkt->SH = 1;
	pkt->SSN = 0;	
	memcpy(pkt->data, str2, 9);
	while(msgsnd(nl_qid, &test_msg, 18, 0) < 0)
	{
		if (errno == EINTR)
			continue;
		else
			{
				EPT(stdout, "!!!!!!!!!!!!!!!!!------snd 2 to himac wrong------------\n");
			}
	}
*/
	pkt->PR = 0;
	pkt->TYPE=0;
	pkt->SubT = 1;
	pkt->rcv_addr = 1;
	pkt->snd_addr = 1;
	pkt->dst_addr= 1;
	pkt->src_addr = 1;
	pkt->SEQ = 8;
	pkt->H = 1;
	pkt->SN = 2;
	pkt->CoS = 2;
	pkt->TTL = 7;
	pkt->ACK = 0;
	pkt->SH = 1;
	pkt->SSN = 0;	
	memcpy(pkt->data, str1, 9);
	while(msgsnd(nl_qid, &test_msg, 18, 0) < 0)
	{
		if (errno == EINTR)
			continue;
		else
			{
				EPT(stdout, "!!!!!!!!!!!!!!!!!------snd 2 to himac wrong------------\n");
			}
	}

	pkt->PR = 0;
	pkt->TYPE=0;
	pkt->SubT = 1;
	pkt->rcv_addr = 1;
	pkt->snd_addr = 1;
	pkt->dst_addr= 1;
	pkt->src_addr = 1;
	pkt->SEQ = 8;
	pkt->H = 0;
	pkt->SN = 1;
	pkt->CoS = 2;
	pkt->TTL = 7;
	pkt->ACK = 0;
	pkt->SH = 1;
	pkt->SSN = 0;	
	memcpy(pkt->data, str2, 9);
	while(msgsnd(nl_qid, &test_msg, 18, 0) < 0)
	{
		if (errno == EINTR)
			continue;
		else
			{
				EPT(stdout, "!!!!!!!!!!!!!!!!!------snd 2 to himac wrong------------\n");
			}
	}
		pkt->PR = 0;
	pkt->TYPE=0;
	pkt->SubT = 1;
	pkt->rcv_addr = 1;
	pkt->snd_addr = 1;
	pkt->dst_addr= 1;
	pkt->src_addr = 1;
	pkt->SEQ = 8;
	pkt->H = 0;
	pkt->SN = 2;
	pkt->CoS = 2;
	pkt->TTL = 7;
	pkt->ACK = 0;
	pkt->SH = 1;
	pkt->SSN = 0;	
	memcpy(pkt->data, str3, 5);
	while(msgsnd(nl_qid, &test_msg, 14, 0) < 0)
	{
		if (errno == EINTR)
			continue;
		else
			{
				EPT(stdout, "!!!!!!!!!!!!!!!!!------snd 2 to himac wrong------------\n");
			}
	}

    int rcv_size ;
	while (1)
	{
	    printf("===========himac_test waiting for data from nl_layer==========\n");

		rcv_size=msgrcv(hm_qid, &rcv_buf, sizeof(mmsg_t) - sizeof(long), 0, 0);
		if (rcv_size < 0)
            break;
		printf("\n");
		printf("himac receive %d bytes\n", rcv_size);
		printf("himac receive rcv_buff->mtype = %ld\n" ,rcv_buf.mtype);
		printf("himac receive rcv_buff->node_addr = %d\n" ,rcv_buf.node);

		nl_package_t * pkt;
		pkt = (nl_package_t *)(rcv_buf.data);

		printf("himac receive pkt->data = %s\n",pkt->data);
		printf("pkt PR: %d\n",pkt->PR);
		printf("pkt TYPE: %d\n",pkt->TYPE);
		printf("pkt SubT: %d\n",pkt->SubT);
		printf("pkt rcv_addr: %d\n",pkt->rcv_addr);
		printf("pkt snd_addr: %d\n",pkt->snd_addr);
		printf("pkt dst_addr: %d\n",pkt->dst_addr);
		printf("pkt src_addr: %d\n",pkt->src_addr);
		printf("pkt SEQ: %d\n",pkt->SEQ);
		printf("pkt H: %d\n",pkt->H);
		printf("pkt SN: %d\n",pkt->SN);
		printf("pkt CoS: %d\n",pkt->CoS);
		printf("pkt SH: %d\n",pkt->SH);
		printf("pkt SSN: %d\n",pkt->SSN);	

		sleep(1);
        printf("sleep 1s for send back\n");

        printf("size of rcv data : %zu\n",strlen((char *)(rcv_buf.data)));

        memcpy( snd_buf.data,rcv_buf.data,rcv_size - sizeof(MADR) );// cant use strcpy!!
        //strcpy(snd_buf.data,"hello ,this is himac\n");
        //strcpy(snd_buf.data,(char *)(rcv_buf.data));

        printf("size of snd data : %zu\n",strlen(snd_buf.data));




        printf("himac send data = %s\n",&snd_buf.data[8]);

		int k = msgsnd(nl_qid, &snd_buf, rcv_size, 0);

		if(k == 0)
            printf("himac send already \n");
		else
            printf("himac send error\n");

	}
	return 0;
}

