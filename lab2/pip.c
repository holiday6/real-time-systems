#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include <unistd.h>
void *do_one_thing(void *);
void *do_another_thing(void *);
void do_wrap_up(int, int);
void *do_third_thing(void *);

pthread_mutex_t mutex_section1 = PTHREAD_MUTEX_INITIALIZER;

int r1 = 0, r2 = 0, r3 = 0;
void workload_1ms (void)
{
	int repeat = 35000; // tune this for the right amount of workload
	for (int i = 0; i <= repeat; i++)
	{
		sqrt(sqrt(i));                
		// add some computation here (e.g., use sqrt() in cmath)
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
	extern int
main(void)
{
	// similarly, the PIP is applied to the mutex
    pthread_mutexattr_t mutexattr_prioinherit;
    int mutex_protocol;
    pthread_mutexattr_init (&mutexattr_prioinherit);
    pthread_mutexattr_getprotocol (&mutexattr_prioinherit,
                                   &mutex_protocol);
    if (mutex_protocol != PTHREAD_PRIO_INHERIT)
    {
        pthread_mutexattr_setprotocol (&mutexattr_prioinherit,
                                       PTHREAD_PRIO_INHERIT);
    }
    pthread_mutex_init (&mutex_section1, &mutexattr_prioinherit);
	pinCPU(0);
	pthread_t thread1, thread2, thread3;

	pthread_t *pool = (pthread_t*)malloc(10*sizeof(pthread_t));
	
	for(int i=0;i<7;i++)
	{
		if (pthread_create(&pool[i], 
				NULL,
				do_another_thing,
				(void *) &r1) != 0)
		perror("pthread_create"), exit(1);
	}
	if (pthread_create(&thread1, 
				NULL,
				do_one_thing,
				(void *) &r1) != 0)
		perror("pthread_create"), exit(1); 

	if (pthread_create(&thread2, 
				NULL, 
				do_another_thing,
				(void *) &r2) != 0)
		perror("pthread_create"), exit(1); 

	if (pthread_create(&thread3, 
				NULL, 
				do_third_thing,
				(void *) &r3) != 0)
		perror("pthread_create"), exit(1); 
	
	for(int i=0;i<7;i++)
	{
		if (pthread_join(pool[i], NULL) != 0)
			perror("pthread_join"),exit(1);
	}

	if (pthread_join(thread1, NULL) != 0)
		perror("pthread_join"),exit(1);

	if (pthread_join(thread2, NULL) != 0)
		perror("pthread_join"),exit(1);

	if (pthread_join(thread3, NULL) != 0)
		perror("pthread_join"),exit(1);



	return 0; 
}

void *do_one_thing(void *pnum_times)
{
	pinCPU(1);
	setSchedulingPolicy(SCHED_FIFO,99);
	int i, j, x;
	
	int period = 50000; //microseconds
	int delta;
	while(1) {
		//printf("doing one thing\n"); 
		std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();

		pthread_mutex_lock (&mutex_section1);
		for(j=0;j<1;j++)
		{
		workload_1ms();
		}
		pthread_mutex_unlock (&mutex_section1);
		std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
		delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		//if(delta>5000) 
			printf("99 priority pthread response time: %d\n",delta);
		if (delta > period)
		{
			continue;
		}
		else
		{
			usleep (period-delta);
		}
		(*(int *)pnum_times)++;
	}
	return(NULL);
}

void *do_another_thing(void *pnum_times)
{
	pinCPU(1);
	setSchedulingPolicy(SCHED_FIFO,98);
	int i, j, x;
	
	int period = 100000; //microseconds
	int delta;
	while(1) {
		//printf("doing another thing\n");
		std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
		for(int i=0;i<10;i++)
		{
			workload_1ms();
		}
		std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
		delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		//printf("98 priority pthread response time: %d\n",delta);

		if (delta > period)
		{
			continue;
		}
		else
		{
			usleep (period-delta);
		}
		(*(int *)pnum_times)++;
	}
	return(NULL);
}

void *do_third_thing(void *pnum_times)
{
	pinCPU(1);
	setSchedulingPolicy(SCHED_FIFO,97);
	int i, j, x;
	
	int period = 200000; //microseconds
	int delta;
	while(1)
	{
		//printf("doing another \n"); 
		std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
		workload_1ms();
		pthread_mutex_lock (&mutex_section1);
		for(int i=0;i<8;i++)
		{
			workload_1ms();
		}	
		pthread_mutex_unlock (&mutex_section1);
		workload_1ms();
		
		std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
		delta = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
		//printf("97 priority pthread response time: %d\n",delta);

		if (delta > period)
		{
			continue;
		}
		else
		{
			usleep (period-delta);
		}
		(*(int *)pnum_times)++;
	}
	return(NULL);
}
