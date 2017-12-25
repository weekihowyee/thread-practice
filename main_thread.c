#include "stdio.h"
#include "stdlib.h"
#include <pthread.h>
#include "module_queue.h"
#include "main_thread.h"

int enqueue_msg(private_t *priv,thread_msg_t *msg)
{

	pthread_mutex_lock(&priv->msg_q_lock);
	Insert_Q(priv->Q_Msg,(void*)msg);
	pthread_mutex_unlock(&thread_data->msg_q_lock);


	pthread_mutex_lock(&priv->msg_q_lock);
	pthread_cond_signal(&priv->thread_cond);
	pthread_mutex_unlock(&thread_data->msg_q_lock);

	return 1;
}

int dequeue_msg(private_t *priv , thread_msg_t *msg)
{
	Delete_Q(priv->Q_Msg,msg);
	return 1;
}

int enqueue_buffer(thread_ctl *t_ctl , char *buf ,buffer_q_type_t queue_flag)
{
	if(queue_flag == EMPTY_Q_TYPE)  // enqueue to empty queue
	{
		Insert_Q(t_ctl->Q_Empty , (void *)buf);
	}
	else  // enqueue to done queue
	{
		Insert_Q(t_ctl->Q_Done , (void *)buf);
	}

	return 0;
}

int dequeue_buffer(thread_ctl *t_ctl , char *buf ,buffer_q_type_t queue_flag)
{
	if(queue_flag == EMPTY_Q_TYPE)  // dequeue from empty queue
	{
		Delete_Q(t_ctl->Q_Empty,(void *)buf);
	}
	else  // dequeue from done queue
	{
		Delete_Q(t_ctl->Q_Done , (void *)buf);
	}

	return 0;
}

int Process_Msg(private_t *priv,thread_msg_t *msg)
{
	char *buf;
	switch(msg->type)
	{

		case MSG_PRODUCT_INIT_BUFFER:
		{
			int *num;
			
			if(priv->thread_type == THREAD_TYPE_CONSUMER)
			{
				printf("Process_Msg: THREAD_TYPE_CONSUMER , Pass\n");
				break;
			}
			num = (int *)msg->msg_data;
			for(i=0;i<(*num);i++)  // alloc init buffers
			{
				buf=(char *)malloc(BUFFER_SIZE); 
				memcpy(buf,0,BUFFER_SIZE);
				enqueue_buffer(priv->parent,buf,EMPTY_Q_TYPE);
			}	
			break;
		}

		case MSG_WRITE:
		{
			char *data = (char *)msg->msg_data;
			
			if(priv->thread_type == THREAD_TYPE_CONSUMER)
			{
				printf("Process_Msg: THREAD_TYPE_CONSUMER , Pass\n");
				break;
			}
			dequeue_buffer(priv->parent,buf,EMPTY_Q_TYPE);
			memcpy(buf,data,strlen(data));
			enqueue_buffer(priv->parent,buf,DONE_Q_TYPE);
			break;
		}

		case MSG_READ:
		{
			if(priv->thread_type == THREAD_TYPE_PRODUCT)
			{
				printf("Process_Msg: THREAD_TYPE_PRODUCT , Pass\n");
				break;
			}
			dequeue_buffer(priv->parent,buf,DONE_Q_TYPE);
			memcpy(buf,data,strlen(data));
			puts(buf);
			memcpy(buf,0,BUFFER_SIZE);
			enqueue_buffer(priv->parent,buf,EMPTY_Q_TYPE);
			break;
		}
	}

	return 1;
}

int product_thread_handler(thread_ctl *t_ctl)
{

	int i,exit_flag=0;
	thread_msg_t *msg;

	printf("enter product_thread_handler , alloc buffer\n");

	while(!exit_flag)
	{
		pthread_mutex_lock(&t_ctl->product_priv->msg_q_lock);
		while(Is_Empty_Q(t_ctl->product_priv->Q_Msg))
		{
			pthread_cond_wait(&t_ctl->product_priv->thread_cond,&t_ctl->product_priv->msg_q_lock);
		}
		pthread_mutex_unlock(&t_ctl->product_priv->msg_q_lock);

		pthread_mutex_lock(&t_ctl->product_priv->msg_q_lock);
		dequeue_msg(&t_ctl->product_priv,msg);
		pthread_mutex_unlock(&t_ctl->product_priv->msg_q_lock);

		Process_Msg(&t_ctl->product_priv,msg);

	}

	return 0;

}

int consumer_thread_handler(thread_ctl *t_ctl)
{

	int i,exit_flag=0;
	thread_msg_t *msg;

	printf("enter consumer_thread_handler %d\n");

	while(!exit_flag)
	{
		pthread_mutex_lock(&t_ctl->consumer_priv->msg_q_lock);
		while(Is_Empty_Q(t_ctl->consumer_priv->Q_Msg))
		{
			pthread_cond_wait(&t_ctl->consumer_priv->thread_cond,&t_ctl->consumer_priv->msg_q_lock);
		}
		pthread_mutex_unlock(&t_ctl->consumer_priv->msg_q_lock);

		pthread_mutex_lock(&t_ctl->consumer_priv->msg_q_lock);
		dequeue_msg(&t_ctl->consumer_priv,msg);
		pthread_mutex_unlock(&t_ctl->consumer_priv->msg_q_lock);

		Process_Msg(&t_ctl->consumer_priv,msg);

	}

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

int Init_Private_Data(private_t *priv)
{

	pthread_mutex_init(&priv->msg_q_lock, NULL);

	priv->Q_Msg = (LinkQueue *)malloc(sizeof(LinkQueue));
	Init_Q(priv->Q_Msg);

	pthread_cond_init(&priv->thread_cond, NULL);

	return 1;

}

int Init_Ctl_Data(thread_ctl *t_ctl)
{
	t_ctl->buffer_num = 6;

	t_ctl->Q_Done = (LinkQueue *)malloc(sizeof(LinkQueue));
	t_ctl->Q_Empty = (LinkQueue *)malloc(sizeof(LinkQueue));
	Init_Q(t_ctl->Q_Done);
	Init_Q(t_ctl->Q_Empty);

	/* create thread private data */
	t_ctl->product_priv = (private_t *)malloc(sizeof(private_t));
	t_ctl->consumer_priv = (private_t *)malloc(sizeof(private_t));
	Init_Private_Data(t_ctl->product_priv);
	Init_Private_Data(t_ctl->consumer_priv);
	t_ctl->product_priv->parent = t_ctl;
	t_ctl->consumer_priv->parent = t_ctl;

	t_ctl->product_priv->thread_type = THREAD_TYPE_PRODUCT;
	t_ctl->product_priv->thread_type = THREAD_TYPE_CONSUMER;

	return 1;
}

int process_cmd(main_cmd *cmd)
{
	// write your code here

	//cmd->type = ;
	//cmd->cmd_data =(void *) ;

	//return x;  // If cmd come , please set x to 1.
}
int main()
{

	int exit_main=0;

	thread_ctl t_ctl;

	Init_Ctl_Data(&t_ctl);

	thread_create_entry(&t_ctl);

	sleep(3); // delay to make sure thread creating successfully

	printf("create thread done\n");

	return 0;
}

