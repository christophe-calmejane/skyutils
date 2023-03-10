/****************************************************************/
/* Threads unit                                                 */
/* (c) Christophe CALMEJANE (Ze KiLleR) - 1999-2014             */
/****************************************************************/

/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include <skyutils/skyutils.h>

#ifdef _WIN32
#pragma warning( disable: 4100)
#endif /* _WIN32 */

#ifndef SU_TRACE_INTERNAL
#ifdef SU_MALLOC_TRACE
#undef malloc
#undef calloc
#undef realloc
#undef strdup
#undef free
#endif /* SU_MALLOC_TRACE */
#endif /* !SU_TRACE_INTERNAL */

SKYUTILS_API bool SU_CreateThread(SU_THREAD_HANDLE *Handle,SU_THREAD_ID *ThreadId,SU_THREAD_ROUTINE_TYPE(Entry),void *User,bool Detached)
{
#ifdef _WIN32
  *Handle = (SU_THREAD_HANDLE)_beginthreadex(NULL,0,Entry,User,0,ThreadId);
  return ((*Handle) != 0); /* _beginthreadex returns 0 on error, while _beginthread returns -1 */
#else /* !_WIN32 */
  if(pthread_create(Handle,NULL,Entry,User) != 0)
    return false;
  *ThreadId = *Handle;
  if(Detached)
    pthread_detach(*Handle);
  return true;
#endif /* _WIN32 */
}

SKYUTILS_API bool SU_SetThreadPriority(SU_THREAD_HANDLE Handle,int Priority)
{
#ifdef _WIN32
  return SetThreadPriority(Handle,Priority) != 0;
#else /* !_WIN32 */
  int policy;
  struct sched_param sp;
  if(pthread_getschedparam(Handle,&policy,&sp) == 0) // Get current policy
  {
    sp.sched_priority = Priority;
    if(pthread_setschedparam(Handle,policy,&sp) == 0) // Modify priority, keeping current policy
    {
      if(pthread_getschedparam(Handle,&policy,&sp) == 0) // Check if priority was correctly changed (tempo check)
      {
        if(sp.sched_priority == Priority)
        return true;
      }
    }
  }
  return false;
#endif /* _WIN32 */
}

SKYUTILS_API void SU_KillThread(SU_THREAD_HANDLE Handle)
{
#ifdef _WIN32
  TerminateThread((void *)Handle,0);
#else /* !_WIN32 */
  pthread_kill(Handle,SIGKILL);
#endif /* _WIN32 */
}

SKYUTILS_API void SU_TermThread(SU_THREAD_HANDLE Handle)
{
#ifdef _WIN32
  TerminateThread((void *)Handle,0);
#else /* !_WIN32 */
  pthread_kill(Handle,SIGTERM);
#endif /* _WIN32 */
}

SKYUTILS_API void SU_SuspendThread(SU_THREAD_HANDLE Handle)
{
#ifdef _WIN32
  SuspendThread((void *)Handle);
#else /* !_WIN32 */
  pthread_kill(Handle,SIGSTOP);
#endif /* _WIN32 */
}

SKYUTILS_API void SU_ResumeThread(SU_THREAD_HANDLE Handle)
{
#ifdef _WIN32
  ResumeThread((void *)Handle);
#else /* !_WIN32 */
  pthread_kill(Handle,SIGCONT);
#endif /* _WIN32 */
}

SKYUTILS_API void *SU_WaitForThread(SU_THREAD_HANDLE Handle)
{
  void *retval = 0;
#ifdef _WIN32
  WaitForSingleObject(Handle,INFINITE);
  GetExitCodeThread(Handle,(LPDWORD)&retval);
#else /* !_WIN32 */
  pthread_join(Handle,&retval);
#endif /* _WIN32 */
  return retval;
}

SKYUTILS_API bool SU_CreateThreadKey(SU_THREAD_KEY_HANDLE *Handle,SU_THREAD_ONCE_HANDLE *Once,void (*destroyts)(void *))
{
#ifdef _WIN32
  if(*Once == SU_THREAD_ONCE_INIT)
  {
    *Handle = TlsAlloc();
    (*Once)++;
  }
  return ((*Handle) != 0xFFFFFFFF);
#else /* !_WIN32 */
  return  pthread_key_create(Handle,destroyts) == 0;
#endif /* _WIN32 */
}

SKYUTILS_API bool SU_CreateSem(SU_SEM_HANDLE *Handle,int InitialCount,int MaximumCount,const char SemName[])
{
#ifdef _WIN32
  *Handle = CreateSemaphore(NULL,InitialCount,MaximumCount,SemName);
  return ((*Handle) != NULL);
#else /* !_WIN32 */
  return sem_init(Handle,0,InitialCount) == 0;
#endif /* _WIN32 */
}

SKYUTILS_API bool SU_FreeSem(SU_SEM_HANDLE *Handle)
{
#ifdef _WIN32
  return CloseHandle(*Handle);
#else /* !_WIN32 */
  return sem_destroy(Handle) == 0;
#endif /* _WIN32 */
}

SKYUTILS_API void SU_ThreadBlockSigs(void)
{
#ifdef __unix__
  sigset_t mask;

  sigfillset(&mask);
  pthread_sigmask(SIG_BLOCK,&mask,NULL);
#endif /* __unix__ */
}

/* Try to lock a semaphore - Returns 0 if sem was available and has been taken, -1 otherwise (if it was already taken) */
SKYUTILS_API int SU_SemTryWait(SU_SEM_HANDLE *sem)
{
#ifdef _WIN32
  if(WaitForSingleObject(*sem,0) == WAIT_OBJECT_0)
    return 0; /* Sem has been taken */
  return -1;
#else /* !_WIN32 */
  while(sem_trywait(sem) == -1)
  {
    if(errno != EINTR) /* Only return from function if not EINTR */
      return -1;
  }
  return 0;
#endif /* _WIN32 */
}

/* Wait for a semaphore for a maximum of 'msec' milliseconds (a negative value will never timeout) - Returns 0 if sem was available and has been taken, -1 if timed out */
SKYUTILS_API int SU_SemWaitTimeout(SU_SEM_HANDLE *sem,int msec)
{
#ifdef _WIN32
	if(msec < 0)
	{
		WaitForSingleObject(*sem,INFINITE);
		return 0;
	}
  if(WaitForSingleObject(*sem,msec) == WAIT_OBJECT_0)
    return 0; /* Sem has been taken */
  return -1;
#else /* !_WIN32 */
	if(msec < 0)
	{
		 while(sem_wait(sem) == -1)
		 {
			 if(errno != EINTR)
				 break; // It's an error, we should handle this case
		 }
	}
	else
	{
		struct timeval now;
		struct timespec to;
		gettimeofday(&now,NULL);
		// We stay in usec to prevent going over the size of an int on a 32bit computer
		now.tv_usec += (msec * 1000);
		if(now.tv_usec > 1000000)
		{
			now.tv_sec += now.tv_usec / 1000000;
			now.tv_usec = now.tv_usec % 1000000;
		}
		to.tv_sec = now.tv_sec;
		to.tv_nsec = now.tv_usec * 1000;
		while(sem_timedwait(sem,&to) == -1)
		{
			if(errno != EINTR) /* Only return from function if not EINTR */
				return -1;
		}
	}
  return 0;
#endif /* _WIN32 */
}

/* Create a new mutex */
SKYUTILS_API bool SU_CreateMutex(SU_MUTEX_HANDLE *Handle,const char MutexName[])
{
#ifdef _WIN32
  *Handle = CreateMutex(NULL,false,MutexName);
  return ((*Handle) != NULL);
#else /* !_WIN32 */
  return pthread_mutex_init(Handle,NULL) == 0;
#endif /* _WIN32 */
}
/* Free a mutex */
SKYUTILS_API bool SU_FreeMutex(SU_MUTEX_HANDLE *Handle)
{
#ifdef _WIN32
  return CloseHandle(*Handle);
#else /* !_WIN32 */
  return pthread_mutex_destroy(Handle) == 0;
#endif /* _WIN32 */
}

/* Create a critical section */
SKYUTILS_API bool SU_CriticalInit(SU_CRITICAL *Crit,int Type) /* True on success */
{
#ifdef _WIN32
  if(Type == SU_CRITICAL_TYPE_NON_RECURSIVE)
    return false;
  InitializeCriticalSection(Crit);
  return true;
#else /* !_WIN32 */
#ifdef __linux__
  if(Type == SU_CRITICAL_TYPE_RECURSIVE)
  {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);
    return pthread_mutex_init(Crit,&attr) == 0;
  }
  else
    return pthread_mutex_init(Crit,NULL) == 0;
#else /* !__linux__ */
  if(Type != SU_CRITICAL_TYPE_ANY)
    return false;
  return pthread_mutex_init(Crit,NULL) == 0;
#endif /* __linux__ */
#endif /* _WIN32 */
}
/* Free a critical section */
SKYUTILS_API bool SU_CriticalDelete(SU_CRITICAL *Crit)
{
#ifdef _WIN32
  DeleteCriticalSection(Crit);
  return true;
#else /* !_WIN32 */
  return pthread_mutex_destroy(Crit) == 0;
#endif /* _WIN32 */
}
