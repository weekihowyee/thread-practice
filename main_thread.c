#include "stdio.h"
#include "stdlib.h"
#include <pthread.h>
#include "module_queue.h"
#include "main_thread.h"

int enqueue_msg(private_t *priv,thread_msg_t *msg)
{

	printf("enqueue_msg E , %d\n",msg->type);

	pthread_mutex_lock(&priv->msg_q_lock);
	Insert_Q(priv->Q_Msg,(void*)msg);
	pthread_mutex_unlock(&priv->msg_q_lock);

	pthread_mutex_lock(&priv->msg_q_lock);
	pthread_cond_signal(&priv->thread_cond);
	pthread_mutex_unlock(&priv->msg_q_lock);

	printf("enqueue_msg L\n");

	return 1;
}

int dequeue_msg(private_t *priv , void *msg)
{
	Delete_Q(priv->Q_Msg,msg);
	return 1;
}

int Transfer_Cmd_To_Msg(main_cmd *cmd,thread_msg_t *msg)
{

	printf("Transfer_Cmd_To_Msg E\n");
	if(cmd->type == THREAD_CMD_INIT)
	{
		printf("Transfer_Cmd_To_Msg MSG_PRODUCT_INIT_BUFFER %d\n",*((int *)msg->msg_data));
		msg->type = MSG_PRODUCT_INIT_BUFFER;
		msg->msg_data = cmd->cmd_data;
	}
	if(cmd->type == THREAD_CMD_WRITE)
	{
		msg->type = MSG_WRITE;
		msg->msg_data = cmd->cmd_data;
	}
	if(cmd->type == THREAD_CMD_READ)
	{
		msg->type = MSG_READ;
	}

	printf("Transfer_Cmd_To_Msg L\n");

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
	int i;
	printf("Process_Msg: ENTER %d\n",msg->type);
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
			printf("Process_Msg: Init Buffer %d\n",*num);
			num = (int *)msg->msg_data;
			if(*num>10)
				*num = 10;
			printf("Process_Msg: Init Buffer %d\n",*num);
			for(i=0;i<(*num);i++)  // alloc init buffers
			{
				printf("Process_Msg alloc\n");
				buf=(char *)malloc(BUFFER_SIZE); 
				printf("Process_Msg \n");
				memset(buf,0,BUFFER_SIZE);
				enqueue_buffer((thread_ctl *)priv->parent,buf,EMPTY_Q_TYPE);
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
			dequeue_buffer((thread_ctl *)priv->parent,buf,EMPTY_Q_TYPE);
			memcpy(buf,data,strlen(data));
			enqueue_buffer((thread_ctl *)priv->parent,buf,DONE_Q_TYPE);
			break;
		}

		case MSG_READ:
		{
			if(priv->thread_type == THREAD_TYPE_PRODUCT)
			{
				printf("Process_Msg: THREAD_TYPE_PRODUCT , Pass\n");
				break;
			}
			dequeue_buffer((thread_ctl *)priv->parent,buf,DONE_Q_TYPE);
			memcpy(buf,msg->msg_data,strlen(msg->msg_data));
			puts(buf);
			memcpy(buf,0,BUFFER_SIZE);
			enqueue_buffer((thread_ctl *)priv->parent,buf,EMPTY_Q_TYPE);
			break;
		}
	}

	return 1;
}

int product_thread_handler(thread_ctl *t_ctl)
{

	int i,exit_flag=0;
	thread_msg_t *msg = (thread_msg_t *)malloc(sizeof(thread_msg_t));
	
	printf("enter product_thread_handler\n");

	while(!exit_flag)
	{
		pthread_mutex_lock(&t_ctl->product_priv->msg_q_lock);
		while(Is_Empty_Q(t_ctl->product_priv->Q_Msg))
		{
			pthread_cond_wait(&t_ctl->product_priv->thread_cond,&t_ctl->product_priv->msg_q_lock);
		}
		pthread_mutex_unlock(&t_ctl->product_priv->msg_q_lock);

		printf("Get Msg for product\n");

		pthread_mutex_lock(&t_ctl->product_priv->msg_q_lock);
		dequeue_msg(t_ctl->product_priv,(void *)msg);
		pthread_mutex_unlock(&t_ctl->product_priv->msg_q_lock);

		printf("PROC Msg for product %p\n",msg);
		Process_Msg(t_ctl->product_priv,(thread_msg_t *)msg);

	}

	return 0;

}

int consumer_thread_handler(thread_ctl *t_ctl)
{

	int i,exit_flag=0;
	void *msg;

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
		dequeue_msg(t_ctl->consumer_priv,msg);
		pthread_mutex_unlock(&t_ctl->consumer_priv->msg_q_lock);

		Process_Msg(t_ctl->consumer_priv,(thread_msg_t *)msg);

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
	t_ctl->product_priv->parent = (void *)t_ctl;
	t_ctl->consumer_priv->parent = (void *)t_ctl;

	t_ctl->product_priv->thread_type = THREAD_TYPE_PRODUCT;
	t_ctl->consumer_priv->thread_type = THREAD_TYPE_CONSUMER;

	return 1;
}

int process_cmd(thread_ctl *t_ctl,main_cmd *cmd)
{
	char in_buf[10];
	int num;
	thread_msg_t *msg = (thread_msg_t *)malloc(sizeof(thread_msg_t));
	INPUT:
	puts("Input your cmd! THREAD_CMD_INIT THREAD_CMD_WRITE THREAD_CMD_READ");
	gets(in_buf);

	if(strcmp(in_buf,"THREAD_CMD_INIT") == 0)
	{
		cmd->type = THREAD_CMD_INIT;
		puts("Input number ");
		gets(in_buf);
		num = atoi(in_buf);
		printf("want %d num bufs",num);
		cmd->cmd_data =(void *)(&num);
		Transfer_Cmd_To_Msg(cmd,msg);
		enqueue_msg(t_ctl->product_priv,msg);
		sleep(3);
		goto INPUT;
	}

	if(strcmp(in_buf,"THREAD_CMD_WRITE") == 0)
	{
		cmd->type = THREAD_CMD_WRITE;
		puts("Input Text ");
		gets(in_buf);
		cmd->cmd_data =(void *)(in_buf);
		Transfer_Cmd_To_Msg(cmd,msg);
		enqueue_msg(t_ctl->product_priv,msg);
		sleep(3);
		goto INPUT;
	}

	if(strcmp(in_buf,"THREAD_CMD_READ") == 0)
	{
		cmd->type = THREAD_CMD_READ;
		cmd->cmd_data =NULL;
		Transfer_Cmd_To_Msg(cmd,msg);
		enqueue_msg(t_ctl->consumer_priv,msg);
		sleep(3);
		goto INPUT;
	}
	
	//cmd->type = ;
	//cmd->cmd_data =(void *) ;

	//return x;  // If cmd come , please set x to 1.
}
int main()
{

	int exit_main=0;

	thread_ctl t_ctl;

	main_cmd *cmd = (main_cmd *)malloc(sizeof(main_cmd));

	Init_Ctl_Data(&t_ctl);

	thread_create_entry(&t_ctl);

	sleep(3); // delay to make sure thread creating successfully

	printf("create thread done\n");

	process_cmd(&t_ctl,cmd);

	return 0;
}

