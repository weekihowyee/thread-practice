#ifndef __MAIN_THREAD_H__
#define __MAIN_THREAD_H__


#define BUFFER_SIZE 10

typedef enum {
  EMPTY_Q_TYPE,
  DONE_Q_TYPE
  } buffer_q_type_t;

typedef enum {
  MSG_PRODUCT_INIT_BUFFER,
  MSG_WRITE,
  MSG_READ
  } thread_msg_type_t;

typedef enum {
  THREAD_TYPE_PRODUCT,
  THREAD_TYPE_CONSUMER,
  THREAD_TYPE_BOTH
  } thread_type_t;

typedef enum {
  THREAD_CMD_INIT,
  THREAD_CMD_WRITE,
  THREAD_CMD_READ
  } thread_cmd_type_t;

typedef struct {

    thread_cmd_type_t type;

    void *cmd_data;

}main_cmd;

typedef struct {
	
  	thread_msg_type_t type;

  	void *msg_data;

}thread_msg_t;

typedef struct {  //thread private data
	
  void *parent;

  thread_type_t thread_type;

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
