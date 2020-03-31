#include <pthread.h>		//Create POSIX threads.
#include <time.h>			//Wait for a random time.
#include <unistd.h>			//Thread calls sleep for specified number of seconds.
#include <semaphore.h>		//To create semaphores
#include <stdlib.h>			 // to work with the dynamic memory allocation
#include <stdio.h>			// Standard Input Output

pthread_t *students;  // N threads  are which are considered as the students
pthread_t TA;         // Teacher Assistance  thread

int chaircount = 0;           // initially chair count is 0
int currentindex = 0;  

//declarating of Semaphores and Mutex Lock 

sem_t TA_sleep;                  // to check wheather the TA is sleeping or nor
sem_t Student_sem;
sem_t ChairsSem[3];            // total chairs available outside the room
pthread_mutex_t ChairAccess;
  
  //declaration of the functions 
  // divided into two sections 1. TA_ Activity   2. Student_Activity
void *TA_Activity();
void *Student_Activity(void *threadID);

int main(int argc, char* argv[])
{
	int  NumberOfStudents;
	int id;
	srand(time(NULL));
	
  // here initializing Mutex lock and Semaphores for TA and Student
	sem_init(&TA_sleep, 0,0);
	sem_init(&Student_sem,0,0);
	for(id=0;id<3;++id)
	 	sem_init(&ChairsSem[id],0,0);
	
	pthread_mutex_init(&ChairAccess,NULL);
	
	if(argc<2)
	{
		printf("NUmber of Students not specified. considering default as (5)Students\n");
		NumberOfStudents= 5;
	}
	else
	{
		printf("NUmber of Students not specified. creating %d threads .\n",NumberOfStudents);
		NumberOfStudents= atoi(argv[1]);
	}
	
	
	//allocating memory dynamically using malloc()-->which stands for memory allocation
	students =(pthread_t*)malloc(sizeof(pthread_t)*NumberOfStudents);
	
	//creating TA thread and N Students threads.
	pthread_create(&TA,NULL,TA_Activity,NULL);
	for(id=0; id< NumberOfStudents;id++)
		pthread_create(&students[id],NULL,Student_Activity,(void*) (long)id);
		
	
	// waiting for TA and N Students threads.
	pthread_join(TA,NULL);
	for(id=0;id<NumberOfStudents;id++)
		pthread_join(students[id],NULL);
		
	
	// Free the memory allocated for the Students
	free(students);
	return 0;
	
}
		
void *TA_Activity()
{
		
	while(1)
	{
		sem_wait(&TA_sleep);             //TA is currenty sleeping
		printf("---------------------------->>>TA has been awakened by the Student <<<------------------\n");
		
		while(1)
		{
			///lock
			pthread_mutex_lock(&ChairAccess);
			if(chaircount == 0)
			{
				pthread_mutex_unlock(&ChairAccess);   // if chairs are empty then break the loop.
				break;
			}
			
			
			// this code is for      -- TA gets next student on chair
			sem_post(&ChairsSem[currentindex]);
			chaircount--;
			printf("Student left his/her chair. REmaining chairs %d \n",3-chaircount);
			currentindex = (currentindex+1)%3;
			pthread_mutex_unlock(&ChairAccess);
			
			
			printf("\t TA is currently helping the student. \n");
			sleep(5);
			sem_post(&Student_sem);
			usleep(2000);
			
		}
	}
}	 	

	
void  *Student_Activity(void *threadID)
{
	int ProgrammingTime;
	
	while(1)
	{
		printf("Student %ld is doing programming assignment.\n", (long)threadID);
		ProgrammingTime = rand() % 10 + 1;
		sleep(ProgrammingTime);		//Sleep for a random time period.

		printf("Student %ld needs help from the TA\n", (long)threadID);
		
		pthread_mutex_lock(&ChairAccess);
		int count = chaircount;
		pthread_mutex_unlock(&ChairAccess);

		if(count < 3)		//Student tried to sit on a chair.
		{
			if(count == 0)		//If student sits on first empty chair, wake up the TA.
				sem_post(&TA_sleep);
			else
				printf("Student %ld sat on a chair waiting for the TA to finish. \n", (long)threadID);

			// lock
			pthread_mutex_lock(&ChairAccess);
			int index = (currentindex + chaircount) % 3;
			chaircount++;
			printf("Student sat on chair.Chairs Remaining: %d\n", 3 - chaircount);
			pthread_mutex_unlock(&ChairAccess);
			// unlock

			sem_wait(&ChairsSem[index]);		//Student leaves his/her chair.
			printf("\t Student %ld is getting help from the TA. \n", (long)threadID);
			sem_wait(&Student_sem);		//Student waits to go next.
			printf("Student %ld left TA room.\n",(long)threadID);
		}
		else 
			printf("Student %ld will return at another time. \n", (long)threadID);
			//If student didn't find any chair to sit on.
	}
	
}










			

	
