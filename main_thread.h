#ifndef __MAIN_THREAD_H__
#define __MAIN_THREAD_H__


#define BUFFER_SIZE 10

typedef enum {
  MSG_PRODUCT_INIT_BUFFER,
  MSG_WRITE,
  MSG_READ
  } thread_msg_type_t;

typedef struct {
	
  	thread_msg_type_t type;

  	void *msg_data;

}thread_msg_t;

typedef struct {  //thread private data
	
  thread_ctl *parent;

	LinkQueue *Q_Msg;

	pthread_mutex_t msg_q_lock;

	pthread_cond_t  thread_cond;

	//pthread_t       thread_id;

} private_t;

typedef struct {	// common data

        int start;

        int buffer_num;

        LinkQueue *Q_Empty;  //BUFFER Q

        LinkQueue *Q_Done;  //BUFFER Q

        private_t *product_priv;

        private_t *consumer_priv;

} thread_ctl;

#endif
