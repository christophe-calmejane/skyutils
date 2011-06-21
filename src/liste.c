/****************************************************************/
/* Chained list unit                                            */
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


#include <stdio.h>
#include <stdlib.h>

#include "skyutils.h"

#ifndef SU_TRACE_INTERNAL
#undef malloc
#undef calloc
#undef realloc
#undef strdup
#undef free
#endif /* !SU_TRACE_INTERNAL */

SKYUTILS_API SU_PList SU_AddElementTail(SU_PList List,void *Elem)
{
  SU_PList Ptr,Ptr2,El;

  Ptr = List;
  Ptr2 = NULL;
  while(Ptr != NULL)
  {
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  El = (SU_PList) malloc(sizeof(SU_TList));
  El->Next = NULL;
  El->Data = Elem;
  if(List == NULL)
    return El;
  Ptr2->Next = El;
  return List;
}

SKYUTILS_API SU_PList SU_AddElementHead(SU_PList List,void *Elem)
{
  SU_PList El;

  El = (SU_PList) malloc(sizeof(SU_TList));
  El->Next = List;
  El->Data = Elem;
  return El;
}

SKYUTILS_API SU_PList SU_AddElementPos(SU_PList List,int Pos,void *Elem)
{
  SU_PList Ptr;
  int p;

  if((Pos <= 0) || (List == NULL))
    return SU_AddElementHead(List,Elem);
  Ptr = List;
  for(p=0;p<(Pos-1);p++)
  {
    if(Ptr->Next == NULL)
    {
      Ptr->Next = SU_AddElementHead(NULL,Elem);
      return List;
    }
    Ptr = Ptr->Next;
  }
  Ptr->Next = SU_AddElementHead(Ptr->Next,Elem);
  return List;
}

SKYUTILS_API SU_PList SU_DelElementElem(SU_PList List,void *Elem)
{
  SU_PList Ptr,Ptr2,St;

  if(List == NULL)
    return NULL;
  Ptr = List;
  St = List;
  Ptr2 = NULL;
  while(Ptr != NULL)
  {
    if(Ptr->Data == Elem)
    {
      Ptr = SU_DelElementHead(Ptr);
      if(Ptr2 == NULL)
        St = Ptr;
      else
        Ptr2->Next = Ptr;
      if(Ptr == NULL)
        return St;
    }
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  return St;
}

SKYUTILS_API SU_PList SU_DelElementTail(SU_PList List)
{
  SU_PList Ptr,Ptr2;

  if(List == NULL)
    return NULL;
  Ptr = List;
  Ptr2 = NULL;
  while(Ptr->Next != NULL)
  {
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  free(Ptr);
  if(Ptr2 == NULL)
    return NULL;
  Ptr2->Next = NULL;
  return List;
}

SKYUTILS_API SU_PList SU_DelElementHead(SU_PList List)
{
  SU_PList Ptr;

  if(List == NULL)
    return NULL;
  Ptr = List->Next;
  free(List);
  return Ptr;
}

SKYUTILS_API SU_PList SU_DelElementPos(SU_PList List,int Pos)
{
  int p;
  SU_PList Ptr;

  if(List == NULL)
    return NULL;
  if(Pos <= 0)
    return SU_DelElementHead(List);
  Ptr = List;
  for(p=0;p<(Pos-1);p++)
  {
    Ptr = Ptr->Next;
    if(Ptr == NULL)
      return List;
  }
  if(Ptr->Next == NULL)
    return List;
  Ptr->Next = SU_DelElementHead(Ptr->Next);
  return List;
}

SKYUTILS_API void *SU_GetElementTail(SU_PList List)
{
  SU_PList Ptr;

  if(List == NULL)
    return NULL;
  Ptr = List;
  while(Ptr->Next != NULL)
    Ptr = Ptr->Next;
  return Ptr->Data;
}

SKYUTILS_API void *SU_GetElementHead(SU_PList List)
{
  if(List == NULL)
    return NULL;
  return List->Data;
}

SKYUTILS_API void *SU_GetElementPos(SU_PList List,int Pos)
{
  int p;
  SU_PList Ptr;

  if(List == NULL)
    return NULL;
  if(Pos <= 0)
    return SU_GetElementHead(List);
  Ptr = List;
  for(p=0;p<Pos;p++)
  {
    Ptr = Ptr->Next;
    if(Ptr == NULL)
      return NULL;
  }
  return Ptr->Data;
}

SKYUTILS_API void SU_FreeList(SU_PList List)
{
  SU_PList Ptr,Ptr2;

  Ptr = List;
  while(Ptr != NULL)
  {
    Ptr2 = Ptr->Next;
    free(Ptr);
    Ptr = Ptr2;
  }
}

SKYUTILS_API void SU_FreeListElem(SU_PList List)
{
  SU_PList Ptr,Ptr2;

  Ptr = List;
  while(Ptr != NULL)
  {
    Ptr2 = Ptr->Next;
    free(Ptr->Data);
    free(Ptr);
    Ptr = Ptr2;
  }
}

SKYUTILS_API unsigned int SU_ListCount(SU_PList List)
{
  SU_PList Ptr;
  unsigned int c;

  c = 0;
  Ptr = List;
  while(Ptr != NULL)
  {
    c++;
    Ptr = Ptr->Next;
  }
  return c;
}

/* Assocs */


SKYUTILS_API SU_PAssoc SU_AddAssocTail(SU_PAssoc List,void *Left,void *Right)
{
  SU_PAssoc Ptr,Ptr2,El;

  Ptr = List;
  Ptr2 = NULL;
  while(Ptr != NULL)
  {
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  El = (SU_PAssoc) malloc(sizeof(SU_TAssoc));
  El->Next = NULL;
  El->Left = Left;
  El->Right = Right;
  if(List == NULL)
    return El;
  Ptr2->Next = El;
  return List;
}

SKYUTILS_API SU_PAssoc SU_AddAssocHead(SU_PAssoc List,void *Left,void *Right)
{
  SU_PAssoc El;

  El = (SU_PAssoc) malloc(sizeof(SU_TAssoc));
  El->Next = List;
  El->Left = Left;
  El->Right = Right;
  return El;
}

SKYUTILS_API SU_PAssoc SU_AddAssocPos(SU_PAssoc List,int Pos,void *Left,void *Right)
{
  SU_PAssoc Ptr;
  int p;

  if((Pos <= 0) || (List == NULL))
    return SU_AddAssocHead(List,Left,Right);
  Ptr = List;
  for(p=0;p<(Pos-1);p++)
  {
    if(Ptr->Next == NULL)
    {
      Ptr->Next = SU_AddAssocHead(NULL,Left,Right);
      return List;
    }
    Ptr = Ptr->Next;
  }
  Ptr->Next = SU_AddAssocHead(Ptr->Next,Left,Right);
  return List;
}

SKYUTILS_API SU_PAssoc SU_DelAssocLeft(SU_PAssoc List,void *Left)
{
  SU_PAssoc Ptr,Ptr2,St;

  if(List == NULL)
    return NULL;
  Ptr = List;
  St = List;
  Ptr2 = NULL;
  while(Ptr != NULL)
  {
    if(Ptr->Left == Left)
    {
      Ptr = SU_DelAssocHead(Ptr);
      if(Ptr2 == NULL)
        St = Ptr;
      else
        Ptr2->Next = Ptr;
      if(Ptr == NULL)
        return St;
    }
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  return St;
}

SKYUTILS_API SU_PAssoc SU_DelAssocTail(SU_PAssoc List)
{
  SU_PAssoc Ptr,Ptr2;

  if(List == NULL)
    return NULL;
  Ptr = List;
  Ptr2 = NULL;
  while(Ptr->Next != NULL)
  {
    Ptr2 = Ptr;
    Ptr = Ptr->Next;
  }
  free(Ptr);
  if(Ptr2 == NULL)
    return NULL;
  Ptr2->Next = NULL;
  return List;
}

SKYUTILS_API SU_PAssoc SU_DelAssocHead(SU_PAssoc List)
{
  SU_PAssoc Ptr;

  if(List == NULL)
    return NULL;
  Ptr = List->Next;
  free(List);
  return Ptr;
}

SKYUTILS_API SU_PAssoc SU_DelAssocPos(SU_PAssoc List,int Pos)
{
  int p;
  SU_PAssoc Ptr;

  if(List == NULL)
    return NULL;
  if(Pos <= 0)
    return SU_DelAssocHead(List);
  Ptr = List;
  for(p=0;p<(Pos-1);p++)
  {
    Ptr = Ptr->Next;
    if(Ptr == NULL)
      return List;
  }
  if(Ptr->Next == NULL)
    return List;
  Ptr->Next = SU_DelAssocHead(Ptr->Next);
  return List;
}

SKYUTILS_API bool SU_GetAssocTail(SU_PAssoc List,void **Left,void **Right)
{
  SU_PAssoc Ptr;

  if(List == NULL)
    return false;
  Ptr = List;
  while(Ptr->Next != NULL)
    Ptr = Ptr->Next;
  *Left = Ptr->Left;
  *Right = Ptr->Right;
  return true;
}

SKYUTILS_API bool SU_GetAssocHead(SU_PAssoc List,void **Left,void **Right)
{
  if(List == NULL)
    return false;

  *Left = List->Left;
  *Right = List->Right;
  return true;
}

SKYUTILS_API bool SU_GetAssocPos(SU_PAssoc List,int Pos,void **Left,void **Right)
{
  int p;
  SU_PAssoc Ptr;

  if(List == NULL)
    return false;
  if(Pos <= 0)
    return SU_GetAssocHead(List,Left,Right);
  Ptr = List;
  for(p=0;p<Pos;p++)
  {
    Ptr = Ptr->Next;
    if(Ptr == NULL)
      return false;
  }
  *Left = Ptr->Left;
  *Right = Ptr->Right;
  return true;
}

SKYUTILS_API bool SU_GetAssocLeft(SU_PAssoc List,void *Left,void **Right)
{
  SU_PAssoc Ptr;

  Ptr = List;
  while(Ptr != NULL)
  {
    if(Ptr->Left == Left)
    {
      *Right = Ptr->Right;
      return true;
    }
    Ptr = Ptr->Next;
  }
  return false;
}

SKYUTILS_API void SU_FreeAssoc(SU_PAssoc List)
{
  SU_PAssoc Ptr,Ptr2;

  Ptr = List;
  while(Ptr != NULL)
  {
    Ptr2 = Ptr->Next;
    free(Ptr);
    Ptr = Ptr2;
  }
}

SKYUTILS_API void SU_FreeAssocAssoc(SU_PAssoc List)
{
  SU_PAssoc Ptr,Ptr2;

  Ptr = List;
  while(Ptr != NULL)
  {
    Ptr2 = Ptr->Next;
    free(Ptr->Left);
    free(Ptr->Right);
    free(Ptr);
    Ptr = Ptr2;
  }
}

SKYUTILS_API unsigned int SU_AssocCount(SU_PAssoc List)
{
  SU_PAssoc Ptr;
  unsigned int c;

  c = 0;
  Ptr = List;
  while(Ptr != NULL)
  {
    c++;
    Ptr = Ptr->Next;
  }
  return c;
}

