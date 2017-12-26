#include<stdio.h>
#include<stdlib.h>


typedef struct QNode
{
    void *data;
    struct QNode *next;
}QNode,*QueuePtr;

typedef struct
{
    QueuePtr Front,Rear;
}LinkQueue;

int Init_Q(LinkQueue *q)
{
    q->Front=q->Rear=(QueuePtr)malloc(sizeof(QNode));
    if(!q->Front)
        exit(0);
    q->Front->next=NULL;
    return 1;
}

int Insert_Q(LinkQueue *q,void *data)
{
    QueuePtr p;
    p=(QueuePtr)malloc(sizeof(QNode));
    if(p==NULL)
        exit(0);
    p->data=data;
	printf("Insert_Q %p\n",data);
    p->next=NULL;
    q->Rear->next=p;
    q->Rear=p;
    return 1;
}

int Delete_Q(LinkQueue *q,void *data)
{
    QueuePtr p;
    p=q->Front->next;
    data=p->data;
	printf("Delete_Q %p\n",data);
    q->Front->next=p->next;
    if(q->Rear==p)
        q->Rear=q->Front;
    free(p);
    return 1;
}

int Is_Empty_Q(LinkQueue *q)
{
    if(q->Rear==q->Front)
    {
        //printf("empty q\n");
        return 1;
    }

    else
        return 0;
}
