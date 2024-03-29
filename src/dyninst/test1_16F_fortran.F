 !
 ! See the dyninst/COPYRIGHT file for copyright information.
 ! 
 ! We provide the Paradyn Tools (below described as "Paradyn")
 ! on an AS IS basis, and do not warrant its validity or performance.
 ! We reserve the right to update, modify, or discontinue this
 ! software at any time.  We shall have no obligation to supply such
 ! updates or modifications or any other form of support to you.
 ! 
 ! By your use of Paradyn, you understand and agree that we (or any
 ! other person or entity with proprietary rights in Paradyn) are
 ! under no obligation to provide either maintenance services,
 ! update services, notices of latent defects, or correction of
 ! defects for Paradyn.
 ! 
 ! This library is free software; you can redistribute it and/or
 ! modify it under the terms of the GNU Lesser General Public
 ! License as published by the Free Software Foundation; either
 ! version 2.1 of the License, or (at your option) any later version.
 ! 
 ! This library is distributed in the hope that it will be useful,
 ! but WITHOUT ANY WARRANTY; without even the implied warranty of
 ! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 ! Lesser General Public License for more details.
 ! 
 ! You should have received a copy of the GNU Lesser General Public
 ! License along with this library; if not, write to the Free Software
 ! Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 !

! libdyninst validation suite test #1
! author: Jon Beyer (6/1/01)
!	derived from a previous test by Jeff Hollingsworth

!------------------------------------------------------------------------------
#define stop_process_ stop_process

#ifdef __GNUC__
#define AND iand
#define OR ior
#else
#define AND and
#define OR or
#endif

!------------------------------------------------------------------------------

	SUBROUTINE test1_16f_init_globals
#include "test1_16F_common.h"

	test1_16_globalVariable16_1 = 0
	test1_16_globalVariable16_2 = 0
	test1_16_globalVariable16_3 = 0
	test1_16_globalVariable16_4 = 0
	test1_16_globalVariable16_5 = 0
	test1_16_globalVariable16_6 = 0
	test1_16_globalVariable16_7 = 0
	test1_16_globalVariable16_8 = 0
	test1_16_globalVariable16_9 = 0
	test1_16_globalVariable16_10 = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_16_func1
#include "test1_16F_common.h"
	LOGICAL failed

	failed = .false.

	CALL test1_16_func2
	if (test1_16_globalVariable16_1 .ne. 1 .or. test1_16_globalVariable16_2 .ne. 0) then
		print *, '**Failed test #16 in spot 1 (if-else)'
		if (test1_16_globalVariable16_1 .ne. 1) then
			print *, '    True clause of first if should have been',
     & ' executed but was not'
		endif
		if (test1_16_globalVariable16_2 .ne. 0) then
			print *, '    False clause of first if should not have',
     & ' been executed but was'
		endif
		failed = .true.
	endif

	CALL test1_16_func3
	if (test1_16_globalVariable16_3 .ne. 0 .or. test1_16_globalVariable16_4 .ne. 1) then
		print *, '**Failed test #16 (if-else)'
		if (test1_16_globalVariable16_3 .ne. 1) then
			print *, '    True clause of second if should not have',
     & ' been executed but was'
		endif
		if (test1_16_globalVariable16_4 .ne. 0) then
			print *, '    False clause of second if should have',
     & ' been executed but was not'
		endif
		failed = .true.
	endif

	CALL test1_16_func4
	if (test1_16_globalVariable16_5 .ne. 0 .or. test1_16_globalVariable16_6 .ne. 1
     & .or. test1_16_globalVariable16_7 .ne. 0
     & .or. test1_16_globalVariable16_8 .ne. 1
     & .or. test1_16_globalVariable16_9 .ne. 0
     & .or. test1_16_globalVariable16_10 .ne. 1)
     & then
			print *, '    failed large if clauses tests'
			failed = .true.
	endif

	if (failed .eqv. .false.) then
		passedTest = 1
	endif
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_16_func2
#include "test1_16F_common.h"
	INTEGER foo

	foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_16_func3
#include "test1_16F_common.h"
	INTEGER foo

	foo = 0
	END

!------------------------------------------------------------------------------

	SUBROUTINE test1_16_func4
#include "test1_16F_common.h"
	INTEGER foo

	foo = 0
	END

!------------------------------------------------------------------------------
