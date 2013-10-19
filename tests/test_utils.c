/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                test_utils.c                                |
 |                                                                            |
 |                               test utilities                               |
 |                                                                            |
 *----------------------------------------------------------------------------*/
/*
Copyright (c) 2013, Walter Tross <waltertross at gmail dot com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "test_utils.h"

typedef          long long  LLONG;
typedef unsigned long long ULLONG;
typedef unsigned long      ULONG;
typedef unsigned int       UINT;

/*---------------------------------------------------------------------------*/

/* "Some portable very-long-period random number generators"
 * George Marsaglia and Arif Zaman
 * Computers in Physics, Vol. 8, No. 1, Jan/Feb 1994
 * With fixes by Walter Tross
 */

static uint32_t mz_x = 521288629, mz_y = 362436069, mz_z = 16163801;
static uint32_t mz_c = 1, mz_n = 1131199209;

static uint32_t mzran13()
{
   uint32_t mz_s;
   if (mz_y > mz_x + mz_c) {
      mz_s = mz_y - (mz_x + mz_c);
      mz_c = 0;
   } else {
      mz_s = mz_y - (mz_x + mz_c) - 18;
      mz_c = 1;
   }
   mz_x = mz_y;
   mz_y = mz_z;
   return (mz_z = mz_s) + (mz_n - 69069 * mz_n + 1013904243);
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#ifdef MZRAN13SET_USED
static void mzran13set(uint32_t x, uint32_t y, uint32_t z, uint32_t n)
{
   mz_x = x;
   mz_y = y;
   mz_z = z;
   mz_n = n;
   mz_c = y > z;
}
#endif

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

static uint32_t mz_rand_uint32(uint32_t max)
{
   if ((max + 1) & max) {
      uint32_t r, n = 0xffffffff / (max + 1);
      do {
         r = mzran13() / n;
      } while (r > max);
      return r;
   } else {
      return mzran13() & max;
   }
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

static ULONG mz_rand_ulong(ULONG max)
{
   #if ULONG_MAX > 0xFFFFFFFFUL
   if (sizeof(ULONG) * CHAR_BIT == 64) {
      if ((max + 1) & max) {
         ULONG r, n = (ULONG)(-1L) / (max + 1);
         do {
            r = ((ULONG)mzran13() << 32 | (ULONG)mzran13()) / n;
         } while (r > max);
         return r;
      } else {
         return mzran13() & max;
      }
   } else
   #endif
   if (sizeof(ULONG) * CHAR_BIT == 32) {
      return mz_rand_uint32(max);
   } else {
      assert( !"unsupported number of bits in unsigned long");
   }
}

/*---------------------------------------------------------------------------*/

int32_t random_int32(int32_t min, int32_t max)
{
   return (int32_t)((uint32_t)min + mz_rand_uint32((uint32_t)(max - min)));
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

uint32_t random_uint32(uint32_t min, uint32_t max)
{
   return min + mz_rand_uint32(max - min);
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

long random_long(long min, long max)
{
   return (long)((ULONG)min + mz_rand_ulong((ULONG)(max - min)));
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

ULONG random_ulong(ULONG min, ULONG max)
{
   return min + mz_rand_ulong(max - min);
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

int random_int(int min, int max)
{
   if (sizeof(int) * CHAR_BIT == 64) {
      return (int)random_long(min, max);
   } else if (sizeof(int) * CHAR_BIT <= 32) {
      return (int)random_int32(min, max);
   } else {
      assert( !"unsupported number of bits in int");
   }
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

UINT random_uint(UINT min, UINT max)
{
   if (sizeof(UINT) * CHAR_BIT == 64) {
      return (UINT)random_ulong(min, max);
   } else if (sizeof(UINT) * CHAR_BIT <= 32) {
      return (UINT)random_uint32(min, max);
   } else {
      assert( !"unsupported number of bits in unsigned int");
   }
}

/*---------------------------------------------------------------------------*/

float random_float(void)
{
   if (sizeof(float) * CHAR_BIT == 32) {
      union { float f; uint32_t ui32; } u;
      do {
         u.ui32 = random_uint32(0, 0xFFFFFFFFU);
      } while (u.f != u.f); /*NaN*/
      return u.f;
   } else if (sizeof(float) * CHAR_BIT == 64 && sizeof(ULONG) == 64) {
      union { float f; ULONG ul; } u;
      do {
         u.ul = random_ulong(0, ULONG_MAX);
      } while (u.f != u.f); /*NaN*/
      return u.f;
   } else {
      assert( !"unsupported number of bits in float");
   }
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

double random_double(void)
{
   if (sizeof(double) * CHAR_BIT == 64 && sizeof(ULONG) * CHAR_BIT == 64) {
      union { double d; ULONG ul; } u;
      do {
         u.ul = random_ulong(0, ULONG_MAX);
      } while (u.d != u.d); /*NaN*/
      return u.d;
   } else if (sizeof(double) * CHAR_BIT == 64 && sizeof(ULLONG) * CHAR_BIT == 64
                                              && sizeof(ULONG ) * CHAR_BIT == 32) {
      union { double d; ULLONG ull; } u;
      do {
         u.ull = (ULLONG)random_ulong(0, ULONG_MAX) << 32 | (ULLONG)random_ulong(0, ULONG_MAX);
      } while (u.d != u.d); /*NaN*/
      return u.d;
   } else if (sizeof(double) * CHAR_BIT == 32) {
      union { double d; uint32_t ui32; } u;
      do {
         u.ui32 = random_uint32(0, 0xFFFFFFFFU);
      } while (u.d != u.d); /*NaN*/
      return u.d;
   } else {
      assert( !"unsupported number of bits in double");
   }
}

/*---------------------------------------------------------------------------*/

struct random_index_generator
{
   int *v;
   int  n;
};

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void *new_random_index_generator(int vector_size)
{
   int n = vector_size;
   int i;
   struct random_index_generator *rig = malloc(sizeof(*rig));
   if ( !rig) {
      return NULL;
   }
   rig->v = n > 0 ? malloc(n * sizeof(int)) : NULL;
   rig->n = n;
   for (i = 0; i < n; i++) {
      rig->v[i] = i;
   }
   return rig;
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

int random_index(void *_rig)
{
   int top, i, ret;
   struct random_index_generator *rig = _rig;
   if (rig->n <= 0) {
      if (rig->v) {
         free(rig->v);
         rig->v = NULL;
      }
      return -1;
   }
   top = --rig->n;
   i = random_int(0, top);
   ret = rig->v[i];
   if (i != top) {
      rig->v[i] = rig->v[top];
   }
   rig->n = top;
   return ret;
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void free_random_index_generator(void *_rig)
{
   struct random_index_generator *rig = _rig;
   if (rig->v) {
      free(rig->v);
      rig->v = NULL;
      rig->n = -1;
   }
   free(rig);
}

/*---------------------------------------------------------------------------*/

void random_string(char *str, int length, char *alphabet)
{
   char *c;
   int max = strlen(alphabet) - 1;
   if (max < 0) max = 0;
   if (0 && length == 5) {
      for (c = str; c < str + 4; c++) {
         *c = '-';
      }
      *c++ = alphabet[random_int(0, max)];
   } else {
      for (c = str; c < str + length; c++) {
         *c = alphabet[random_int(0, max)];
      }
   }
   str[length] = '\0';
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

void alloc_random_string(char **p_str, int length, char *alphabet)
{
   char *str = *p_str;
   if (str) {
      str = realloc(str, length + 1);
   } else {
      str = malloc(length + 1);
   }
   if ( !str) return;
   random_string(str, length, alphabet);
   *p_str = str;
}
