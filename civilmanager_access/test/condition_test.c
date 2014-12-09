#include <pthread.h>
#include <stdio.h>

int condition = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;

void * thread_func1(void *param){
	fprintf(stdout, "thread1...\n");
	pthread_mutex_lock(&mutex);
	condition = 1;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
	return NULL;
}

void * thread_func2(void *param){
	fprintf(stdout, "thread2...\n");
	pthread_mutex_lock(&mutex);
	while(condition == 0){
		fprintf(stdout, "wait for thread1...\n");
		pthread_cond_wait(&cond, &mutex); 
		fprintf(stdout, "thread1 give signal ...\n");
	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int main(){
	pthread_t tid1, tid2;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);
	pthread_create(&tid1, NULL, thread_func1, NULL);
	pthread_create(&tid2, NULL, thread_func2, NULL);

	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);

}
