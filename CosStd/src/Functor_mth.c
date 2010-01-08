/*
 o---------------------------------------------------------------------o
 |
 | COS Functor (Method Expression)
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
 | $Id: Functor_mth.c,v 1.6 2010/01/07 14:53:52 ldeniau Exp $
 |
*/

#include <cos/Array.h>
#include <cos/Functor.h>
#include <cos/gen/accessor.h>
#include <cos/gen/functor.h>
#include <cos/gen/message.h>
#include <cos/gen/object.h>
#include <cos/gen/value.h>

#include <assert.h>
#include <string.h>

#include "Functor_utl.h"

// ----- Functor method

defclass(MthExpr, Functor)
  SEL sel;
endclass

defclass(MthExpr1, MthExpr)
  OBJ   arg[1];
  void *args;
endclass

defclass(MthExpr2, MthExpr)
  OBJ   arg[2];
  void *args;
endclass

defclass(MthExpr3, MthExpr)
  OBJ   arg[3];
  void *args;
endclass

defclass(MthExpr4, MthExpr)
  OBJ   arg[4];
  void *args;
endclass

defclass(MthExpr5, MthExpr)
  OBJ   arg[5];
  void *args;
endclass

makclass(MthExpr , Functor);
makclass(MthExpr1, MthExpr);
makclass(MthExpr2, MthExpr);
makclass(MthExpr3, MthExpr);
makclass(MthExpr4, MthExpr);
makclass(MthExpr5, MthExpr);

// ----- partially evaluated method

defclass(MthPart1, MthExpr1) endclass
defclass(MthPart2, MthExpr2) endclass
defclass(MthPart3, MthExpr3) endclass
defclass(MthPart4, MthExpr4) endclass
defclass(MthPart5, MthExpr5) endclass

makclass(MthPart1, MthExpr1);
makclass(MthPart2, MthExpr2);
makclass(MthPart3, MthExpr3);
makclass(MthPart4, MthExpr4);
makclass(MthPart5, MthExpr5);

// -----

useclass(ExBadAlloc);

// ----- helper

#define aMthExpr(...) ( (OBJ)atMthExpr(__VA_ARGS__) )

#define atMthExpr(S,A,...) \
  atMthExprN(COS_PP_NARG(__VA_ARGS__), S, A, __VA_ARGS__)

#define atMthExprN(N,S,A,...) \
  COS_PP_CAT(MthExpr_init,N)( &(struct COS_PP_CAT(MthExpr,N)) { \
    {{{ cos_object_auto(COS_PP_CAT(MthExpr,N)) }, 0 }, S }, \
    {__VA_ARGS__}, A }, __FILE__, __LINE__)

// ----- utils

static void*
cloneArgs(SEL sel, char *_args)
{
  U32   size = sel->argsize;
  char *args = 0;
  
  // generic has monomorphic arguments
  if (size) {
    U32 narg = COS_GEN_NARG(sel) - COS_GEN_RNK(sel);
    
    // sanity check
    assert(args);
    assert(narg);
  
    args = malloc(size);
    if (!args) THROW(ExBadAlloc);
    
    // all arguments are OBJects
    if (COS_GEN_OARG(sel)) {
      OBJ *dst = (OBJ*)   args;
      OBJ *src = (OBJ*)  _args;
      OBJ *end = (OBJ*) (_args + size);
      
      // sanity check
      assert(size >= narg*sizeof(OBJ));
      
      while (src < end) {
        *dst++ = *src ? gretain(*src) : 0;
         src++;
      }
    }
    
    // some/all arguments aren't OBJect
    else {
      // sanity check
      assert(sel->arginfo[0].offset == 0);
      
      memcpy(args, _args, size);
    
      for (U32 idx = 0; idx < narg; idx++)
        if (!sel->arginfo[idx].size) { // convention for objects
          OBJ *obj = (OBJ*) (args + sel->arginfo[idx].offset);
          if (*obj) *obj = gretain(*obj);
        }
    }
  }

  return args;
}

static void
deleteArgs(SEL sel, char *args)
{
  // generic has monomorphic arguments
  if (args) {
    U32 size = sel->argsize;
    U32 narg = COS_GEN_NARG(sel) - COS_GEN_RNK(sel);
    
    // sanity check
    assert(narg);

    // all arguments are OBJects
    if (COS_GEN_OARG(sel)) {
      OBJ *obj = (OBJ*)  args;
      OBJ *end = (OBJ*) (args + size);
      
      // sanity check
      assert(size >= narg*sizeof(OBJ));
      
      while (obj < end) {
        if (*obj) grelease(*obj);
        ++obj;
      }
    }

    // some/all arguments aren't OBJect
    else {
      // sanity check
      assert(sel->arginfo[0].offset == 0);
      
      for (U32 idx = 0; idx < narg; idx++)
        if (!sel->arginfo[idx].size) { // convention for objects
          OBJ *obj = (OBJ*) (args + sel->arginfo[idx].offset);
          if (*obj) grelease(*obj);
        }
    }
    
    free(args);
  }
}

// ----- initializer

#undef  DEFFUNC
#define DEFFUNC(N) \
\
static struct Functor* COS_PP_CAT(MthExpr_init,N) \
(struct COS_PP_CAT(MthExpr,N) *mth, STR file, int line) \
{ \
  test_assert(COS_GEN_ORET(mth->MthExpr.sel), \
              "invalid method expression, returned value must be an object"); \
  test_assert(!COS_GEN_VARG(mth->MthExpr.sel), \
              "invalid method expression, variadic generic not supported"); \
\
  mth->MthExpr.Functor.msk = Functor_getMask(mth->arg, N, file, line); \
\
  if (getPar(mth->MthExpr.Functor.msk)) \
    mth->MthExpr.Functor.Expression.Object.id = \
      cos_class_id(classref(COS_PP_CAT(MthPart,N))); \
\
  return &mth->MthExpr.Functor; \
}

DEFFUNC(1)
DEFFUNC(2)
DEFFUNC(3)
DEFFUNC(4)
DEFFUNC(5)

// ----- ctors

// TODO

// ----- copy

#undef  DEFMETHOD
#define DEFMETHOD(N) \
\
defmethod(OBJ, ginitWith, COS_PP_CAT(MthExpr,N), COS_PP_CAT(MthExpr,N)) \
  self->MthExpr.Functor.msk = self2->MthExpr.Functor.msk; \
  self->MthExpr.sel = self2->MthExpr.sel; \
  self->args = 0; \
\
  for (int i = 0; i < N; i++) \
    self->arg[i] = isIdx(self2->MthExpr.Functor.msk, i) \
                   ? self2->arg[i] : gretain(self2->arg[i]); \
\
  self->args = cloneArgs(self2->MthExpr.sel, self2->args); \
\
  retmethod(_1); \
endmethod

DEFMETHOD(1)
DEFMETHOD(2)
DEFMETHOD(3)
DEFMETHOD(4)
DEFMETHOD(5)

// ----- dtors

#undef  DEFMETHOD
#define DEFMETHOD(N) \
\
defmethod(OBJ, gdeinit, COS_PP_CAT(MthExpr,N)) \
  for (int i = 0; i < N; i++) \
    if (!isIdx(self->MthExpr.Functor.msk, i)) \
      grelease(self->arg[i]); \
\
  deleteArgs(self->MthExpr.sel, self->args); \
\
  retmethod(_1); \
endmethod

DEFMETHOD(1)
DEFMETHOD(2)
DEFMETHOD(3)
DEFMETHOD(4)
DEFMETHOD(5)

// ----- str

defmethod(STR, gstr, MthExpr)
  retmethod(self->sel->str);
endmethod

// ---- eval (stack-like environment)

defmethod(OBJ, gevalEnv, MthExpr1, Array)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);

  U32 aid0 = cos_object_id(arg0);
  IMP1 mth = cos_method_fastLookup1(sel, aid0);

  mth(sel, arg0, self->args, _ret);
endmethod

defmethod(OBJ, gevalEnv, MthPart1, Array)
  U32  msk = self->MthExpr1.MthExpr.Functor.msk;
  SEL  sel = self->MthExpr1.MthExpr.sel;
  OBJ *arg = self->MthExpr1.arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);

  // partial evaluation
  retmethod( gautoDelete(aMthExpr(sel, arg, arg0)) );
endmethod

defmethod(OBJ, gevalEnv, MthExpr2, Array)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);
  OBJ arg1 = getArg(1, msk, arg, var, size, _2);

  U32 aid0 = cos_object_id(arg0);
  U32 aid1 = cos_object_id(arg1);
  IMP2 mth = cos_method_fastLookup2(sel, aid0, aid1);

  mth(sel, arg0, arg1, self->args, _ret);
endmethod

defmethod(OBJ, gevalEnv, MthPart2, Array)
  U32  msk = self->MthExpr2.MthExpr.Functor.msk;
  SEL  sel = self->MthExpr2.MthExpr.sel;
  OBJ *arg = self->MthExpr2.arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);
  OBJ arg1 = getArg(1, msk, arg, var, size, _2);

  // partial evaluation
  retmethod( gautoDelete(aMthExpr(sel, arg, arg0, arg1)) );
endmethod

defmethod(OBJ, gevalEnv, MthExpr3, Array)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);
  OBJ arg1 = getArg(1, msk, arg, var, size, _2);
  OBJ arg2 = getArg(2, msk, arg, var, size, _2);

  U32 aid0 = cos_object_id(arg0);
  U32 aid1 = cos_object_id(arg1);
  U32 aid2 = cos_object_id(arg2);
  IMP3 mth = cos_method_fastLookup3(sel, aid0, aid1, aid2);

  mth(sel, arg0, arg1, arg2, self->args, _ret);
endmethod

defmethod(OBJ, gevalEnv, MthPart3, Array)
  U32  msk = self->MthExpr3.MthExpr.Functor.msk;
  SEL  sel = self->MthExpr3.MthExpr.sel;
  OBJ *arg = self->MthExpr3.arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);
  OBJ arg1 = getArg(1, msk, arg, var, size, _2);
  OBJ arg2 = getArg(2, msk, arg, var, size, _2);

  // partial evaluation
  retmethod( gautoDelete(aMthExpr(sel, arg, arg0, arg1, arg2)) );
endmethod

defmethod(OBJ, gevalEnv, MthExpr4, Array)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);
  OBJ arg1 = getArg(1, msk, arg, var, size, _2);
  OBJ arg2 = getArg(2, msk, arg, var, size, _2);
  OBJ arg3 = getArg(3, msk, arg, var, size, _2);

  U32 aid0 = cos_object_id(arg0);
  U32 aid1 = cos_object_id(arg1);
  U32 aid2 = cos_object_id(arg2);
  U32 aid3 = cos_object_id(arg3);
  IMP4 mth = cos_method_fastLookup4(sel, aid0, aid1, aid2, aid3);

  mth(sel, arg0, arg1, arg2, arg3, self->args, _ret);
endmethod

defmethod(OBJ, gevalEnv, MthPart4, Array)
  U32  msk = self->MthExpr4.MthExpr.Functor.msk;
  SEL  sel = self->MthExpr4.MthExpr.sel;
  OBJ *arg = self->MthExpr4.arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);
  OBJ arg1 = getArg(1, msk, arg, var, size, _2);
  OBJ arg2 = getArg(2, msk, arg, var, size, _2);
  OBJ arg3 = getArg(3, msk, arg, var, size, _2);

  // partial evaluation
  retmethod( gautoDelete(aMthExpr(sel, arg, arg0, arg1, arg2, arg3)) );
endmethod

defmethod(OBJ, gevalEnv, MthExpr5, Array)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);
  OBJ arg1 = getArg(1, msk, arg, var, size, _2);
  OBJ arg2 = getArg(2, msk, arg, var, size, _2);
  OBJ arg3 = getArg(3, msk, arg, var, size, _2);
  OBJ arg4 = getArg(4, msk, arg, var, size, _2);

  U32 aid0 = cos_object_id(arg0);
  U32 aid1 = cos_object_id(arg1);
  U32 aid2 = cos_object_id(arg2);
  U32 aid3 = cos_object_id(arg3);
  U32 aid4 = cos_object_id(arg4);
  IMP5 mth = cos_method_fastLookup5(sel, aid0, aid1, aid2, aid3, aid4);

  mth(sel, arg0, arg1, arg2, arg3, arg4, self->args, _ret);
endmethod

defmethod(OBJ, gevalEnv, MthPart5, Array)
  U32  msk = self->MthExpr5.MthExpr.Functor.msk;
  SEL  sel = self->MthExpr5.MthExpr.sel;
  OBJ *arg = self->MthExpr5.arg;
  OBJ *var = self2->object;
  U32 size = self2->size;

  OBJ arg0 = getArg(0, msk, arg, var, size, _2);
  OBJ arg1 = getArg(1, msk, arg, var, size, _2);
  OBJ arg2 = getArg(2, msk, arg, var, size, _2);
  OBJ arg3 = getArg(3, msk, arg, var, size, _2);
  OBJ arg4 = getArg(4, msk, arg, var, size, _2);

  // partial evaluation
  retmethod( gautoDelete(aMthExpr(sel, arg, arg0, arg1, arg2, arg3, arg4)) );
endmethod

// ---- eval (generic environement)

defmethod(OBJ, gevalEnv, MthExpr1, Container)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;

  OBJ arg0 = getArgVar(0, msk, arg, _2);

  U32 aid0 = cos_object_id(arg0);
  IMP1 mth = cos_method_fastLookup1(sel, aid0);

  mth(sel, arg0, self->args, _ret);
endmethod

defmethod(OBJ, gevalEnv, MthPart1, Container)
  U32  msk = self->MthExpr1.MthExpr.Functor.msk;
  SEL  sel = self->MthExpr1.MthExpr.sel;
  OBJ *arg = self->MthExpr1.arg;

  OBJ arg0 = getArgVar(0, msk, arg, _2);

  // partial evaluation
  retmethod( gautoDelete(aMthExpr(sel, arg, arg0)) );
endmethod

defmethod(OBJ, gevalEnv, MthExpr2, Container)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;

  OBJ arg0 = getArgVar(0, msk, arg, _2);
  OBJ arg1 = getArgVar(1, msk, arg, _2);

  U32 aid0 = cos_object_id(arg0);
  U32 aid1 = cos_object_id(arg1);
  IMP2 mth = cos_method_fastLookup2(sel, aid0, aid1);

  mth(sel, arg0, arg1, self->args, _ret);
endmethod

defmethod(OBJ, gevalEnv, MthPart2, Container)
  U32  msk = self->MthExpr2.MthExpr.Functor.msk;
  SEL  sel = self->MthExpr2.MthExpr.sel;
  OBJ *arg = self->MthExpr2.arg;

  OBJ arg0 = getArgVar(0, msk, arg, _2);
  OBJ arg1 = getArgVar(1, msk, arg, _2);

  // partial evaluation
  retmethod( gautoDelete(aMthExpr(sel, arg, arg0, arg1)) );
endmethod

defmethod(OBJ, gevalEnv, MthExpr3, Container)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;

  OBJ arg0 = getArgVar(0, msk, arg, _2);
  OBJ arg1 = getArgVar(1, msk, arg, _2);
  OBJ arg2 = getArgVar(2, msk, arg, _2);

  U32 aid0 = cos_object_id(arg0);
  U32 aid1 = cos_object_id(arg1);
  U32 aid2 = cos_object_id(arg2);
  IMP3 mth = cos_method_fastLookup3(sel, aid0, aid1, aid2);

  mth(sel, arg0, arg1, arg2, self->args, _ret);
endmethod

defmethod(OBJ, gevalEnv, MthPart3, Container)
  U32  msk = self->MthExpr3.MthExpr.Functor.msk;
  SEL  sel = self->MthExpr3.MthExpr.sel;
  OBJ *arg = self->MthExpr3.arg;

  OBJ arg0 = getArgVar(0, msk, arg, _2);
  OBJ arg1 = getArgVar(1, msk, arg, _2);
  OBJ arg2 = getArgVar(2, msk, arg, _2);

  // partial evaluation
  retmethod( gautoDelete(aMthExpr(sel, arg, arg0, arg1, arg2)) );
endmethod

defmethod(OBJ, gevalEnv, MthExpr4, Container)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;

  OBJ arg0 = getArgVar(0, msk, arg, _2);
  OBJ arg1 = getArgVar(1, msk, arg, _2);
  OBJ arg2 = getArgVar(2, msk, arg, _2);
  OBJ arg3 = getArgVar(3, msk, arg, _2);

  if (getPar(msk)) // partial evaluation
    retmethod( gautoDelete(aMthExpr(sel, arg, arg0, arg1, arg2, arg3)) );

  U32 aid0 = cos_object_id(arg0);
  U32 aid1 = cos_object_id(arg1);
  U32 aid2 = cos_object_id(arg2);
  U32 aid3 = cos_object_id(arg3);
  IMP4 mth = cos_method_fastLookup4(sel, aid0, aid1, aid2, aid3);

  mth(sel, arg0, arg1, arg2, arg3, self->args, _ret);
endmethod

defmethod(OBJ, gevalEnv, MthExpr5, Container)
  U32  msk = self->MthExpr.Functor.msk;
  SEL  sel = self->MthExpr.sel;
  OBJ *arg = self->arg;

  OBJ arg0 = getArgVar(0, msk, arg, _2);
  OBJ arg1 = getArgVar(1, msk, arg, _2);
  OBJ arg2 = getArgVar(2, msk, arg, _2);
  OBJ arg3 = getArgVar(3, msk, arg, _2);
  OBJ arg4 = getArgVar(4, msk, arg, _2);

  if (getPar(msk)) // partial evaluation
    retmethod( gautoDelete(aMthExpr(sel, arg, arg0, arg1, arg2, arg3, arg4)) );

  U32 aid0 = cos_object_id(arg0);
  U32 aid1 = cos_object_id(arg1);
  U32 aid2 = cos_object_id(arg2);
  U32 aid3 = cos_object_id(arg3);
  U32 aid4 = cos_object_id(arg4);
  IMP5 mth = cos_method_fastLookup5(sel, aid0, aid1, aid2, aid3, aid4);

  mth(sel, arg0, arg1, arg2, arg3, arg4, self->args, _ret);
endmethod

// ---- unrecognizedMessage (ctors)

// ---- 1

defmethod(void, gunrecognizedMessage1, Expression)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1));
endmethod

// ---- 2

defmethod(void, gunrecognizedMessage2, Expression, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2));
endmethod

defmethod(void, gunrecognizedMessage2, Object, Expression)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2));
endmethod

// ---- 3

defmethod(void, gunrecognizedMessage3, Expression, Object, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3));
endmethod

defmethod(void, gunrecognizedMessage3, Object, Expression, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3));
endmethod

defmethod(void, gunrecognizedMessage3, Object, Object, Expression)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3));
endmethod

// ---- 4

defmethod(void, gunrecognizedMessage4, Expression, Object, Object, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3, _4));
endmethod

defmethod(void, gunrecognizedMessage4, Object, Expression, Object, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3, _4));
endmethod

defmethod(void, gunrecognizedMessage4, Object, Object, Expression, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3, _4));
endmethod

defmethod(void, gunrecognizedMessage4, Object, Object, Object, Expression)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3, _4));
endmethod

// ---- 5

defmethod(void, gunrecognizedMessage5, Expression, Object, Object, Object, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3, _4, _5));
endmethod

defmethod(void, gunrecognizedMessage5, Object, Expression, Object, Object, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3, _4, _5));
endmethod

defmethod(void, gunrecognizedMessage5, Object, Object, Expression, Object, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3, _4, _5));
endmethod

defmethod(void, gunrecognizedMessage5, Object, Object, Object, Expression, Object)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3, _4, _5));
endmethod

defmethod(void, gunrecognizedMessage5, Object, Object, Object, Object, Expression)
  *(OBJ*)_ret = gautoDelete(aMthExpr(_sel, _arg, _1, _2, _3, _4, _5));
endmethod
