#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include  <stdlib.h>


struct element { int payload; struct element* next;};
typedef struct element element_t;
element_t *head=0, *tail=0;
int size=0, max_size;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sent = PTHREAD_COND_INITIALIZER;
pthread_cond_t recv = PTHREAD_COND_INITIALIZER;

/* 
  message sender with locking mechanism
*/
void send(int message){
  pthread_mutex_lock(&mutex);

  while(size >= max_size){
    pthread_cond_wait(&recv, &mutex);
  }

  if(head==0) {
   head = malloc(sizeof(element_t));
   head->payload = message;
   head->next = 0;
   tail = head;
  } else {
   tail->next = malloc(sizeof(element_t));
   tail = tail->next;
   tail-> payload = message;
   tail->next = 0;
  }
  size++;

  pthread_cond_signal(&sent);

  pthread_mutex_unlock(&mutex);
}

/* 
   message reception mechanism
*/
int get(){
  element_t* element;
  int result;

  pthread_mutex_lock(&mutex);

  while(size ==0){
    pthread_cond_wait(&sent, &mutex);
  }

  result = head->payload;
  element = head;
  head = head->next;
  free(element);
  size--;

  pthread_cond_signal(&recv);

  pthread_mutex_unlock(&mutex);
  return result;
} 

/*
  produces 30 random numbers
*/

void* producer(void* arg) {
  int i;
//while(1){
  for (i=0; i < 50; i++){
   if(size <= max_size){
    send(i);
   }else{
    printf("message buffer is full! can not send %dth message\n", i);
   }
  }
  return NULL;
}

/*
  consumes generated messages.
*/
void* consumer(void* arg){
  while(1){
    printf("buffer size:%d received %d\n", size, get());
  }
  return NULL;
}

int main(int argc, char **argv){
  pthread_t thread1, thread2;

  void* exitStatus;
  if(argc <=0){
    printf("program expects max message size in integer!");
  }
  max_size = atoi(argv[1]);
  printf("Maximum message buffering capacity of the system is: %d\n", max_size);
  
  //pthread_mutex_init(&mutex, NULL);
  pthread_create(&thread1, NULL, producer, NULL);
  pthread_create(&thread2, NULL, consumer, NULL);

  pthread_join(thread1, &exitStatus);
  pthread_join(thread2, &exitStatus);

  return 0;
}
