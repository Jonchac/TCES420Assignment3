
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include<time.h>

#define CpuThreads 8
#define IOThreads 4
#define SThreads 4
#define CPU 1
#define IO 2
#define NR_PHASES 2
#define MaxJobs 20

pthread_mutex_t mutex0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
int NR_JOBS =0;
int jobID = 0;
struct job {
	int job_id;
	int nr_phases;
	int jobTime;
    int current_phase;
	// Phase types: 1 = CPU phase; 2 = IO phase
	int phasetype_and_duration[NR_PHASES][NR_PHASES];
	int is_completed;
}; 
// A linked list (LL) node to store a queue entry
struct QNode
{
    struct job *currentJob;
    struct QNode *next;
};
 
// The queue, front stores the front node of LL and rear stores ths
// last node of LL
struct Queue
{
    struct QNode *front, *rear;
	int size;
};
 
// A utility function to create a new linked list node.
struct QNode* newNode(struct job *j)
{
    struct QNode *temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->currentJob = j;
    temp->next = NULL;
    return temp; 
}
 
// A utility function to create an empty queue
struct Queue *createQueue()
{
    struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a key k to q
void enQueue(struct Queue *q,struct job* k)
{
    // Create a new LL node
    struct QNode *temp = newNode(k);
 
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
       q->front = q->rear = temp;
       return;
    }
 
    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
	q->size++;
}
 
// Function to remove a key from given queue q
struct QNode *deQueue(struct Queue *q)
{
	printf("DEQUEUE THE JOB \n");
    // If queue is empty, return NULL.
    if (q->front == NULL)
       return NULL;
 
    // Store previous front and move front one node ahead
    struct QNode *temp = q->front;
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL){
		 q->rear = NULL;
	}
    q->size--;
    return temp;
}
struct Queue* cpuQ;
struct Queue* ioQ;
struct Queue* fQ;
// Driver Program to test anove functions

void *JobCreation()
{
		printf("job init");
	while (NR_JOBS<MaxJobs){
		if (jobID < MaxJobs){
			int k;
			int i;
			pthread_mutex_lock( &mutex0 );
			struct job *j = (struct job*)malloc(sizeof(struct job));
			j->job_id = jobID;
			j->current_phase = 0;
			j->nr_phases = NR_PHASES;

			for (k = 0; k < NR_PHASES; k++){
				j->phasetype_and_duration[0][k] = (rand()%2);
				j->phasetype_and_duration[1][k] = (1);
			}
			jobID++;
			
			printf("Job CREATED %d\n", j->job_id);
			
			enQueue(cpuQ,j);
			printf("Job is ENTERING CPU QUEUE \n");
	
			if(fQ->size>0){
				printf("dequeueing off FinalQ /n");
				struct QNode *temp = deQueue(fQ);
				NR_JOBS++;
				free(temp->currentJob);
			}
			pthread_mutex_unlock( &mutex0 );
			sleep(2);
		}else {
			if (fQ->size > 0){
				printf("dequeueing off FinalQ \n");
				struct QNode *temp = deQueue(fQ);
				NR_JOBS++;
				free(temp->currentJob);
			}
		}
	}
}
void *CpuProcess(){
	printf("COU init");
	while(NR_JOBS<MaxJobs){
	if(cpuQ->size != 0){
		pthread_mutex_lock( &mutex1 );
		struct QNode* temp = deQueue(cpuQ);
		int currPhase =temp->currentJob->current_phase;
		if(currPhase == (NR_PHASES-1)){
			printf("JOB IS FINISHED");
			enQueue(fQ,temp->currentJob);
			
		}	
		else{
				int phaseType =temp->currentJob->phasetype_and_duration[0][currPhase];
				if(phaseType == 1){
					
					int waitTime = temp->currentJob->phasetype_and_duration[1][currPhase];
					printf("Waiting for JOB");
					sleep(waitTime);
					temp->currentJob->current_phase++;
					enQueue(cpuQ,temp->currentJob);
					
				}
				if(phaseType == 2){
					enQueue(ioQ,temp->currentJob);
					printf("Entering ioQueue");
				}
			
			}
			free(temp);
			pthread_mutex_unlock( &mutex1 );
		}
	}
}
void *IOProcess(){
		printf("io init");
	while (NR_JOBS<MaxJobs){
		if ( ioQ->size > 0){
			pthread_mutex_lock( &mutex2 );
			struct QNode *temp=deQueue(ioQ);

			
			int dur;
			int currPhase = temp->currentJob->current_phase;
			dur = temp->currentJob->phasetype_and_duration[1][currPhase];
			sleep(dur*.0001);
			temp->currentJob->current_phase++;
			enQueue(cpuQ,temp->currentJob);
			printf("ENTERING BACK TO CPU QUEUE");
			free(temp);
			pthread_mutex_unlock( &mutex2 );
		}
		
	}
}

int main()
{
	pthread_mutex_init (&mutex0,NULL);
	pthread_mutex_init (&mutex1,NULL);
	pthread_mutex_init (&mutex2,NULL);
	int i;
	//struct job* test = createJob();
	cpuQ = createQueue();
	ioQ = createQueue();
	fQ = createQueue();
	
	cpuQ->size = 0;
	ioQ->size = 0;
	fQ->size = 0;	

	/*enQueue(&cpuQ,test);
	deQueue(&cpuQ);
	JobCreation();*/
	//enQueue(cpuQ,test);
    pthread_t t[16];

	
    for (i = 0; i < 8; i++){
        pthread_create(&t[i], NULL, CpuProcess, NULL);
    }
	for (; i < 12; i++){
        pthread_create(&t[i], NULL, IOProcess, NULL);
    }
	for (; i < 16; i++){
        pthread_create(&t[i], NULL, JobCreation, NULL);
    }
	
    for (i = 0; i < 16; i++){
        pthread_join(t[i], NULL);
    }
	/*pthread_t CThreads[CpuThreads];
	pthread_t IOThread[IOThreads];
	pthread_t JThread[SThreads];
	for(i=0;i<CpuThreads;i++){
		pthread_create(&CThreads[i],NULL,CpuProcess,NULL);
	}
	for(i=0;i<IOThreads;i++){
		pthread_create(&IOThread[i],NULL,IOProcess,NULL);
	}
	for(i=0;i<SThreads;i++){
		pthread_create(&JThread[i],NULL,JobCreation,NULL);
	}
	for(i=0;i<CpuThreads;i++){
		pthread_join(CThreads[i],NULL);
	}
	for(i=0;i<IOThreads;i++){
		pthread_join(IOThread[i],NULL);
	}
	for(i=0;i<SThreads;i++){
		pthread_join(JThread[i],NULL);
	}
	int test = cpuQ->size;
	printf("%d",test);*/
	/*for(i=0;i<NR_JOBS;i++){
		struct QNode* temp = deQueue(cpuQ);
		printf(" test %d \n", temp->currentJob->job_id );
		
	}
	*/	
    return 0;
}
