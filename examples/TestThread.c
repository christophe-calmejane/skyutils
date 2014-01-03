/* Compilation command line
    cl TestThread.c -I../include -c
    link TestThread.obj ../src/windows/skyutils/debug/skyutils.lib /nodefaultlib:libcmtd
*/

#include <skyutils/skyutils.h>

static double startTime;
static const SU_THREAD_RET_TYPE _thread_ret_zero = 0;

SU_THREAD_HANDLE hThread1 = SU_THREAD_NULL;
SU_THREAD_ID ThreadId1;
SU_THREAD_HANDLE hThread2 = SU_THREAD_NULL;
SU_THREAD_ID ThreadId2;

SU_SEM_HANDLE sem1;

#define MSG(str) printf("[%.3f:%s] %s\n",(SU_GetTimeAsMilli()-startTime)/1000.0f,myName,str)
#define FATAL(str) { printf("SHOULD NEVER HAPPEN: %s\n",str); exit(-1); }

SU_THREAD_ROUTINE(thread1,info)
{
	char myName[] = "Thrd1";

	MSG("Entering thread\n");

	MSG("Wait for sem1 (2)\n");
	SU_SEM_WAIT(sem1);
	MSG("Got sem1 (2)\n");

	MSG("Release sem1 (2)\n");
	SU_SEM_POST(sem1);

	MSG("Terminating thread\n");
	SU_THREAD_RETURN(_thread_ret_zero);
}

SU_THREAD_ROUTINE(thread2,info)
{
	char myName[] = "Thrd2";

	MSG("Entering thread\n");

	MSG("Try to lock sem1 with 1500msec timeout (7)\n");
	if(SU_SEM_WAIT_TIMEOUT(sem1,1500))
	{
		FATAL("Got sem1 (7)\n");
	}
	else
	{
		MSG("Timed out trying to get sem1 (7)\n");
	}

	MSG("Try to lock sem1 with 800msec timeout (8)\n");
	if(SU_SEM_WAIT_TIMEOUT(sem1,800))
	{
		MSG("Got sem1 (8)\n");
	}
	else
	{
		FATAL("Timed out trying to get sem1 (8)\n");
	}

	MSG("Terminating thread\n");
	SU_THREAD_RETURN(_thread_ret_zero);
}

int main(int argc, char *argv[])
{
	char myName[] = "Main";
	startTime = SU_GetTimeAsMilli();

	MSG("Creating sem1\n");
	SU_CreateSem(&sem1,1,1,NULL);

	MSG("Wait for sem1 (1)\n");
	SU_SEM_WAIT(sem1);
	MSG("Got sem1 (1)\n");

	MSG("Creating thread1\n");
	SU_CreateThread(&hThread1,&ThreadId1,thread1,NULL,true);

	MSG("sleep 1sec\n");
	SU_SLEEP(1);

	MSG("Release sem1 (1)\n");
	SU_SEM_POST(sem1);

	MSG("sleep 1sec\n");
	SU_SLEEP(1);

	MSG("Try to lock sem1 (3)\n");
	if(SU_SEM_TRY_AND_ENTER(sem1))
	{
		MSG("Got sem1 (3)\n");
	}
	else
	{
		FATAL("sem1 was already locked (3)\n");
	}

	MSG("Try to lock sem1 (4)\n");
	if(SU_SEM_TRY_AND_ENTER(sem1))
	{
		FATAL("Got sem1 (4)\n");
	}
	else
	{
		MSG("sem1 was already locked (4)\n");
	}

	MSG("Try to lock sem1 with 300msec timeout (5)\n");
	if(SU_SEM_WAIT_TIMEOUT(sem1,300))
	{
		FATAL("Got sem1 (5)\n");
	}
	else
	{
		MSG("Timed out trying to get sem1 (5)\n");
	}

	MSG("Release sem1 (3)\n");
	SU_SEM_POST(sem1);

	MSG("Try to lock sem1 with 500msec timeout (6)\n");
	if(SU_SEM_WAIT_TIMEOUT(sem1,500))
	{
		MSG("Got sem1 (6)\n");
	}
	else
	{
		FATAL("Timed out trying to get sem1 (6)\n");
	}

	MSG("Creating thread2\n");
	SU_CreateThread(&hThread2,&ThreadId2,thread2,NULL,true);

	MSG("sleep 2sec\n");
	SU_SLEEP(2);

	MSG("Release sem1 (6)\n");
	SU_SEM_POST(sem1);

	MSG("sleep 1sec\n");
	SU_SLEEP(1);

	printf("TEST SUCCESSFULL!!\n");

	return 0;
}
