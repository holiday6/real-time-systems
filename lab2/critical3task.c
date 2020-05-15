#include <sched.h>
#include <stdio.h>

#include <iostream>
#include <chrono>
#include <cmath>
#include <unistd.h>

void setSchedulingPolicy(int policy,int priority);
void workload_1ms(void);
void *hightask(void* n);
void *middletask(void* n);
void *lowtask(void* n);
void pinCPU(int c);

int t1=0,t2=0,t3=0;

extern int
main(void)
{
	pinCPU(0);
	pthread_t tau1,tau2,tau3,tau4;
	
	if(pthread_create(&tau1,NULL,hightask,(void*)&t1)!=0)
		perror("pthread_create"), exit(1); 
	if(pthread_create(&tau2,NULL,middletask,(void*)&t2)!=0)
		perror("pthread_create"), exit(1); 
	if(pthread_create(&tau3,NULL,lowtask,(void*)&t3)!=0)
		perror("pthread_create"), exit(1); 

	
	if(pthread_join(tau1,NULL)!=0)
		perror("pthread_join"), exit(1);
	if(pthread_join(tau2,NULL)!=0)
		perror("pthread_join"), exit(1);
	if(pthread_join(tau3,NULL)!=0)
		perror("pthread_join"), exit(1);


	return(0);
}

void *hightask(void * n)
{
	pinCPU(1);
       	setSchedulingPolicy(SCHED_FIFO,99); 
	pthread_mutex_t mutex_section1 = PTHREAD_MUTEX_INITIALIZER;
	int period = 50000; //microseconds
	int delta;
	while(1)
    {
        std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
        
	pthread_mutex_lock (&mutex_section1);
	workload_1ms();
        pthread_mutex_unlock (&mutex_section1);
        std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
        delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        printf("99 priority pthread response time: %d\n",delta);
	if (delta > period)
        {
            continue;
        }
        else
        {
            usleep (period-delta);
        }
    }

}
void *middletask(void * n)
{
	pinCPU(1);
	setSchedulingPolicy(SCHED_FIFO,98); 
	pthread_mutex_t mutex_section1 = PTHREAD_MUTEX_INITIALIZER;
	int period = 100000; //microseconds
	int delta;
	while (1)
    {
        std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
        for(int i=0;i<80;i++)
	{
		workload_1ms();
	}
	std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
        delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
         printf("98 priority pthread response time: %d\n",delta);
       
	if (delta > period)
        {
            continue;
        }
        else
        {
            usleep (period-delta);
        }
    }
}
void *lowtask(void * n)
{
	pinCPU(1);
	setSchedulingPolicy(SCHED_FIFO,97);
	pthread_mutex_t mutex_section1 = PTHREAD_MUTEX_INITIALIZER;
	int period = 200000; //microseconds
	int delta;
	while (1)
    {
        std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
 	for(int i=0;i<60;i++)
	{
		workload_1ms();
	}	
	pthread_mutex_lock (&mutex_section1);
	workload_1ms();
        pthread_mutex_unlock (&mutex_section1);
        std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
        delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
         printf("97 priority pthread response time: %d\n",delta);
       
	if (delta > period)
        {
            continue;
        }
        else
        {
            usleep (period-delta);
        }
    }
}
void pinCPU (int cpu_number)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);

    CPU_SET(cpu_number, &mask);

    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask) == -1)
    {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }
}

void setSchedulingPolicy (int newPolicy, int priority)
{
    sched_param sched;
    int oldPolicy;
    if (pthread_getschedparam(pthread_self(), &oldPolicy, &sched)) {
        perror("pthread_setschedparam");
        exit(EXIT_FAILURE);
    }
    sched.sched_priority = priority;
    if (pthread_setschedparam(pthread_self(), newPolicy, &sched)) {
        perror("pthread_setschedparam");
        exit(EXIT_FAILURE);
    }
}
void workload_1ms (void)
{
        int repeat = 65000; // tune this for the right amount of workload
        for (int i = 0; i <= repeat; i++)
        {
		sqrt(i);                
		// add some computation here (e.g., use sqrt() in cmath)
        }
}
