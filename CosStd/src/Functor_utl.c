/*
 o---------------------------------------------------------------------o
 |
 | COS Functor (Function Expression)
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
 | $Id: Functor_utl.c,v 1.7 2010/01/07 14:53:52 ldeniau Exp $
 |
*/

#include <cos/Functor.h>
#include <cos/gen/accessor.h>
#include <cos/gen/functor.h>

#include "Functor_utl.h"

// -----

static inline U32
getFunIdx(OBJ arg) // unsafe, use with isIdx()
{
  return STATIC_CAST(struct FunArg*, arg)->idx;
}

static inline OBJ
getFunVar(OBJ arg) // unsafe, use with isVar()
{
  return STATIC_CAST(struct FunVar*, arg)->var;
}

static inline OBJ
getFunFun(OBJ arg) // unsafe
{
  return STATIC_CAST(struct FunLzy*, arg)->fun;
}

static inline U32
getFunCnt(OBJ arg) // unsafe
{
  return STATIC_CAST(struct FunLzy*, arg)->cnt;
}

static inline U32
getFunMsk(OBJ arg) // unsafe
{
  return STATIC_CAST(struct Functor*, arg)->msk;
}

static inline U32
getFunPar(OBJ arg, OBJ *var)
{
  U32 cnt = 0;
  
  do {
    cnt += getFunCnt(arg);
    arg  = getFunFun(arg);
  } while (cos_object_isa(arg, classref(FunLzy)));

  *var = arg;

  return cnt;
}

// -----

static inline void
setPar(U32 *msk, U32 par)
{
  *msk = (*msk & ~PAR_MASK) | par;
}

static inline void
setFunPar(OBJ arg, OBJ var, U32 cnt)
{
  struct FunLzy* lzy = STATIC_CAST(struct FunLzy*, arg);
  
  lzy->fun = var;
  lzy->cnt = cnt;
}

// -----

U32
Functor_getMask(OBJ arg[], U32 n, STR file, int line)
{
  U32 idx, msk = 0;

  for (idx = 0; idx < n; idx++) {

    test_assert( arg[idx], "invalid (null) argument", file, line );

          // placeholder
    if (cos_object_isKindOf(arg[idx], classref(PlaceHolder))) {
              // environment index 
        if (cos_object_isa(arg[idx], classref(FunArg))) {
          setIdx(&msk, idx);
          arg[idx] = (OBJ)(size_t)getFunIdx(arg[idx]);
        }
    
        else  // environment key
        if (cos_object_isa(arg[idx], classref(FunVar))) {
          setVar(&msk, idx);
          arg[idx] = getFunVar(arg[idx]);    
        }

        else  // lazy expression
        if (cos_object_isa(arg[idx], classref(FunLzy))) {
          OBJ var = Nil;
          U32 cnt = getFunPar(arg[idx], &var);
      
          if (cos_object_isKindOf(var, classref(PlaceHolder))) {
            setFunPar(arg[idx], var, cnt);
            U32 par = cnt*PAR_UNIT;
            if (par > msk) setPar(&msk, par);
          } else { // lazyness is idempotent on non-placeholder
            setArg(&msk, idx);
            arg[idx] = var;
          }
        }
        
        else
          test_assert( 0, "invalid placeholder", file, line );
    }
    
    else  // functor
    if (cos_object_isKindOf(arg[idx], classref(Functor))) {
      U32 par = getPar(getFunMsk(arg[idx]));
      if (par > msk + PAR_UNIT) setPar(&msk, par - PAR_UNIT);
    }
          
    else  // argument (free variable)
      setArg(&msk, idx);
  }

  return msk;
}
