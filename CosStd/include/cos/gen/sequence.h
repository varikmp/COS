#ifndef COS_GEN_SEQUENCE_H
#define COS_GEN_SEQUENCE_H

/*
 o---------------------------------------------------------------------o
 |
 | COS generics for sequences
 |
 o---------------------------------------------------------------------o
 |
 | C Object System
 |
 | Copyright (c) 2006+ Laurent Deniau, laurent.deniau*cern.ch
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
 | $Id: sequence.h,v 1.9 2010/05/25 15:33:39 ldeniau Exp $
 |
*/

#include <cos/Object.h>

// in place
defgeneric(OBJ, greverse, _1);
defgeneric(OBJ, gpermute, _1, idx);

// concat
defgeneric(OBJ, gconcat , _1, _2);
defgeneric(OBJ, gconcat3, _1, _2, _3);
defgeneric(OBJ, gconcat4, _1, _2, _3, _4);
defgeneric(OBJ, gconcatn, _1);

// zip
defgeneric(OBJ, gzip , _1, _2);
defgeneric(OBJ, gzip3, _1, _2, _3);
defgeneric(OBJ, gzip4, _1, _2, _3, _4);
defgeneric(OBJ, gzipn, _1);

// array-like accessor
defgeneric(I32, gindexOf, _1, fun); // return index or -1 if not found

// list-like accessor
defgeneric(OBJ, ginsertAt , _1, at, what);
defgeneric(OBJ, gremoveAt , _1, at);

// stack-like accessors
defgeneric(OBJ, gpush     , _1, what);
defgeneric(OBJ, gtop      , _1);
defgeneric(OBJ, gpop      , _1); // return _1

// dequeue-like accessors
defgeneric(OBJ, gpushFront, _1, what);
defgeneric(OBJ, gpushBack , _1, what);
defgeneric(OBJ, gpopFront , _1);
defgeneric(OBJ, gpopBack  , _1);
defgeneric(OBJ, gfront    , _1);
defgeneric(OBJ, gback     , _1);

// seq-like accessor
defgeneric(OBJ, gprepend  , _1, what);
defgeneric(OBJ, gappend   , _1, what);
defgeneric(OBJ, gchop     , _1, what);
defgeneric(OBJ, gdrop     , _1, num); // back if num<0
defgeneric(OBJ, gdropFirst, _1);
defgeneric(OBJ, gdropLast , _1);
defgeneric(OBJ, gfirst    , _1);
defgeneric(OBJ, glast     , _1);

// adjust sequence (e.g. capacity to size)
defgeneric(OBJ, gadjust   , _1);

// englarge sequence memory (e.g. front if by<0)
defgeneric(OBJ, genlarge  , _1, by);

#endif // COS_GEN_SEQUENCE_H
