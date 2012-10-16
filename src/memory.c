/****************************************************************/
/* Memory unit                                                  */
/* (c) Christophe CALMEJANE (Ze KiLleR) - 1999-2011             */
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


#include "skyutils.h"
#include <time.h>
#include <stdarg.h>

#ifdef _WIN32
#pragma warning( disable: 4100)
#endif /* _WIN32 */

#define SU_DEFAULT_MALLOC_CHECK 2
#define SU_MALLOC_KEY 0x5c
#define SU_MALLOC_KEY2 0xa7
#define SU_MALLOC_BOUND_VALUE 0x66AA55CC
#define SU_MALLOC_REUSE_VALUE 0x11CC77BB
#define SU_MALLOC_REUSE_SIZE 64

#ifdef _REENTRANT
SU_CRITICAL SU_alloc_trace_sem; /* Semaphore to protect the use of SU_alloc_trace_list in MT environment */
#endif /* _REENTRANT */
bool SU_sem_init=false;
int SU_env_check = SU_DEFAULT_MALLOC_CHECK;
int SU_env_trace = 0;
int SU_env_print = 0;
size_t SU_total_memory_allocated = 0;

SU_PList SU_alloc_trace_list = NULL; /* SU_PAlloc */

typedef struct
{
  void *ptr;
  size_t size;
  time_t time;
  char file[512];
  SU_u32 line;
  bool freed;
} SU_TAlloc, *SU_PAlloc;

#undef malloc
#undef calloc
#undef realloc
#undef strdup
#undef free

SU_PList SU_AddElementHead_no_trace(SU_PList List,void *Elem)
{
  SU_PList El;

  El = (SU_PList) malloc(sizeof(SU_TList));
  El->Next = List;
  El->Data = Elem;
  return El;
}

SU_PList SU_DelElementHead_no_trace(SU_PList List)
{
  SU_PList Ptr;

  if(List == NULL)
    return NULL;
  Ptr = List->Next;
  free(List);
  return Ptr;
}

static void SU_DefaultPrintFunc(bool Fatal,char *Txt,...)
{
  va_list argptr;
  char Str[8192];
  va_start(argptr,Txt);
#ifdef _WIN32
  _vsnprintf(Str,sizeof(Str),Txt,argptr);
#else /* !_WIN32 */
  vsnprintf(Str,sizeof(Str),Txt,argptr);
#endif /* _WIN32 */
  va_end(argptr);
  printf("%s\n",Str);
}

static SU_PRINT_FUNC *SU_PrintFunc = SU_DefaultPrintFunc;
void SU_SetPrintFunc(SU_PRINT_FUNC *Func)
{
  SU_PrintFunc = Func;
}

/* MEMORY ALIGNEMENT FUNCTIONS */
void *SU_malloc(size_t size)
{
  unsigned char pad;
  void *memblock,*retblock;

  memblock = malloc((size_t)(size+2*SU_MALLOC_ALIGN_SIZE));
  if(memblock == NULL)
  {
    SU_PrintFunc(true,"SkyUtils_SU_malloc Warning : malloc returned NULL");
    return NULL;
  }
  pad = (unsigned char)(((size_t)memblock)%SU_MALLOC_ALIGN_SIZE);
  if(pad == 0)
    pad = SU_MALLOC_ALIGN_SIZE;
  if(pad < (2+sizeof(size_t)))
    pad += SU_MALLOC_ALIGN_SIZE;
  retblock = (unsigned char *)memblock+pad;
  *((unsigned char *)retblock-1) = pad;
  *((unsigned char *)retblock-2) = SU_MALLOC_KEY;
  *(size_t *)((unsigned char *)retblock-(2+sizeof(size_t))) = size;
  return retblock;
}

void *SU_calloc(size_t nbelem,size_t size)
{
  void *ptr;

  ptr = SU_malloc(nbelem*size);
  if(ptr == NULL)
    return NULL;
  memset(ptr,0,nbelem*size);
  return ptr;
}

void *SU_realloc(void *memblock,size_t size)
{
  char *ptr;
  size_t oldsize;

  if(memblock == NULL) /* If memblock is NULL -> malloc */
    return SU_malloc(size);

  if(*((unsigned char *)memblock-2) == SU_MALLOC_KEY2)
  {
    SU_PrintFunc(true,"SkyUtils_SU_realloc Warning : bloc already freed");
    return NULL;
  }
  else if(*((unsigned char *)memblock-2) != SU_MALLOC_KEY)
  {
    SU_PrintFunc(true,"SkyUtils_SU_realloc Warning : bloc might have been underwritten");
    return NULL;
  }
  oldsize = *(size_t *)((unsigned char *)memblock-(2+sizeof(size_t)));
  ptr = SU_malloc(size);
  memcpy(ptr,memblock,(size_t)oldsize);
  SU_free(memblock);
  return ptr;
}

char *SU_strdup_memory(const char *in)
{
  char *s;
  size_t len;

  len = strlen(in) + 1;
  s = (char *) SU_malloc(len);
  if(s == NULL)
    return NULL;
  SU_strcpy(s,in,len);
  return s;
}

void SU_free(void *memblock)
{
  unsigned char pad;
  if(*((unsigned char *)memblock-2) == SU_MALLOC_KEY2)
  {
    SU_PrintFunc(true,"SkyUtils_SU_free Warning : bloc already freed");
    return;
  }
  else if(*((unsigned char *)memblock-2) != SU_MALLOC_KEY)
  {
    SU_PrintFunc(true,"SkyUtils_SU_free Warning : bloc might have been underwritten");
    return;
  }
  *((unsigned char *)memblock-2) = SU_MALLOC_KEY2;
  pad = *((unsigned char *)memblock-1);
  free((unsigned char *)memblock-pad);
}

/* TRACE DEBUG FUNCTIONS */
void SU_printf_trace_debug(char *func,char *Str,void *memblock,char *file,SU_u32 line,char *file2,SU_u32 line2)
{
  if(SU_env_check > 0)
  {
    if(file2 == NULL)
      SU_PrintFunc(true,"SkyUtils_%s Warning : bloc %p %s (%s:%d)",func,memblock,Str,file,line);
    else
      SU_PrintFunc(true,"SkyUtils_%s Warning : bloc %p %s %s:%d (%s:%d)",func,memblock,Str,file,line,file2,line2);
  }
  if(SU_env_check == 2)
    abort();
}

void SU_malloc_CheckInit(bool readenv)
{
  char *s;

  if(!SU_sem_init)
  {
#ifdef _REENTRANT
    if(!SU_CriticalInit(&SU_alloc_trace_sem,SU_CRITICAL_TYPE_ANY))
      SU_PrintFunc(true,"SkyUtils_SU_malloc_trace Warning : Couldn't init SU_CRITICAL");
#endif /* _REENTRANT */
    SU_sem_init = true;
    if(readenv)
    {
      s = getenv("MALLOC_CHECK_");
      SU_env_check = (s==NULL)?SU_DEFAULT_MALLOC_CHECK:atoi(s);
      s = getenv("SU_MALLOC_TRACE");
      SU_env_trace = (s==NULL)?0:atoi(s);
      s = getenv("SU_MALLOC_PRINT");
      SU_env_print = (s==NULL)?0:atoi(s);
      SU_PrintFunc(false,"SkyUtils Information : Using SU_MALLOC_TRACE hooks : MALLOC_CHECK_=%d SU_MALLOC_TRACE=%d SU_MALLOC_PRINT=%d",SU_env_check,SU_env_trace,SU_env_print);
    }
  }
}

void SU_SetMallocConfig(int check,int trace,int print)
{
  SU_malloc_CheckInit(false);
  SU_env_check = check;
  SU_env_trace = trace;
  SU_env_print = print;
  SU_PrintFunc(false,"SkyUtils Information : Using SU_MALLOC_TRACE hooks : MALLOC_CHECK_=%d SU_MALLOC_TRACE=%d SU_MALLOC_PRINT=%d",SU_env_check,SU_env_trace,SU_env_print);
}

void SU_GetMallocConfig(int *check,int *trace,int *print)
{
  if(check)
    *check = SU_env_check;
  if(trace)
    *trace = SU_env_trace;
  if(print)
    *print = SU_env_print;
}

void *SU_malloc_trace(size_t size,char *file,SU_u32 line)
{
  SU_PAlloc Al = NULL;
  void *ptr;
  SU_PList Ptr;

  SU_malloc_CheckInit(true);
  ptr = malloc((size_t)(size+16)); /* 8 before, 8 after */
  if(ptr == NULL)
  {
    SU_PrintFunc(true,"SkyUtils_SU_malloc_trace Warning : malloc returned NULL");
    return NULL;
  }
  SU_total_memory_allocated += size;
  *((SU_u32 *)((char *)ptr+4)) = SU_MALLOC_BOUND_VALUE;
  *((SU_u32 *)((char *)ptr+size+8)) = SU_MALLOC_BOUND_VALUE;
#ifdef _REENTRANT
  SU_CRITICAL_ENTER(SU_alloc_trace_sem);
#endif /* _REENTRANT */
  Ptr = SU_alloc_trace_list;
  while(Ptr != NULL)
  {
    Al = (SU_PAlloc)Ptr->Data;
    if(Al->ptr == ptr)
      break;
    Ptr = Ptr->Next;
  }
  if(Ptr == NULL)
  {
    Al = (SU_PAlloc) malloc(sizeof(SU_TAlloc));
    if(Al == NULL)
    {
      free(ptr);
      return NULL;
    }
    SU_alloc_trace_list = SU_AddElementHead_no_trace(SU_alloc_trace_list,Al);
  }
  Al->ptr = ptr;
  Al->size = size;
  Al->time = time(NULL);
  SU_strcpy(Al->file,file,sizeof(Al->file));
  Al->line = line;
  Al->freed = false;
  if(SU_env_print)
    SU_PrintFunc(false,"SU_malloc_trace Information : Allocating bloc %p (%ld bytes) in pid %d (%s:%d)",(char *)ptr+8,size,getpid(),file,line);
#ifdef _REENTRANT
  SU_CRITICAL_LEAVE(SU_alloc_trace_sem);
#endif /* _REENTRANT */
  return (void *)((char *)ptr+8);
}

void *SU_calloc_trace(size_t nbelem,size_t size,char *file,SU_u32 line)
{
  void *ptr;

  ptr = SU_malloc_trace(nbelem*size,file,line);
  if(ptr == NULL)
    return NULL;
  memset(ptr,0,nbelem*size);
  return ptr;
}

void *SU_realloc_trace(void *memblock,size_t size,char *file,SU_u32 line)
{
  SU_PList Ptr;
  void *new_ptr;

  if(memblock == NULL) /* If memblock is NULL -> malloc */
    return SU_malloc_trace(size,file,line);

  SU_malloc_CheckInit(true);
#ifdef _REENTRANT
  SU_CRITICAL_ENTER(SU_alloc_trace_sem);
#endif /* _REENTRANT */
  Ptr = SU_alloc_trace_list;
  while(Ptr != NULL)
  {
    if(((SU_PAlloc)Ptr->Data)->ptr == (void *)((char *)memblock-8))
      break;
    Ptr = Ptr->Next;
  }
#ifdef _REENTRANT
  SU_CRITICAL_LEAVE(SU_alloc_trace_sem);
#endif /* _REENTRANT */
  if(Ptr == NULL)
  {
    SU_printf_trace_debug("SU_realloc_trace","already freed, or never allocated",memblock,file,line,NULL,0);
    return NULL;
  }
  if(((SU_PAlloc)Ptr->Data)->freed)
  {
    SU_printf_trace_debug("SU_realloc_trace","was freed at",memblock,((SU_PAlloc)Ptr->Data)->file,((SU_PAlloc)Ptr->Data)->line,file,line);
    return NULL;
  }
  if(size == 0) /* If size is 0 -> free */
  {
    SU_free_trace(memblock,file,line);
    return NULL;
  }
  if(size > ((SU_PAlloc)Ptr->Data)->size)
  {
    new_ptr = SU_malloc_trace(size,file,line);
    if(new_ptr != NULL)
    {
      memcpy(new_ptr,memblock,((SU_PAlloc)Ptr->Data)->size);
      SU_free_trace(memblock,file,line);
    }
    return new_ptr;
  }
  else
  {
    SU_strcpy(((SU_PAlloc)Ptr->Data)->file,file,sizeof(((SU_PAlloc)Ptr->Data)->file));
    ((SU_PAlloc)Ptr->Data)->line = line;
    /* Adjust size */
    SU_total_memory_allocated -= ((SU_PAlloc)Ptr->Data)->size;
    ((SU_PAlloc)Ptr->Data)->size = size;
    SU_total_memory_allocated += ((SU_PAlloc)Ptr->Data)->size;
    *((SU_u32 *)((char *)(((SU_PAlloc)Ptr->Data)->ptr)+size+8)) = SU_MALLOC_BOUND_VALUE;
    return memblock;
  }
}

char *SU_strdup_trace(const char *in,char *file,SU_u32 line)
{
  char *s;
  size_t len;

  len = strlen(in) + 1;
  s = (char *) SU_malloc_trace(len,file,line);
  if(s == NULL)
    return NULL;
  SU_strcpy(s,in,len);
  return s;
}

void SU_free_trace(void *memblock,char *file,SU_u32 line)
{
  SU_PList Ptr,Ptr2;

  SU_malloc_CheckInit(true);
  if(SU_env_print)
    SU_PrintFunc(false,"SU_free_trace Information : Freeing bloc %p in pid %d (%s:%d)",memblock,getpid(),file,line);
#ifdef _REENTRANT
  SU_CRITICAL_ENTER(SU_alloc_trace_sem);
#endif /* _REENTRANT */
  Ptr = SU_alloc_trace_list;
  Ptr2 = NULL;
  while(Ptr != NULL)
  {
    if(((SU_PAlloc)Ptr->Data)->ptr == (void *)((char *)memblock-8))
      break;
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  if(Ptr == NULL)
  {
    if(SU_env_trace)
      SU_printf_trace_debug("SU_free_trace","was never allocated",memblock,file,line,NULL,0);
    else
      SU_printf_trace_debug("SU_free_trace","already freed, or never allocated",memblock,file,line,NULL,0);
#ifdef _REENTRANT
    SU_CRITICAL_LEAVE(SU_alloc_trace_sem);
#endif /* _REENTRANT */
    return;
  }
  if(((SU_PAlloc)Ptr->Data)->freed)
  {
    SU_printf_trace_debug("SU_free_trace","was freed at",memblock,((SU_PAlloc)Ptr->Data)->file,((SU_PAlloc)Ptr->Data)->line,file,line);
#ifdef _REENTRANT
    SU_CRITICAL_LEAVE(SU_alloc_trace_sem);
#endif /* _REENTRANT */
    return;
  }
  if(*((SU_u32 *)((char *)((SU_PAlloc)Ptr->Data)->ptr+4)) != SU_MALLOC_BOUND_VALUE)
    SU_printf_trace_debug("SU_free_trace","might have been pre-written",memblock,file,line,NULL,0);
  if(*((SU_u32 *)((char *)((SU_PAlloc)Ptr->Data)->ptr+((SU_PAlloc)Ptr->Data)->size+8)) != SU_MALLOC_BOUND_VALUE)
    SU_printf_trace_debug("SU_free_trace","might have been post-written",memblock,file,line,NULL,0);

  SU_total_memory_allocated -= ((SU_PAlloc)Ptr->Data)->size;
  if(SU_env_trace)
  {
    SU_PList Ptr2;

    if(((SU_PAlloc)Ptr->Data)->size <= SU_MALLOC_REUSE_SIZE)
    {
      size_t i;
      for(i=0;i<((SU_PAlloc)Ptr->Data)->size/sizeof(SU_u32);i++)
        *((SU_u32 *)memblock+i) = SU_MALLOC_REUSE_VALUE;
    }
    else
      *((SU_u32 *)memblock) = SU_MALLOC_REUSE_VALUE;
    ((SU_PAlloc)Ptr->Data)->freed = true;
    SU_strcpy(((SU_PAlloc)Ptr->Data)->file,file,sizeof(((SU_PAlloc)Ptr->Data)->file));
    ((SU_PAlloc)Ptr->Data)->line = line;
    /* Check for reused blocs */
    Ptr2 = SU_alloc_trace_list;
    while(Ptr2 != NULL)
    {
      if(((SU_PAlloc)Ptr2->Data)->freed)
      {
        bool reused = false;
        if(((SU_PAlloc)Ptr2->Data)->size <= SU_MALLOC_REUSE_SIZE)
        {
          size_t i;
          for(i=0;i<((SU_PAlloc)Ptr2->Data)->size/sizeof(SU_u32);i++)
          {
            reused = *((SU_u32 *)((char *)((SU_PAlloc)Ptr2->Data)->ptr+8)+i) != SU_MALLOC_REUSE_VALUE;
            if(reused)
              break;
          }
        }
        else
          reused = (*((SU_u32 *)((char *)((SU_PAlloc)Ptr2->Data)->ptr+8)) != SU_MALLOC_REUSE_VALUE);
        if(reused)
        {
          SU_printf_trace_debug("SU_free_trace","might have been reused",(char *)((SU_PAlloc)Ptr2->Data)->ptr+8,((SU_PAlloc)Ptr2->Data)->file,((SU_PAlloc)Ptr2->Data)->line,NULL,0);
        }
      }
      Ptr2 = Ptr2->Next;
    }
  }
  else
  {
    free(((SU_PAlloc)Ptr->Data)->ptr); /* Actually frees the bloc */
    free(Ptr->Data);
    if(Ptr2 == NULL)
      SU_alloc_trace_list = SU_DelElementHead_no_trace(SU_alloc_trace_list);
    else
      Ptr2->Next = SU_DelElementHead_no_trace(Ptr);
  }
#ifdef _REENTRANT
  SU_CRITICAL_LEAVE(SU_alloc_trace_sem);
#endif /* _REENTRANT */
}

void SU_alloc_trace_print(bool detail)
{
  SU_PList Ptr;
  SU_u32 count = 0;

#ifdef _REENTRANT
  SU_CRITICAL_ENTER(SU_alloc_trace_sem);
#endif /* _REENTRANT */
  Ptr = SU_alloc_trace_list;
  while(Ptr != NULL)
  {
    if(!((SU_PAlloc)Ptr->Data)->freed)
    {
      count++;
      if(detail)
        SU_PrintFunc(false,"SkyUtils_SU_alloc_trace_print : %ld %p %ld -> %s:%d",((SU_PAlloc)Ptr->Data)->time,((SU_PAlloc)Ptr->Data)->ptr,((SU_PAlloc)Ptr->Data)->size,((SU_PAlloc)Ptr->Data)->file,((SU_PAlloc)Ptr->Data)->line);
    }
    Ptr = Ptr->Next;
  }
  SU_PrintFunc(false,"SkyUtils_SU_alloc_trace_print : %d blocks",count);
#ifdef _REENTRANT
  SU_CRITICAL_LEAVE(SU_alloc_trace_sem);
#endif /* _REENTRANT */
}

void SU_check_memory(void)
{
  SU_PList Ptr;

  SU_malloc_CheckInit(true);
#ifdef _REENTRANT
  SU_CRITICAL_ENTER(SU_alloc_trace_sem);
#endif /* _REENTRANT */
  Ptr = SU_alloc_trace_list;
  while(Ptr != NULL)
  {
    if(*((SU_u32 *)((char *)((SU_PAlloc)Ptr->Data)->ptr+4)) != SU_MALLOC_BOUND_VALUE)
      SU_printf_trace_debug("SU_check_memory","might have been pre-written",(char *)((SU_PAlloc)Ptr->Data)->ptr+4,((SU_PAlloc)Ptr->Data)->file,((SU_PAlloc)Ptr->Data)->line,NULL,0);
    if(*((SU_u32 *)((char *)((SU_PAlloc)Ptr->Data)->ptr+((SU_PAlloc)Ptr->Data)->size+8)) != SU_MALLOC_BOUND_VALUE)
      SU_printf_trace_debug("SU_check_memory","might have been post-written",(char *)((SU_PAlloc)Ptr->Data)->ptr+4,((SU_PAlloc)Ptr->Data)->file,((SU_PAlloc)Ptr->Data)->line,NULL,0);

    if(((SU_PAlloc)Ptr->Data)->freed)
    {
      bool reused = false;
      if(((SU_PAlloc)Ptr->Data)->size <= SU_MALLOC_REUSE_SIZE)
      {
        size_t i;
        for(i=0;i<((SU_PAlloc)Ptr->Data)->size/sizeof(SU_u32);i++)
        {
          reused = *((SU_u32 *)((char *)((SU_PAlloc)Ptr->Data)->ptr+8)+i) != SU_MALLOC_REUSE_VALUE;
          if(reused)
            break;
        }
      }
      else
        reused = (*((SU_u32 *)((char *)((SU_PAlloc)Ptr->Data)->ptr+8)) != SU_MALLOC_REUSE_VALUE);
      if(reused)
        SU_printf_trace_debug("SU_check_memory","might have been reused",(char *)((SU_PAlloc)Ptr->Data)->ptr+8,((SU_PAlloc)Ptr->Data)->file,((SU_PAlloc)Ptr->Data)->line,NULL,0);
    }
    Ptr = Ptr->Next;
  }
#ifdef _REENTRANT
  SU_CRITICAL_LEAVE(SU_alloc_trace_sem);
#endif /* _REENTRANT */
}

void SU_alloc_stats(SU_MEM_STATS_FUNC *Func)
{
  SU_PList Ptr;

  if(Func == NULL)
    return;

  SU_malloc_CheckInit(true);
#ifdef _REENTRANT
  SU_CRITICAL_ENTER(SU_alloc_trace_sem);
#endif /* _REENTRANT */
  Ptr = SU_alloc_trace_list;
  while(Ptr != NULL)
  {
    if(!((SU_PAlloc)Ptr->Data)->freed)
    {
      Func(((SU_PAlloc)Ptr->Data)->ptr,((SU_PAlloc)Ptr->Data)->size,((SU_PAlloc)Ptr->Data)->time,((SU_PAlloc)Ptr->Data)->file,((SU_PAlloc)Ptr->Data)->line);
    }
    Ptr = Ptr->Next;
  }
#ifdef _REENTRANT
  SU_CRITICAL_LEAVE(SU_alloc_trace_sem);
#endif /* _REENTRANT */
}

size_t SU_alloc_total_size(void)
{
  return SU_total_memory_allocated;
}

