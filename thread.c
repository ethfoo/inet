#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

void* thread1(void *arg);

int main(int argc, int *argv[])
{
	pthread_t t_id;
	int thread_param = 5;
	void* thr_ret;

	if( pthread_create(&t_id, NULL, thread1, (void*)&thread_param) != 0)
	{
		perror("create error");	
	}

	if( pthread_join(t_id, &thr_ret) != 0)
	{
		perror("join error");
	}

	printf("return message: %s\n", (char*)thr_ret);
	puts("end of main");
	return 0;
}

void* thread1(void *arg)
{
	int i;
	int cnt = *((int*)arg);
	for(i=0; i<cnt; i++)
	{
		sleep(1);
		puts("running thread1");
	}
	return "this is return";
}
