/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <stdbool.h>
#include <complex.h>
#include "solo_mutatee_boilerplate.h"

enum enum1 {
	ef1_1 = 20, 
	ef1_2 = 40, 
	ef1_3 = 60, 
	ef1_4 = 80
} e1;

struct mystruct {
	int elem1;
	double elem2;
	char elem3;
	float elem4;
} ms;

typedef enum {ef2_1, ef2_2, ef2_3, ef2_4} enum2;

typedef void (*func_ptr_typedef_t)(int);

const char * my_string = "my_string";
typedef int int_array_t [256];

typedef int * my_intptr_t;

typedef int int_alias_t;

union my_union{
	float my_float;
	int my_int;
} u1;

void my_test_type_info_func(int foo)
{
	int_array_t my_array;
	int rr =0;
	my_intptr_t ip = &rr;
	int_alias_t r = fprintf(stderr, "booga booga: %d: %s\n", foo, my_string);
	if (r)
	{
		(*ip) = (int) r;
		e1 =  ef1_1;
		u1.my_int = rr;
		ms.elem1 = rr;
		my_array[0] = rr;
	}
}

void my_test_type_info_func2()
{
   func_ptr_typedef_t myfuncptr2;
	void (*myfuncptr)(int) = my_test_type_info_func;
	(*myfuncptr)(5);
   myfuncptr2 = my_test_type_info_func;
	(*myfuncptr2)(6);
}

int test_type_info_mutatee() 
{
   /*If mutatee should run, things go here.*/
   return 0;
}

struct integral_t {
#ifdef __cplusplus
	#define boolean bool
#else
	#define boolean _Bool
#endif
	boolean b; char c; short s; int i; long l; long long ll;
} it;

struct signed_integral_t {
	signed char c; signed short s; signed int i; signed long l; signed long long ll;
} sit;

struct unsigned_integral_t {
	unsigned char c; unsigned short s; unsigned int i; unsigned long l; unsigned long long ll;
} uit;

struct floating_point_t {
	float f; double d; long double ld;
	float _Complex fc; double _Complex dc; long double _Complex ldc;
} fpt;
