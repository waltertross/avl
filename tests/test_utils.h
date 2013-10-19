/*----------------------------------------------------------------------------*
 |                                                                            |
 |                                test_utils.h                                |
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

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdint.h>
#include <limits.h>

int32_t       random_int32 (      int32_t min,       int32_t max);
uint32_t      random_uint32(     uint32_t min,      uint32_t max);
long          random_long  (         long min,          long max);
unsigned long random_ulong (unsigned long min, unsigned long max);
int           random_int   (         int  min,          int  max);
unsigned int  random_uint  (unsigned int  min, unsigned int  max);

/* These may return infinites, denorms and -0.0, but not NaNs */
/* (unless x != x is false for NaNs, against IEEE 754) */
float  random_float (void);
double random_double(void);

void *new_random_index_generator(int vector_size);
int   random_index(void *rig);
void  free_random_index_generator(void *rig);

void random_string      (char *str,    int length, char *alphabet);
void alloc_random_string(char **p_str, int length, char *alphabet);

#endif
