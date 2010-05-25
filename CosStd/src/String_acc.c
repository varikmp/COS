/*
 o---------------------------------------------------------------------o
 |
 | COS String - accessors
 |
 o---------------------------------------------------------------------o
 |
 | C Object System
 |
 | Copyright (c) 2006+ Laurent Deniau, laurent.deniau@cern.ch
 |
 | For more information, see:
 | http://cern.ch/laurent.deniau/cos.html
 |
 o---------------------------------------------------------------------o
 |
 | This file is part of the C Object System framework.
 |
 | The C Object System is free software; you can redistribute it and/or
 | modify it under the terms of the GNU Lesser General Public License
 | as published by the Free Software Foundation; either version 3 of
 | the License, or (at your option) any later version.
 |
 | The C Object System is distributed in the hope that it will be
 | useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 | of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 |
 | See <http://www.gnu.org/licenses> for more details.
 |
 o---------------------------------------------------------------------o
 |
 | $Id: String_acc.c,v 1.11 2010/05/25 15:33:39 ldeniau Exp $
 |
*/

#include <cos/String.h>
#include <cos/IntVector.h>
#include <cos/Number.h>
#include <cos/Slice.h>

#include <cos/gen/accessor.h>
#include <cos/gen/collection.h>
#include <cos/gen/object.h>
#include <cos/gen/sequence.h>
#include <cos/gen/value.h>

// -----

useclass(String);

// --- first, last

defmethod(OBJ,  glast, String)
  retmethod( self->size
           ? gautoDelete(aChar(self->value[self->size-1]))
           : Nil );
endmethod

defmethod(OBJ, gfirst, String)
  retmethod( self->size ? gautoDelete(aChar(self->value[0])) : Nil );
endmethod

// ----- getters (index, slice, range, intvector)

defmethod(OBJ, ggetAtIdx, String, (I32)idx)
  U32 i = Range_index(idx, self->size);
  test_assert( i < self->size, "index out of range" );

  retmethod( gautoDelete(aChar(self->value[i])) );
endmethod

defmethod(OBJ, ggetAt, String, Int)
  U32 i = Range_index(self2->value, self->size);
  test_assert( i < self->size, "index out of range" );

  retmethod( gautoDelete(aChar(self->value[i])) );
endmethod

defmethod(OBJ, ggetAt, String, Slice)
  retmethod( gautoDelete(gnewWith2(String,_1,_2)) );
endmethod

defmethod(OBJ, ggetAt, String, Range)
  retmethod( gautoDelete(gnewWith2(String,_1,_2)) );
endmethod

defmethod(OBJ, ggetAt, String, IntVector)
  retmethod( gautoDelete(gnewWith2(String,_1,_2)) );
endmethod

// ----- object setters (index, slice, range, intvector)

defmethod(OBJ, gputAtIdx, String, (I32)idx, Object)
  U32 i = Range_index(idx, self->size);
  test_assert( i < self->size, "index out of range" );

  self->value[i] = (U32)gchr(_2);
    
  retmethod(_1);
endmethod

defmethod(OBJ, gputAt, String, Int, Object)
  U32 i = Range_index(self2->value, self->size);
  test_assert( i < self->size, "index out of range" );

  self->value[i] = (U32)gchr(_3);
    
  retmethod(_1);
endmethod

defmethod(OBJ, gputAt, String, Slice, Object)
  PRE
    test_assert( Slice_first(self2) < self->size &&
                 Slice_last (self2) < self->size, "slice out of range" );

  BODY
    U32 dst_n = Slice_size  (self2);
    I32 dst_s = Slice_stride(self2);
    U8* dst   = Slice_start (self2) + self->value;
    U8* end   = dst + dst_n;
    U8  val   = (U32)gchr(_3);

    while (dst != end) {
      *dst = val;
      dst += dst_s;
    }
    
    retmethod(_1);
endmethod

defmethod(OBJ, gputAt, String, Range, Object)
  struct Range *range = Range_normalize(Range_copy(atRange(0),self2),self->size);
  struct Slice *slice = Slice_fromRange(atSlice(0),range);

  retmethod( gputAt(_1,(OBJ)slice,_3) );
endmethod

defmethod(OBJ, gputAt, String, IntVector, Object)
  U32  dst_n = self->size;
  U8*  dst   = self->value;
  U32  idx_n = self2->size;
  I32  idx_s = self2->stride;
  I32 *idx   = self2->value;
  I32 *end   = idx + idx_s*idx_n;
  U8   val   = (U32)gchr(_3);

  while (idx != end) {
    U32 i = Range_index(*idx, dst_n);
    test_assert( i < dst_n, "index out of range" );
    dst[i] = val;
    idx += idx_s;
  }
  
  retmethod(_1);
endmethod

// ----- string setters (slice, range, intvector)

defmethod(OBJ, gputAt, String, Slice, String)
  PRE
    test_assert( Slice_first(self2) < self->size &&
                 Slice_last (self2) < self->size, "slice out of range" );
    test_assert( Slice_size (self2) <= self3->size, "source string is too small" );

  BODY
    U32 dst_n = Slice_size  (self2);
    I32 dst_s = Slice_stride(self2);
    U8* dst   = Slice_start (self2) + self->value;
    U8* src   = self3->value;
    U8* end   = dst + dst_n;

    while (dst != end) {
      *dst = *src++;
      dst += dst_s;
    }
    
    retmethod(_1);
endmethod

defmethod(OBJ, gputAt, String, IntVector, String)
  PRE
    test_assert( self2->size <= self3->size, "source string is too small" );

  BODY
    U32  dst_n = self->size;
    U8*  dst   = self->value;
    U32  idx_n = self2->size;
    I32  idx_s = self2->stride;
    I32 *idx   = self2->value;
    U8*  src   = self3->value;
    I32 *end   = idx + idx_s*idx_n;

    while (idx != end) {
      U32 i = Range_index(*idx, dst_n);
      test_assert( i < dst_n, "index out of range" );
      dst[i] = *src++;
      idx += idx_s;
    }
    
    retmethod(_1);
endmethod

// ----- value getter

defmethod(I32, gchrAt, String, Int)
  U32 i;

  PRE
    i = Range_index(self2->value, self->size);
    test_assert( i < self->size, "index out of range" );

  BODY
    if (!COS_CONTRACT) // no PRE
      i = Range_index(self2->value, self->size);
      
    retmethod( self->value[i] );
endmethod

// ----- value setter

defmethod(OBJ, gputAt, String, Int, Char)
  U32 i;
  
  PRE
    i = Range_index(self2->value, self->size);
    test_assert( i < self->size, "index out of range" );

  BODY
    if (!COS_CONTRACT) // no PRE
      i = Range_index(self2->value, self->size);
      
    self->value[i] = self3->Int.value;
    
    retmethod(_1);
endmethod

