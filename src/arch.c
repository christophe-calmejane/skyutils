/****************************************************************/
/* Archive unit                                                 */
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


#ifdef SU_USE_ARCH
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#ifdef __BORLANDC__
#include <utime.h>
#define utime _utime
#else /* !__BORLANDC__ */
#include <sys/utime.h>
#endif /* __BORLANDC__ */
#else /* !_WIN32 */
#include <utime.h>
#endif /* _WIN32 */
#include <skyutils/skyutils.h>
#ifdef HAVE_ZLIB
#include <zlib.h>
#endif /* HAVE_ZLIB */
#ifdef HAVE_BZLIB
#include <bzlib.h>
#endif /* HAVE_BZLIB */
#ifdef HAVE_MINILZO
#include <minilzo.h>
#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]
static HEAP_ALLOC(SU_AR_wrkmem,LZO1X_1_MEM_COMPRESS);
#endif /* HAVE_MINILZO */

#ifndef SU_TRACE_INTERNAL
#ifdef SU_MALLOC_TRACE
#undef malloc
#undef calloc
#undef realloc
#undef strdup
#undef free
#endif /* SU_MALLOC_TRACE */
#endif /* !SU_TRACE_INTERNAL */

#define SU_AR_SIGNATURE "SkyArch4"


typedef struct
{
  SU_u32 Index;    /* Resource Index (any,unique)      */
  char *Name;      /* Resource Name (NULL if none)     */
  SU_u32 Offset;   /* Data offset from start of file   */
  SU_u32 CompSize; /* Compressed size                  */
  SU_u32 CompType; /* Compression type                 */
  SU_u32 Reserved; /* Reserved for future use          */
  SU_u32 OrigSize; /* Original size                    */
  SU_u32 OrigTime; /* Original time stamp              */

  void   *Data;    /* Data pointer used when creating  */
  bool   IsFile;   /* If Data represents a filename    */
} SU_TResHdr, *SU_PResHdr;

struct SU_SArch
{
  FILE *fp;              /* Resource file              */
  SU_TResHdr *Resources; /* Table of resources header  */
  SU_u32 NbRes;          /* Number of resources        */
  bool Flush;            /* Archive need to be flushed */
};

/* ------------------- INTERNAL FUNCTIONS ------------------- */

bool _SU_AR_CheckIndexNameUnique(SU_PArch Arch,SU_u32 Index,const char *Name)
{
  SU_u32 i;

  for(i=0;i<Arch->NbRes;i++)
  {
    if((Index != 0) && (Arch->Resources[i].Index == Index))
      return false;
    if((Name != NULL) && (Arch->Resources[i].Name != NULL) && (strcmp(Arch->Resources[i].Name,Name) == 0))
      return false;
  }
  return true;
}

bool _SU_AR_CompressFile(SU_PResHdr RH)
{
  FILE *fp;
  struct stat st;

  fp = fopen((char *)RH->Data,"rb");
  if(fp == NULL)
    return false;
  if(stat((char *)RH->Data,&st) != 0)
    return false;
  fseek(fp,(long)0,SEEK_END);
  RH->OrigSize = ftell(fp);
  RH->OrigTime = (SU_u32) st.st_ctime;
  rewind(fp);

  switch(RH->CompType)
  {
    case SU_ARCH_COMP_NONE :
      /* RH->Data doesn't change */
      RH->CompSize = RH->OrigSize;
      break;
#ifdef HAVE_ZLIB
    case SU_ARCH_COMP_Z :
      /* TO DO */
      /* Set RH->Data to new FileName */
      /* Set RH->CompSize to correct value */
      RH->CompSize = RH->OrigSize;
      RH->CompType = SU_ARCH_COMP_NONE;
      break;
#endif /* HAVE_ZLIB */
#ifdef HAVE_BZLIB
    case SU_ARCH_COMP_BZ :
      /* TO DO */
      /* Set RH->Data to new FileName */
      /* Set RH->CompSize to correct value */
      RH->CompSize = RH->OrigSize;
      RH->CompType = SU_ARCH_COMP_NONE;
      break;
#endif /* HAVE_BZLIB */
#ifdef HAVE_MINILZO
    case SU_ARCH_COMP_LZO :
    {
      void *in_buf = malloc((size_t)RH->OrigSize);
      void *out_buf = malloc((size_t)(RH->OrigSize * 1.01 + 1000));

      if((in_buf == NULL) || (out_buf == NULL))
      {
        if(in_buf) free(in_buf);
        if(out_buf) free(out_buf);
        fclose(fp);
        RH->CompType = SU_ARCH_COMP_NONE;
        return _SU_AR_CompressFile(RH); /* Fallback to no compression */
      }
      if(fread(in_buf,(size_t)1,RH->OrigSize,fp) != RH->OrigSize)
      {
        free(in_buf);
        free(out_buf);
        fclose(fp);
        RH->CompType = SU_ARCH_COMP_NONE;
        return _SU_AR_CompressFile(RH); /* Fallback to no compression */
      }
      if(lzo1x_1_compress(in_buf,RH->OrigSize,out_buf,(lzo_uintp)&RH->CompSize,SU_AR_wrkmem) != LZO_E_OK)
      {
        free(in_buf);
        free(out_buf);
        fclose(fp);
        RH->CompType = SU_ARCH_COMP_NONE;
        return _SU_AR_CompressFile(RH); /* Fallback to no compression */
      }

      /* Everything OK, change file to buffer */
      RH->Data = out_buf;
      free(in_buf);
      RH->IsFile = false;
      break;
    }
#endif /* HAVE_MINILZO */
    default :
      fclose(fp);
      return false;
  }

  fclose(fp);
  return true;
}

bool _SU_AR_CopyFileToArchive(FILE *fp,SU_PResHdr RH,const char FileName[])
{
  FILE *in;
  char Buf[32768];
  SU_u32 total = 0,len;

  in = fopen(FileName,"rb");
  if(in == NULL)
    return false;

  while(total < RH->CompSize)
  {
    len = RH->CompSize - total;
    if(len > sizeof(Buf))
      len = sizeof(Buf);
    if(fread(Buf,(size_t)1,len,in) != len)
    {
      fclose(in);
      return false;
    }
    if(fwrite(Buf,(size_t)1,len,fp) != len)
    {
      fclose(in);
      return false;
    }
    total += len;
  }
  fclose(in);
  return true;
}

bool _SU_AR_CopyFileToDisk(FILE *fp,SU_PResHdr RH,const char FileName[])
{
  FILE *out;
  char Buf[32768];
  SU_u32 total = 0,len;

  out = fopen(FileName,"wb");
  if(out == NULL)
    return false;

  while(total < RH->OrigSize)
  {
    len = RH->OrigSize - total;
    if(len > sizeof(Buf))
      len = sizeof(Buf);
    if(fread(Buf,(size_t)1,len,fp) != len)
    {
      fclose(out);
      return false;
    }
    if(fwrite(Buf,(size_t)1,len,out) != len)
    {
      fclose(out);
      return false;
    }
    total += len;
  }
  fclose(out);
  return true;
}

bool _SU_AR_CopyBufferToDisk(void *buffer,SU_u32 Size,const char FileName[])
{
  FILE *fp;

  fp = fopen(FileName,"wb");
  if(fp == NULL)
    return false;

  if(fwrite(buffer,(size_t)1,Size,fp) != Size)
  {
    fclose(fp);
    return false;
  }
  fclose(fp);
  return true;
}

SU_PArch _SU_AR_ReadHeaders(FILE *fp)
{
  SU_PArch Arch;
  SU_u32 NbRes,i;
  char Signature[sizeof(SU_AR_SIGNATURE)-1];
  SU_u32 c;

  if(fread(&Signature,(size_t)1,sizeof(Signature),fp) != sizeof(Signature))
  {
    fclose(fp);
    return NULL;
  }
  if(strncmp(Signature,SU_AR_SIGNATURE,sizeof(Signature)) != 0)
  {
    fclose(fp);
    return NULL;
  }
  if(fread(&NbRes,(size_t)1,sizeof(NbRes),fp) != sizeof(NbRes))
  {
    fclose(fp);
    return NULL;
  }

  Arch = (SU_PArch) malloc(sizeof(SU_TArch));
  memset(Arch,0,sizeof(SU_TArch));
  Arch->fp = fp;
  Arch->NbRes = NbRes;
  Arch->Resources = (SU_TResHdr *) malloc(sizeof(SU_TResHdr)*NbRes);
  memset(Arch->Resources,0,sizeof(SU_TResHdr)*NbRes);
  for(i=0;i<NbRes;i++)
  {
    if(fread(&Arch->Resources[i].CompSize,(size_t)1,sizeof(Arch->Resources[i].CompSize),fp) != sizeof(Arch->Resources[i].CompSize))
    {
      SU_AR_CloseArchive(Arch);
      return NULL;
    }
    if(fread(&Arch->Resources[i].CompType,(size_t)1,sizeof(Arch->Resources[i].CompType),fp) != sizeof(Arch->Resources[i].CompType))
    {
      SU_AR_CloseArchive(Arch);
      return NULL;
    }
    if(fread(&Arch->Resources[i].Reserved,(size_t)1,sizeof(Arch->Resources[i].Reserved),fp) != sizeof(Arch->Resources[i].Reserved))
    {
      SU_AR_CloseArchive(Arch);
      return NULL;
    }
    if(fread(&Arch->Resources[i].OrigSize,(size_t)1,sizeof(Arch->Resources[i].OrigSize),fp) != sizeof(Arch->Resources[i].OrigSize))
    {
      SU_AR_CloseArchive(Arch);
      return NULL;
    }
    if(fread(&Arch->Resources[i].OrigTime,(size_t)1,sizeof(Arch->Resources[i].OrigTime),fp) != sizeof(Arch->Resources[i].OrigTime))
    {
      SU_AR_CloseArchive(Arch);
      return NULL;
    }
    if(fread(&Arch->Resources[i].Index,(size_t)1,sizeof(Arch->Resources[i].Index),fp) != sizeof(Arch->Resources[i].Index))
    {
      SU_AR_CloseArchive(Arch);
      return NULL;
    }
    /* Check if there is a resource name */
    if(fread(&c,(size_t)1,sizeof(c),fp) != sizeof(c))
    {
      SU_AR_CloseArchive(Arch);
      return NULL;
    }
    if(c != 0)
    {
      Arch->Resources[i].Name = (char *) malloc(c);
      if(fread(Arch->Resources[i].Name,(size_t)1,c,fp) != c)
      {
        SU_AR_CloseArchive(Arch);
        return NULL;
      }
    }
    Arch->Resources[i].Offset = ftell(fp);
    if(fseek(fp,(long)(Arch->Resources[i].CompSize),SEEK_CUR) != 0)
    {
      SU_AR_CloseArchive(Arch);
      return NULL;
    }
  }

  return Arch;
}

bool _SU_AR_Flush(SU_PArch Arch)
{
  SU_u32 i;
  char Signature[sizeof(SU_AR_SIGNATURE)-1];
  bool res = true;
  SU_u32 Ofs = 0;
  SU_u32 c;

  strncpy(Signature,SU_AR_SIGNATURE,sizeof(Signature));
  if(fwrite(&Signature,(size_t)1,sizeof(Signature),Arch->fp) != sizeof(Signature))
    res = false;
  if(fwrite(&Arch->NbRes,(size_t)1,sizeof(Arch->NbRes),Arch->fp) != sizeof(Arch->NbRes))
    res = false;

  for(i=0;i<Arch->NbRes;i++)
  {
    if(Arch->Resources[i].IsFile)
    {
      if(!_SU_AR_CompressFile(&Arch->Resources[i]))
        res = false;
    }

    if(fwrite(&Arch->Resources[i].CompSize,(size_t)1,sizeof(Arch->Resources[i].CompSize),Arch->fp) != sizeof(Arch->Resources[i].CompSize))
      res = false;
    if(fwrite(&Arch->Resources[i].CompType,(size_t)1,sizeof(Arch->Resources[i].CompType),Arch->fp) != sizeof(Arch->Resources[i].CompType))
      res = false;
    if(fwrite(&Arch->Resources[i].Reserved,(size_t)1,sizeof(Arch->Resources[i].Reserved),Arch->fp) != sizeof(Arch->Resources[i].Reserved))
      res = false;
    if(fwrite(&Arch->Resources[i].OrigSize,(size_t)1,sizeof(Arch->Resources[i].OrigSize),Arch->fp) != sizeof(Arch->Resources[i].OrigSize))
      res = false;
    if(fwrite(&Arch->Resources[i].OrigTime,(size_t)1,sizeof(Arch->Resources[i].OrigTime),Arch->fp) != sizeof(Arch->Resources[i].OrigTime))
      res = false;
    if(fwrite(&Arch->Resources[i].Index,(size_t)1,sizeof(Arch->Resources[i].Index),Arch->fp) != sizeof(Arch->Resources[i].Index))
      res = false;
    if(Arch->Resources[i].Name == NULL)
    {
      c = 0;
      if(fwrite(&c,(size_t)1,sizeof(c),Arch->fp) != sizeof(c))
        res = false;
    }
    else
    {
      c = (SU_u32)strlen(Arch->Resources[i].Name) + 1; /* Store the tailing \0 */
      if(fwrite(&c,(size_t)1,sizeof(c),Arch->fp) != sizeof(c))
        res = false;
      if(fwrite(Arch->Resources[i].Name,(size_t)1,c,Arch->fp) != c)
        res = false;
    }
    if(Arch->Resources[i].Data != NULL)
    {
      if(Arch->Resources[i].IsFile)
      {
        if(!_SU_AR_CopyFileToArchive(Arch->fp,&Arch->Resources[i],(char *)Arch->Resources[i].Data))
          res = false;
      }
      else
      {
        if(fwrite(Arch->Resources[i].Data,(size_t)1,(size_t)(Arch->Resources[i].CompSize),Arch->fp) != Arch->Resources[i].CompSize)
          res = false;
      }
      free(Arch->Resources[i].Data);
    }
    else
      res = false;
  }
  /* Write ofs to start of archive... should always be 0 */
  if(fwrite(&Ofs,(size_t)1,sizeof(Ofs),Arch->fp) != sizeof(Ofs))
    res = false;
  return res;
}

SU_PRes _SU_AR_ReadResource(SU_PArch Arch,const SU_u32 ResNum,bool GetData)
{
  SU_PRes Res;
  SU_PResHdr RH = &Arch->Resources[ResNum];

  Res = (SU_PRes) malloc(sizeof(SU_TRes));
  memset(Res,0,sizeof(SU_TRes));
  Res->Size = RH->OrigSize;
  Res->Stamp = RH->OrigTime;
  Res->Index = RH->Index;
  Res->Name = RH->Name;
  if(GetData)
  {
    void *compressed = malloc(RH->CompSize);
    if(compressed == NULL)
    {
      SU_AR_FreeRes(Res);
      return NULL;
    }
    if(fseek(Arch->fp,(long)(RH->Offset),SEEK_SET) != 0)
    {
      free(compressed);
      SU_AR_FreeRes(Res);
      return NULL;
    }
    if(fread(compressed,(size_t)1,(size_t)(RH->CompSize),Arch->fp) != RH->CompSize)
    {
      free(compressed);
      SU_AR_FreeRes(Res);
      return NULL;
    }
    switch(RH->CompType)
    {
      case SU_ARCH_COMP_NONE :
        Res->Data = compressed;
        break;
#ifdef HAVE_ZLIB
      case SU_ARCH_COMP_Z :
        /* TO DO */
        return NULL;
        uncompress(NULL,NULL,NULL,0);
        break;
#endif /* HAVE_ZLIB */
#ifdef HAVE_BZLIB
      case SU_ARCH_COMP_BZ :
        /* TO DO */
        return NULL;
        BZ2_bzBuffToBuffDecompress(NULL,NULL,NULL,0,0,0);
        break;
#endif /* HAVE_BZLIB */
#ifdef HAVE_MINILZO
      case SU_ARCH_COMP_LZO :
      {
        lzo_uint dummy;

        Res->Data = malloc((size_t)(Res->Size));
        if((lzo1x_decompress(compressed,RH->CompSize,Res->Data,&dummy,NULL) != LZO_E_OK) || (dummy != Res->Size))
        {
          free(compressed);
          SU_AR_FreeRes(Res);
          return NULL;
        }
        free(compressed);
        break;
      }
#endif /* HAVE_MINILZO */
      default :
        free(compressed);
        SU_AR_FreeRes(Res);
        return NULL;
    }
  }
  return Res;
}

void *_SU_AR_ReadCompressedBuffer(SU_PResHdr RH,FILE *fp)
{
  void *compressed = malloc(RH->CompSize);

  if(compressed == NULL)
  {
    return NULL;
  }
  if(fread(compressed,(size_t)1,(size_t)RH->CompSize,fp) != RH->CompSize)
  {
    free(compressed);
    return NULL;
  }
  return compressed;
}

bool _SU_AR_ReadResourceToFile(SU_PArch Arch,const SU_u32 ResNum,const char FileName[])
{
  struct utimbuf ut;
  FILE *out;
  SU_PResHdr RH = &Arch->Resources[ResNum];

  out = fopen(FileName,"wb");
  if(out == NULL)
    return false;
  fclose(out);

  if(fseek(Arch->fp,(long)(RH->Offset),SEEK_SET) != 0)
  {
    unlink(FileName);
    return false;
  }

  switch(RH->CompType)
  {
    case SU_ARCH_COMP_NONE :
      if(!_SU_AR_CopyFileToDisk(Arch->fp,&Arch->Resources[ResNum],FileName))
      {
        unlink(FileName);
        return false;
      }
      break;
#ifdef HAVE_ZLIB
    case SU_ARCH_COMP_Z :
      /* TO DO */
      unlink(FileName);
      return false;
      break;
#endif /* HAVE_ZLIB */
#ifdef HAVE_BZLIB
    case SU_ARCH_COMP_BZ :
      /* TO DO */
      unlink(FileName);
      return false;
      break;
#endif /* HAVE_BZLIB */
#ifdef HAVE_MINILZO
    case SU_ARCH_COMP_LZO :
    {
      lzo_uint dummy;
      void *compressed = _SU_AR_ReadCompressedBuffer(RH,Arch->fp);
      void *uncompressed;

      if(compressed == NULL)
      {
        unlink(FileName);
        return false;
      }
      uncompressed = malloc((size_t)(RH->OrigSize));
      if((lzo1x_decompress(compressed,RH->CompSize,uncompressed,&dummy,NULL) != LZO_E_OK) || (dummy != RH->OrigSize))
      {
        free(uncompressed);
        free(compressed);
        unlink(FileName);
        return false;
      }
      _SU_AR_CopyBufferToDisk(uncompressed,RH->OrigSize,FileName);
      free(uncompressed);
      free(compressed);
      break;
    }
      /* TO DO */
      unlink(FileName);
      return false;
      break;
#endif /* HAVE_MINILZO */
    default :
      unlink(FileName);
      return false;
  }

  ut.actime = RH->OrigTime;
  ut.modtime = RH->OrigTime;
  utime(FileName,&ut);
  return true;
}

bool _SU_AR_CheckInits(void);
bool _SU_AR_CheckInits(void)
{
  static bool _su_ar_init_done = false;

  if(_su_ar_init_done == false)
  {
#ifdef HAVE_MINILZO
    if(lzo_init() != LZO_E_OK)
      return false;
#endif /* HAVE_MINILZO */
    _su_ar_init_done = true;
  }
  return true;
}


/* ------------------- EXPORTED FUNCTIONS ------------------- */

/* Opens a skyutils archive file (or a binary file [exe/dll] if the archive is selfcontained) */
SKYUTILS_API SU_PArch SU_AR_OpenArchive(const char FileName[])
{
  FILE *fp;
  SU_u32 ofs;
  SU_PArch Arch;

  if(!_SU_AR_CheckInits())
    return NULL;

  fp = fopen(FileName,"rb");
  if(fp == NULL)
    return NULL;
  /* Check if it's an archive */
  Arch = _SU_AR_ReadHeaders(fp);
  if(Arch)
    return Arch;

  /* Check if it's a selfcontained archive */
  fp = fopen(FileName,"rb");
  if(fp == NULL)
    return NULL;
  if(fseek(fp,-(signed)sizeof(ofs),SEEK_END) != 0)
  {
    fclose(fp);
    return NULL;
  }
  if(fread(&ofs,(size_t)1,sizeof(ofs),fp) != sizeof(ofs))
  {
    fclose(fp);
    return NULL;
  }
  if(fseek(fp,(long)ofs,SEEK_SET) != 0)
  {
    fclose(fp);
    return NULL;
  }
  return _SU_AR_ReadHeaders(fp);
}

/* Reads resource ResNum (0 is the first one) (NULL if failed) */
SKYUTILS_API SU_PRes SU_AR_ReadRes(SU_PArch Arch,const SU_u32 ResNum,bool GetData)
{
  if(Arch == NULL)
    return NULL;
  if(ResNum >= Arch->NbRes)
    return NULL;

  return _SU_AR_ReadResource(Arch,ResNum,GetData);
}

/* Reads resource of Index ResIndex (Index is unique, but can be any value) (NULL if failed) */
SKYUTILS_API SU_PRes SU_AR_ReadResIndex(SU_PArch Arch,const SU_u32 ResIndex,bool GetData)
{
  SU_u32 i;

  if(Arch == NULL)
    return NULL;

  for(i=0;i<Arch->NbRes;i++)
  {
    if(Arch->Resources[i].Index == ResIndex)
      return _SU_AR_ReadResource(Arch,i,GetData);
  }
  return NULL;
}

/* Reads resource named ResName (NULL if failed) */
SKYUTILS_API SU_PRes SU_AR_ReadResName(SU_PArch Arch,const char *ResName,bool GetData)
{
  SU_u32 i;

  if(Arch == NULL)
    return NULL;

  for(i=0;i<Arch->NbRes;i++)
  {
    if(Arch->Resources[i].Name && (strcmp(Arch->Resources[i].Name,ResName) == 0))
      return _SU_AR_ReadResource(Arch,i,GetData);
  }
  return NULL;
}

/* Save resource ResNum to FileName (0 is the first one) (true on success) */
SKYUTILS_API bool SU_AR_ReadResToFile(SU_PArch Arch,const SU_u32 ResNum,const char FileName[])
{
  if(Arch == NULL)
    return false;

  if(ResNum >= Arch->NbRes)
    return false;

  return _SU_AR_ReadResourceToFile(Arch,ResNum,FileName);
}

/* Saves resource ResIndex to FileName (Index is unique, but can be any value) (true on success) */
SKYUTILS_API bool SU_AR_ReadResIndexToFile(SU_PArch Arch,const SU_u32 ResIndex,const char FileName[])
{
  SU_u32 i;

  if(Arch == NULL)
    return false;

  for(i=0;i<Arch->NbRes;i++)
  {
    if(Arch->Resources[i].Index == ResIndex)
      return _SU_AR_ReadResourceToFile(Arch,i,FileName);
  }
  return false;
}

/* Saves resource named ResName to FileName (Index is unique, but can be any value) (true on success) */
SKYUTILS_API bool SU_AR_ReadResNameToFile(SU_PArch Arch,const char *ResName,const char FileName[])
{
  SU_u32 i;

  if(Arch == NULL)
    return false;

  for(i=0;i<Arch->NbRes;i++)
  {
    if(Arch->Resources[i].Name && (strcmp(Arch->Resources[i].Name,ResName) == 0))
      return _SU_AR_ReadResourceToFile(Arch,i,FileName);
  }
  return false;
}

/* Creates a new archive file. FileName can't be NULL */
SKYUTILS_API SU_PArch SU_AR_CreateArchive(const char FileName[])
{
  SU_PArch Arch;
  FILE *fp;

  if(!_SU_AR_CheckInits())
    return NULL;

  fp = fopen(FileName,"wb");
  if(fp == NULL)
    return NULL;

  Arch = (SU_PArch) malloc(sizeof(SU_TArch));
  memset(Arch,0,sizeof(SU_TArch));
  Arch->fp = fp;
  Arch->Flush = true;

  return Arch;
}

/* Adds a resource to the archive (Data can be freed upon return) (Index must be unique, or 0) (Name must be unique, or NULL) (true on success) */
SKYUTILS_API bool SU_AR_AddRes(SU_PArch Arch,void *Data,SU_u32 Size,time_t Time,SU_AR_COMP_TYPE Type,SU_u32 Index,const char *Name)
{
  SU_PResHdr RH;

  if(Arch == NULL)
    return false;

  if(!_SU_AR_CheckIndexNameUnique(Arch,Index,Name))
    return false;

  Arch->NbRes++;
  Arch->Resources = (SU_TResHdr *) realloc(Arch->Resources,sizeof(SU_TResHdr)*Arch->NbRes);
  RH = &Arch->Resources[Arch->NbRes-1];
  memset(RH,0,sizeof(SU_TResHdr));
  RH->OrigSize = Size;
  RH->OrigTime = (SU_u32) Time;
  RH->CompType = Type;
  RH->Index = Index;
  if(Name)
    RH->Name = strdup(Name);
  if((Size == 0) && (Time == 0)) /* Data represents a FileName */
  {
    RH->Data = SU_strdup(Data);
    RH->IsFile = true;
  }
  else
  {
    switch(Type)
    {
      case SU_ARCH_COMP_NONE :
        RH->Data = malloc((size_t)Size);
        memcpy(RH->Data,Data,(size_t)Size);
        RH->CompSize = Size;
        break;
#ifdef HAVE_ZLIB
      case SU_ARCH_COMP_Z :
        /* TO DO */
        RH->Data = malloc((size_t)Size);
        memcpy(RH->Data,Data,(size_t)Size);
        RH->CompSize = Size;
        RH->CompType = SU_ARCH_COMP_NONE;
        break;
#endif /* HAVE_ZLIB */
#ifdef HAVE_BZLIB
      case SU_ARCH_COMP_BZ :
        /* TO DO */
        RH->Data = malloc((size_t)Size);
        memcpy(RH->Data,Data,(size_t)Size);
        RH->CompSize = Size;
        RH->CompType = SU_ARCH_COMP_NONE;
        break;
#endif /* HAVE_BZLIB */
#ifdef HAVE_MINILZO
      case SU_ARCH_COMP_LZO :
      {
        RH->Data = malloc((size_t)(Size * 1.01 + 1000));
        if(lzo1x_1_compress(Data,Size,RH->Data,(lzo_uintp)&RH->CompSize,SU_AR_wrkmem) != LZO_E_OK)
        {
          free(RH->Data);
          Arch->NbRes--;
          free(RH);
          return false;
        }
        RH->Data = realloc(RH->Data,RH->CompSize);
        break;
      }
#endif /* HAVE_MINILZO */
      default :
        Arch->NbRes--;
        free(RH);
        return false;
    }
  }
  return true;
}

/* Adds a file resource to the archive (Index must be unique, or 0) (Name must be unique, or NULL) (true on success) */
SKYUTILS_API bool SU_AR_AddResFile(SU_PArch Arch,const char FileName[],SU_AR_COMP_TYPE Type,SU_u32 Index,const char *Name)
{
  FILE *fp;
  struct stat st;

  if(Arch == NULL)
    return false;

  fp = fopen(FileName,"rb");
  if(fp == NULL)
    return false;
  fclose(fp);
  if(stat(FileName,&st) != 0)
    return false;
  return SU_AR_AddRes(Arch,(void *)FileName,0,(time_t)0,Type,Index,Name);
}

/* Closes a previous opened/created archive (true on success) */
SKYUTILS_API bool SU_AR_CloseArchive(SU_PArch Arch)
{
  bool res = true;
  SU_u32 i;

  if(Arch == NULL)
    return true;

  if(Arch->Flush)
    res = _SU_AR_Flush(Arch);

  for(i=0;i<Arch->NbRes;i++)
  {
    if(Arch->Resources[i].Name)
      free(Arch->Resources[i].Name);
  }
  if(Arch->Resources != NULL)
    free(Arch->Resources);
  fclose(Arch->fp);
  free(Arch);
  return res;
}

/* Frees a previous returned resource */
SKYUTILS_API void SU_AR_FreeRes(SU_PRes Res)
{
  if(Res == NULL)
    return;

  if(Res->Data != NULL)
    free(Res->Data);
  free(Res);
}

/* Returns supported compression types (as a bit field) */
SKYUTILS_API SU_AR_COMP_TYPE SU_AR_SupportedComps(void)
{
  SU_AR_COMP_TYPE Flags = SU_ARCH_COMP_NONE;
#ifdef HAVE_ZLIB
  /* SetFlag(Flags,SU_ARCH_COMP_Z); Not supported yet */
#endif /* HAVE_ZLIB */
#ifdef HAVE_BZLIB
  /* SetFlag(Flags,SU_ARCH_COMP_BZ); Not supported yet */
#endif /* HAVE_BZLIB */
#ifdef HAVE_MINILZO
  SetFlag(Flags,SU_ARCH_COMP_LZO);
#endif /* HAVE_MINILZO */
  return Flags;
}

/* Returns the number of resources in the archive (0 if error or no resources) */
SKYUTILS_API SU_u32 SU_AR_GetResourcesCount(SU_PArch Arch)
{
  if(Arch == NULL)
    return 0;
  return Arch->NbRes;
}


#endif /* SU_USE_ARCH */

