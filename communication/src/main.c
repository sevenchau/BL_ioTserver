/* project/main.c */  
#include <stdio.h>  
#include <pthread.h> 
#include <unistd.h>

#include "../inc/bs_server_net.h"  

#define      MAX_PTHRD_NUM              1

pthread_t pthread_pkt_distbt[MAX_PTHRD_NUM] = {0};

#define   SUCCESS            0
#define   FAILED             -1

int pthreadCreateInit(void)
{
	unsigned int i = 0;
	int ret = 0;
	
    for (i = 0; i < MAX_PTHRD_NUM; i++) {
        ret = pthread_create(&pthread_pkt_distbt[i], NULL, (void * (*)(void *))thread_pkt_downlink, (void *)i);
	    if (ret != 0) {
			return FAILED;
	    }
    }
	
	return SUCCESS;
}

// int main(int argc,char *argv[])
int main(void)
{  
	if (SUCCESS != pthreadCreateInit()) {
		printf("pthread_create error!!\r\n");  
	}
	else {
		printf("pthread_create success\r\n");  
	}

	printf("running"); 
	while(1) {
		printf("."); 
		usleep(1000*1000);
	}
    return 0;  
}  