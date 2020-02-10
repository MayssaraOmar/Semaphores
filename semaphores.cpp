#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define SHARED 1

using namespace std;

vector<int> buffer; /* shared buffer */
int bufferSize = 1; /* Default */
int cnt = 0; /* shared counter */
sem_t cnt_sem; /* counter semaphore */
sem_t emptySlotes, fullSlotes, buffer_sem; /* buffer semaphores */

void* counter(void* args){
	int counterID = *((int*) args);
	string output;
	while(true){
		int r = rand()%10;
		this_thread::sleep_for(chrono::seconds(r));
		output = "Counter Thread " + to_string(counterID) + ": received a message\n";
		write(1, output.c_str(), output.size());
		/* wait for shared counter */
		int c;
		sem_getvalue(&cnt_sem, &c);
		if(c <= 0) {
			output = "Counter Thread " + to_string(counterID) + ": waiting to write\n";
			write(1, output.c_str(), output.size());
		}
		sem_wait(&cnt_sem);
		/* BEG: critical section */
		cnt ++;
		output = "Counter Thread " + to_string(counterID) + ": now adding to counter, counter value = " + 			to_string(cnt) + "\n";
		write(1, output.c_str(), output.size());
		/* END: critical section */
		sem_post(&cnt_sem);
	}
}
void* monitor(void* args){
	int in = 0;
	string output;
	while(true){
		int tempCnt;
		int r = rand()%10000;
		this_thread::sleep_for(chrono::milliseconds(r));

		/* wait for shared counter */
		int c;
		sem_getvalue(&cnt_sem, &c);
		if(c <= 0) {
			output = "Monitor Thread: waiting to read counter\n";
			write(1, output.c_str(), output.size());
		}
		sem_wait(&cnt_sem);
		/* BEG: critical section */
		output = "Monitor Thread: reading a count value of " + to_string(cnt) + "\n";
		write(1, output.c_str(), output.size());
		tempCnt = cnt;
		cnt = 0;
		/* END: critical section */
		sem_post(&cnt_sem);

		/* check if buffer is full */
		int empty;
		sem_getvalue(&emptySlotes, &empty);
		if(empty <= 0) {
			output = "Monitor Thread: buffer is full\n";
			write(1, output.c_str(), output.size());
		}
		sem_wait(&emptySlotes);
		/* wait for buffer */
		int buf;
		sem_getvalue(&buffer_sem, &buf);
		if(buf <= 0) {
			output = "Monitor Thread: waiting for buffer\n";
			write(1, output.c_str(), output.size());
		}
		sem_wait(&buffer_sem);
		/* BEG: critical section */
		buffer[in] = tempCnt;
		output = "Monitor Thread: writing to buffer at position " + to_string(in) + ", with value " + 			to_string(buffer[in]) + "\n";
		write(1, output.c_str(), output.size());
		in = (in + 1) % bufferSize;
		/* END: critical section */
		sem_post(&buffer_sem);
		sem_post(&fullSlotes);
	}
}
void* collector(void* args){
	int out = 0;
	string output;
	while(true){
		int r = rand()%10000;
		this_thread::sleep_for(chrono::milliseconds(r));
		/* check if buffer is empty */
		int full;
		sem_getvalue(&fullSlotes, &full);
		if(full <= 0) {
			output = "Collector Thread: buffer is empty\n";
			write(1, output.c_str(), output.size());
		}
		sem_wait(&fullSlotes);
		/* wait for buffer */
		int buf;
		sem_getvalue(&buffer_sem, &buf);
		if(buf <= 0) {
			output = "Collector Thread: waiting for buffer\n";
			write(1, output.c_str(), output.size());
		}
		sem_wait(&buffer_sem);
		/* BEG: critical section */
		output = "Collector Thread: reading from buffer at position " + to_string(out) + ", with value " + 			to_string(buffer[out]) + "\n";
		write(1, output.c_str(), output.size());
		out = (out + 1) % bufferSize;
		/* END: critical section */
		sem_post(&buffer_sem);
		sem_post(&emptySlotes);
	}
}
int main(){
	srand(time(NULL));
	int n;
	cin >> n >> bufferSize;
	buffer.resize(bufferSize);
	/* initiate semaphores */
	sem_init(&cnt_sem, SHARED, 1);
	sem_init(&buffer_sem, SHARED, 1);
	sem_init(&fullSlotes, SHARED, 0);
	sem_init(&emptySlotes, SHARED, bufferSize);
	/* create threads */
	pthread_t monitor_th;
	pthread_create(&monitor_th, NULL, monitor, NULL);
	pthread_t collector_th;
	pthread_create(&collector_th, NULL, collector, NULL);
	pthread_t counters[n];
	for(int i=0; i<n; i++){
		int* counterID = (int*) malloc(sizeof(int));
		*counterID = i + 1;
		pthread_create(&counters[i], NULL, counter, (void*) counterID);	
	}
	/* join threads */	
	for(int i=0; i<n; i++){
		pthread_join(counters[i], NULL);	
	}
	pthread_join(monitor_th, NULL);
	pthread_join(collector_th, NULL);	
	return 0;
}
