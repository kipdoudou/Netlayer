#include "nl_common.h"


int speed_level = 0;
U16 mtu_grade[MAX_SPEED_LEVEL];

char f_name[] = "/home/root/rata_config";
int flag_mtu = 0;

int mtu_file_len()
{
	FILE *fp_port = NULL;
	char c;
	int j = 0;

	fp_port = fopen(f_name, "r");
	if (NULL == fp_port)
	{
		EPT(stderr,"!!!! file: %s does't exist\n", f_name);
		return -1;
	}
	else
	{
		while((c = fgetc(fp_port)) != EOF)
		{
			j = (int)(c);
			EPT(stderr,"@ j: '%d'\n",j);
			if(j == 255)
				break;
			EPT(stderr,"! read : '%c'\n", c);

			if(c == '\n')
			{
				speed_level++;
			}
		}
		EPT(stderr,"the size of file is %d\n", speed_level);
		if(speed_level == 0)
		{
			EPT(stderr,"!!! file: %s does't have word\n", f_name);
			return 0;
		}
		return 1;
	}
	fclose(fp_port);
	
}

int read_mtu_file()
{
	int rval = 0;
	FILE *fp_port = NULL;

	fp_port = fopen(f_name, "r");
	if (NULL == fp_port)
	{
		rval = 1;
		goto fexit;
	//	printf("~~~~~~does not exist file:%s\n",f_name);
		
	}
	else
	{
		int i = 0;
		while(!feof(fp_port))
		{
			int mtu;
			fscanf(fp_port, "%d", &mtu);

			if(!feof(fp_port))
		//	printf("%d, %d, %d\n", pt, port, Cos);
			{
				mtu_grade[i] = mtu < MAX_PACKAGE_DATA_LENGTH ? (U16)mtu : MAX_PACKAGE_DATA_LENGTH;
				i++;
			}

			rval = 2;
		
		}
	}
fexit:
	if (NULL != fp_port)
		fclose(fp_port);
	return rval;
}

void sort_mtu(int bot, int end)
{
    int i,j,temp;  
    if(bot < end)  
    {  
		i = bot + 1;  
		j = end;  
		while(1)  
		{  
       		while( !( mtu_grade[i] >= mtu_grade[bot] || i == end ) )  
           		i++;  
            while( !( mtu_grade[j] <= mtu_grade[bot] || j == bot ) )  
           		j--;  
            if(i < j)  
            {  
            	temp = mtu_grade[j];  
            	mtu_grade[j] = mtu_grade[i];  
            	mtu_grade[i] = temp;  
            }  
            else  
           		break;  
       }  
       temp = mtu_grade[bot];  
       mtu_grade[bot] = mtu_grade[j];  
       mtu_grade[j] = temp;  
       sort_mtu(bot,j-1);  
       sort_mtu(j+1,end);  
    } 
}

void show_mtu_grade()
{
	int i;
	for( i = 0; i < speed_level; i++)
	{
		EPT(stderr,"mtu_grade[%d] = %d\n", i, mtu_grade[i]);
	}
}

int init_mtu()
{
    if( mtu_file_len() == 1)
    {
    	int i;
		for(i = 0; i < MAX_SPEED_LEVEL; i++)
			mtu_grade[i] = 0;

		read_mtu_file();

		if(mtu_grade[0] == 0 && mtu_grade[1] == 0)//in case file start with\n
		{
			speed_level = 0;
			return 0;
		}

		sort_mtu(0, speed_level - 1);

		show_mtu_grade();  
		return 1; 	
    }
    flag_mtu = 1;
    return 0;
}