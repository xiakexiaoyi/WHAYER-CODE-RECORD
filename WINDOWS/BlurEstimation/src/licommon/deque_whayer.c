#include"../inc/deque_whayer.h"
#include<malloc.h>
#include<stdio.h>

/*����һ���ն���*/
Whayer_Queue *InitQueue()
{
	Whayer_Queue *pqueue = (Whayer_Queue *)malloc(sizeof(Whayer_Queue));
	if(pqueue!=NULL)
	{
		pqueue->front = NULL;
		pqueue->rear = NULL;
		pqueue->size = 0;
	}
	return pqueue;
}

/*����һ������*/
void DestroyQueue(Whayer_Queue *pqueue)
{
	if(IsEmpty(pqueue)!=1)
		ClearQueue(pqueue);
	free(pqueue);
}

/*���һ������*/
void ClearQueue(Whayer_Queue *pqueue)
{
	while(IsEmpty(pqueue)!=1)
	{
		DeQueue(pqueue,NULL);
	}

}

/*�ж϶����Ƿ�Ϊ��*/
int IsEmpty(Whayer_Queue *pqueue)
{
	if(pqueue->front==NULL&&pqueue->rear==NULL&&pqueue->size==0)
		return 1;
	else
		return 0;
}

/*���ض��д�С*/
int GetSize(Whayer_Queue *pqueue)
{
	return pqueue->size;
}

/*���ض�ͷԪ��*/
PNode GetFront(Whayer_Queue *pqueue,Item *pitem)
{
	if(IsEmpty(pqueue)!=1&&pitem!=NULL)
	{
		*pitem = pqueue->front->data;
	}
	return pqueue->front;
}

/*���ض�βԪ��*/

PNode GetRear(Whayer_Queue *pqueue,Item *pitem)
{
	if(IsEmpty(pqueue)!=1&&pitem!=NULL)
	{
		*pitem = pqueue->rear->data;
	}
	return pqueue->rear;
}

/*����Ԫ�����*/
PNode EnQueue(Whayer_Queue *pqueue,Item item)
{
	PNode pnode = (PNode)malloc(sizeof(Node));
	if(pnode != NULL)
	{
		pnode->data = item;
		pnode->next = NULL;
		
		if(IsEmpty(pqueue))
		{
			pqueue->front = pnode;
		}
		else
		{
			pqueue->rear->next = pnode;
		}
		pqueue->rear = pnode;
		pqueue->size++;
	}
	return pnode;
}

/*��ͷԪ�س���*/
PNode DeQueue(Whayer_Queue *pqueue,Item *pitem)
{
	PNode pnode = pqueue->front;
	if(IsEmpty(pqueue)!=1&&pnode!=NULL)
	{
		if(pitem!=NULL)
			*pitem = pnode->data;
		pqueue->size--;
		pqueue->front = pnode->next;
		free(pnode);
		if(pqueue->size==0)
			pqueue->rear = NULL;
	}
	return pqueue->front;
}

/*�������в��Ը����������visit����*/
void QueueTraverse(Whayer_Queue *pqueue,void (*visit)())
{
	PNode pnode = pqueue->front;
	int i = pqueue->size;
	while(i--)
	{
		visit(pnode->data);
		pnode = pnode->next;
	}	
}
