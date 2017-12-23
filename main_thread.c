#include "stdio.h"
#include "stdlib.h"
#include <pthread.h>

typedef struct {

        int start;

} thread_ctl;

int product_thread_handler(thread_ctl *t_ctl)
{

	printf("enter product_thread_handler\n");

	return 0;

}

int consumer_thread_handler(thread_ctl *t_ctl)
{

	printf("enter consumer_thread_handler\n");

	return 0;	
	
}

int thread_create_product(thread_ctl *t_ctl)
{
	pthread_t thread_product,thread_consumer;

	pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


	pthread_create(&thread_product,&attr,product_thread_handler,t_ctl);  //create product thread

	//pthread_create(&thread_consumer,&attr,consumer_thread_handler,t_ctl);  //create consumer thread

	return 0;

}

int thread_create_consumer(thread_ctl *t_ctl)
{
	pthread_t thread_product,thread_consumer;

	pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


	//pthread_create(&thread_product,&attr,product_thread_handler,t_ctl);  //create product thread

	pthread_create(&thread_consumer,&attr,consumer_thread_handler,t_ctl);  //create consumer thread

	return 0;

}

int thread_create_entry(thread_ctl *t_ctl)
{

	thread_create_consumer(t_ctl);

	thread_create_product(t_ctl);

	return 0;

}

int main()
{
	thread_ctl t_ctl;

	thread_create_entry(&t_ctl);

	sleep(3); // delay to make sure thread creating successfully

	printf("create thread done\n");

	return 0;
}

