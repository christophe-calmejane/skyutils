/****************************************************************/
/* String unit                                                  */
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
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#pragma warning( disable: 4127)
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

char *SU_CurrentParseString;
char SU_ZeroString[]="";

SKYUTILS_API char *SU_strcat(char *dest,const char *src,size_t len)
{
  size_t pos = strlen(dest);

  if(src == NULL)
    return dest;
  if(pos >= (len-1))
    return dest;
  do
  {
    dest[pos] = src[0];
    pos++;
    src++;
    if(src[0] == 0)
      break;
  } while(pos < (len-1));
  dest[pos] = 0;
  return dest;
}

SKYUTILS_API char *SU_strcpy(char *dest,const char *src,size_t len)
{
  size_t pos = 0;

  if(src != NULL)
  {
    while(pos < (len-1))
    {
      if(src[pos] == 0)
        break;
      dest[pos] = src[pos];
      pos++;
    }
  }
  dest[pos] = 0;
  return dest;
}

SKYUTILS_API int SU_snprintf(char *dest,size_t len, const char *format,...) /* like snprintf, but always NULL terminate dest - Returns -1 if string truncated */
{
  va_list argptr;
  size_t new_len;

  va_start(argptr,format);
#ifdef _WIN32
  _vsnprintf(dest,len-1,format,argptr);
#else /* !_WIN32 */
  vsnprintf(dest,len-1,format,argptr);
#endif /* _WIN32 */
  va_end(argptr);
  dest[len-1] = 0;
  new_len = strlen(dest);
  if(new_len == len-1)
  {
    return -1;
  }
  return (int)new_len;
}

SKYUTILS_API char *SU_nocasestrstr(char *text, const char *tofind)  /* like strstr(), but nocase */
{
   char *ret = text;
   const char *find = tofind;

   while(1)
   {
      if(*find == '\0') return ret;
      if(*text == '\0') return NULL;
      if(toupper(*find) != toupper(*text))
      {
        ret = text+1;
        find = tofind;
        if(toupper(*find) == toupper(*text))
        find++;
      } else
        find++;
      text++;
   }
   return NULL;
}

SKYUTILS_API bool SU_strwcmp(const char *s,const char *wild) /* True if wild equals s (wild may use '*') */
{
  char *pos,*pos2;
  size_t len;
  char tmp[1024];

  while((s[0] != 0) && (wild[0] != 0))
  {
    if(wild[0] == '*') /* Start wild mode */
    {
      if(wild[1] == 0) /* End of wild string */
        return true;
      wild++;
#ifdef __unix__
      pos = strchr(wild,'*');
#else /* !__unix__ */
      pos = strchr((char *)wild,'*');
#endif /* __unix__ */
      if(pos != NULL)
      {
        len = pos-wild+1; /* +1 for the \0 */
        if(len > sizeof(tmp))
          len = sizeof(tmp);
        SU_strcpy(tmp,wild,len);
      }
      else
        SU_strcpy(tmp,wild,sizeof(tmp));
#ifdef __unix__
      pos2 = strstr(s,tmp);
#else /* !__unix__ */
      pos2 = strstr((char *)s,tmp);
#endif /* __unix__ */
      len = strlen(tmp);
      if(pos2 == NULL)
        return false;
      s = pos2 + len;
      wild += len;
      if(pos == NULL) /* In last part, check for length */
        return (s[0] == 0);
    }
    else
    {
      if(s[0] == wild[0])
      {
        s++;
        wild++;
      }
      else
        return false;
    }
  }
  if((s[0] == 0) && (wild[0] == 0))
    return true;
  if((s[0] == 0) && (wild[0] == '*'))
  {
    if(wild[1] == 0)
      return true;
  }
  return false;
}

SKYUTILS_API bool SU_nocasestrwcmp(const char *s,const char *wild) /* Same as strwcmp but without case */
{
  char *pos,*pos2;
  size_t len;
  char tmp[1024];

  while((s[0] != 0) && (wild[0] != 0))
  {
    if(wild[0] == '*') /* Start wild mode */
    {
      if(wild[1] == 0) /* End of wild string */
        return true;
      wild++;
#ifdef __unix__
      pos = strchr(wild,'*');
#else /* !__unix__ */
      pos = strchr((char *)wild,'*');
#endif /* __unix__ */
      if(pos != NULL)
      {
        len = pos-wild+1; /* +1 for the \0 */
        if(len > sizeof(tmp))
          len = sizeof(tmp);
        SU_strcpy(tmp,wild,len);
      }
      else
        SU_strcpy(tmp,wild,sizeof(tmp));
      pos2 = SU_nocasestrstr((char *)s,tmp);
      len = strlen(tmp);
      if(pos2 == NULL)
        return false;
      s = pos2 + len;
      wild += len;
      if(pos == NULL) /* In last part, check for length */
        return (s[0] == 0);
    }
    else
    {
      if(toupper(s[0]) == toupper(wild[0]))
      {
        s++;
        wild++;
      }
      else
        return false;
    }
  }
  if((s[0] == 0) && (wild[0] == 0))
    return true;
  if((s[0] == 0) && (wild[0] == '*'))
  {
    if(wild[1] == 0)
      return true;
  }
  return false;
}

SKYUTILS_API bool SU_strwparse(const char *s,const char *wild,char buf[],int size,char *buf_ptrs[],int *ptrs_count) /* True if wild equals s (wild may use '*') */
{
  char *pos,*pos2;
  size_t len;
  char tmp[1024];
  int count = 0;
  int buf_pos = 0,l;

  while((s[0] != 0) && (wild[0] != 0))
  {
   if(wild[0] == '*') /* Start wild mode */
   {
     if(wild[1] == 0) /* End of wild string */
     {
       buf_ptrs[count++] = &buf[buf_pos];
       *ptrs_count = count;
       l = (int)(strlen(s) + 1); /* +1 for the \0 */
       if((l+buf_pos) > size)
         l = size - buf_pos;
       SU_strcpy(&buf[buf_pos],s,l);
       return true;
     }
     wild++;
  #ifdef __unix__
     pos = strchr(wild,'*');
  #else /* !__unix__ */
     pos = strchr((char *)wild,'*');
  #endif /* __unix__ */
     if(pos != NULL)
     {
       len = pos-wild+1; /* +1 for the \0 */
       if(len > sizeof(tmp))
         len = sizeof(tmp);
       SU_strcpy(tmp,wild,len);
     }
     else
       SU_strcpy(tmp,wild,sizeof(tmp));
  #ifdef __unix__
     pos2 = strstr(s,tmp);
  #else /* !__unix__ */
     pos2 = strstr((char *)s,tmp);
  #endif /* __unix__ */
     len = strlen(tmp);
     if(pos2 == NULL)
       return false;

     buf_ptrs[count++] = &buf[buf_pos];
     *ptrs_count = count;
     l = (int)(pos2-s+1); /* +1 for the \0 */
     if((l+buf_pos) > size)
       l = size - buf_pos;
     SU_strcpy(&buf[buf_pos],s,l);
     buf_pos += l;

     s = pos2 + len;
     wild += len;
     if(pos == NULL) /* In last part, check for length */
       return (s[0] == 0);
   }
   else
   {
     if(s[0] == wild[0])
     {
       s++;
       wild++;
     }
     else
       return false;
   }
  }
  if((s[0] == 0) && (wild[0] == 0))
   return true;
  if((s[0] == 0) && (wild[0] == '*'))
  {
   if(wild[1] == 0)
   {
     if(buf_pos != 0)
     {
       buf_ptrs[count++] = &buf[buf_pos-1];
     }
     else
     {
       if(size > 0)
       {
         buf_ptrs[count++] = buf;
         buf[0] = 0;
       }
     }
     *ptrs_count = count;
     return true;
   }
  }
  return false;
}

SKYUTILS_API bool SU_nocasestrwparse(const char *s,const char *wild,char buf[],int size,char *buf_ptrs[],int *ptrs_count) /* True if wild equals s (wild may use '*') */
{
  char *tmp = SU_strdup(s);
  char *tmp_wild = SU_strdup(wild);
  bool ret;
  
  SU_strtolower(tmp);
  SU_strtolower(tmp_wild);
  ret = SU_strwparse(tmp,tmp_wild,buf,size,buf_ptrs,ptrs_count);
  free(tmp);
  free(tmp_wild);
  return ret;
}


/* Returns MAX(length of a string,max) (not including terminating null char) */
SKYUTILS_API unsigned int SU_strnlen(const char *s,unsigned int max)
{
  unsigned int i = 0;

  while(s[i] != 0)
  {
    if(i >= max)
      return max;
    i++;
  }
  return i;
}


SKYUTILS_API bool SU_ReadLine(FILE *fp,char S[],int len)
{
  int i;
  char c;

  i = 0;
  S[0] = 0;
  if(fread(&c,1,1,fp) != 1)
    return 0;
  while((c == 0x0A) || (c == 0x0D))
  {
    if(fread(&c,1,1,fp) != 1)
      return 0;
  }
  while((c != 0x0A) && (c != 0x0D))
  {
    if(i >= (len-1))
      break;
    S[i++] = c;
    if(fread(&c,1,1,fp) != 1)
      break;
  }
  S[i] = 0;
  return 1;
}

/* Parses a config file with lines like "Name Value" */
/* Rreturns false on EOF */
SKYUTILS_API bool SU_ParseConfig(FILE *fp,char Name[],int nsize,char Value[],int vsize)
{
  char S[4096];
  char *p,*q;

  while(SU_ReadLine(fp,S,sizeof(S)))
  {
    if((S[0] == '#') || (S[0] == 0))
      continue;
    q = S;
    while((q[0] == ' ') || (q[0] == '\t'))
      q++;
    if((q[0] == '#') || (q[0] == 0))
      continue;
    Value[0] = 0;
    p = strchr(q,' ');
    if(p != NULL)
      p[0] = 0;
    SU_strcpy(Name,q,nsize);
    if(p == NULL)
      return true;
    p++;
    while((p[0] == ' ') || (p[0] == '\t'))
      p++;
    SU_strcpy(Value,p,vsize);
    return true;
  }
  return false;
}


SKYUTILS_API char *SU_TrimLeft(const char *S)
{
  int i;

  if(S == NULL)
    return NULL;
  i = 0;
  while(S[i] == ' ')
  {
    i++;
  }
  return (char *)(S+i);
}

SKYUTILS_API void SU_TrimRight(char *S)
{
  int i;

  if(S == NULL)
    return;
  i = (int)(strlen(S)-1);
  while(S[i] == ' ')
  {
    S[i] = 0;
    i--;
  }
}

SKYUTILS_API char *SU_strparse(char *s,char delim)
{
  char *p,*ret;

  if(s != NULL)
    SU_CurrentParseString = s;
  if(SU_CurrentParseString == NULL)
    return NULL;
  if(SU_CurrentParseString[0] == delim)
  {
    SU_CurrentParseString++;
    return SU_ZeroString;
  }
  p = strchr(SU_CurrentParseString,delim);
  ret = SU_CurrentParseString;
  SU_CurrentParseString = p;
  if(p != NULL)
  {
    p[0] = 0;
    SU_CurrentParseString++;
  }
  return ret;
}

 /* Extracts file name (with suffix) from path */
SKYUTILS_API void SU_ExtractFileName(const char Path[],char FileName[],const int len)
{
  char *pos;

#ifdef __unix__
  pos = strrchr(Path,'/');
#else /* !__unix__ */
  pos = strrchr((char *)Path,'/');
#endif /* __unix__ */
  if(pos == NULL)
    SU_strcpy(FileName,Path,len);
  else
    SU_strcpy(FileName,pos+1,len);
}

SKYUTILS_API char *SU_strchrl(const char *s,const char *l,char *found)
{
  int len,i;

  len = (int)strlen(l);
  while(s[0] != 0)
  {
    for(i=0;i<len;i++)
    {
      if(s[0] == l[i])
      {
        if(found != NULL)
          *found = s[0];
        return (char *)s;
      }
    }
    s++;
  }
  return NULL;
}

SKYUTILS_API char *SU_strrchrl(const char *s,const char *l,char *found)
{
  size_t len,i;
  int j;

  len = strlen(l);
  for(j=(int)(strlen(s)-1);j>=0;j--)
  {
    for(i=0;i<len;i++)
    {
      if(s[j] == l[i])
      {
        if(found != NULL)
          *found = s[j];
        return (char *)(s+j);
      }
    }
  }
  return NULL;
}

SKYUTILS_API unsigned char SU_toupper(unsigned char c)
{
  if((c >= 'a') && (c <= 'z'))
    return (c-32);
  if(c >= 224)
    return (c-32);
  return c;
}

SKYUTILS_API unsigned char SU_tolower(unsigned char c)
{
  if((c >= 'A') && (c <= 'Z'))
    return (c+32);
  if((c >= 192) && (c <= 223))
    return (c+32);
  return c;
}

SKYUTILS_API char *SU_strtoupper(char *s)
{
  int i=0;
  while(s[i] != 0)
  {
    s[i] = SU_toupper(s[i]);
    i++;
  }
  return s;
}

SKYUTILS_API char *SU_strtolower(char *s)
{
  int i=0;
  while(s[i] != 0)
  {
    s[i] = SU_tolower(s[i]);
    i++;
  }
  return s;
}

SKYUTILS_API bool SU_strcasecmp(const char *s,const char *p)
{
  while((*s != 0) && (*p != 0))
  {
    if(SU_toupper(*s) != SU_toupper(*p))
      return false;
    s++;p++;
  }
  return ((*s == 0) && (*p == 0));
}

SKYUTILS_API int SU_htoi(const char *value)
{
  unsigned int val = 0;

  sscanf(value,"%x",&val);
  return val;
}

SKYUTILS_API int SU_atoi(const char *value)
{
  if(strncasecmp(value,"0x",2) == 0) /* HEX value */
    return SU_htoi(value);
  else
    return atoi(value);
}

SKYUTILS_API const void* SU_memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen)
{
	size_t haystackpos = 0;
	const void* ptr;

	while((haystackpos+needlelen) <= haystacklen)
	{
		ptr = memchr(((char*)haystack)+haystackpos,*((char*)needle),haystacklen-haystackpos);
		if(ptr != NULL) // Found the first byte of the needle
		{
			if(memcmp(ptr,needle,needlelen) == 0) // If comparison of the whole needle is OK, we've found it in our haystack!
				return ptr;
		}
		haystackpos++;
	}
	return NULL; // Not found
}


#ifdef _WIN32
static char _SU_w32ErrMsg[512];
SKYUTILS_API char *SU_strerror(int ErrorCode)
{
  char *p;

  if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL,ErrorCode,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),_SU_w32ErrMsg,sizeof(_SU_w32ErrMsg),NULL) == 0)
  {
    snprintf(_SU_w32ErrMsg,sizeof(_SU_w32ErrMsg),"#%d",ErrorCode);
    return _SU_w32ErrMsg;
  }

  p = _SU_w32ErrMsg;
  while((*p != 0) && (*p != 0x0d) && (*p != 0x0a))
    ++p;

  p[0] = 0;
  return _SU_w32ErrMsg;
}
#else /* !_WIN32 */
SKYUTILS_API char *SU_strerror(int ErrorCode)
{
  return strerror(ErrorCode);
}
#endif /* _WIN32 */

