/*
 *  ZenWINX - WIndows Native eXtended library.
 *  Copyright (c) 2007,2008 by Dmitri Arkhangelski (dmitriar@gmail.com).
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
* zenwinx.dll functions to allocate and free memory.
*/

#define WIN32_NO_STATUS
#define NOMINMAX
#include <windows.h>

#include "ntndk.h"
#include "zenwinx.h"

/****ix* zenwinx.internals/long_type_on_x64
* NAME
*   long data type on x64 system
* NOTES
*   If we are using winx_virtual_alloc(long size) then
*   x64 MS C compiler generates the following commands:
*
*   mov  dword ptr [rsp+8], ecx ; ecx contains size
*   sub  rsp, 0x40
*   lea  r9, [rsp+0x50]
*   ...
*   call NtAllocateVirtualMemory
*
*   Low part of r9 register contains size, high part - TRASH!
*   Because MS C compiler assumes that 'long' data type is always
*   32-bit long.
*
*   On the other hand, NtAllocateVirtualMemory() system call
*   is compatible with ISO C standard, it uses all 64-bits of it's 
*   argument.
*
*   Therefore NtAllocateVirtualMemory always has trash in 
*   'size' parameter. Therefore it fails.
*
*   To suppress this mistake we are using since 2.1.0 version
*   of the Ultra Defragmenter the following form:
*   winx_virtual_alloc(ULONGLONG size);
******/

/****f* zenwinx.memory/winx_virtual_alloc
* NAME
*    winx_virtual_alloc
* SYNOPSIS
*    addr = winx_virtual_alloc(size);
* FUNCTION
*    Commit a region of pages in the virtual 
*    address space of the calling process.
* INPUTS
*    size - size of the region, in bytes.
* RESULT
*    If the function succeeds, the return value is
*    the base address of the allocated region of pages.
*    If the function fails, the return value is NULL.
* EXAMPLE
*    buffer = winx_virtual_alloc(1024);
*    if(!buffer){
*        winx_printf("Can't allocate 1024 bytes of memory!\n");
*        winx_exit(2); // exit with code 2
*    }
*    // ...
*    winx_virtual_free(buffer, 1024);
* NOTES
*    1. Memory allocated by this function
*       is automatically initialized to zero.
*    2. Memory protection for the region of
*       pages to be allocated is PAGE_READWRITE.
* SEE ALSO
*    winx_virtual_free
******/
void * __stdcall winx_virtual_alloc(ULONGLONG size)
{
	void *addr = NULL;
	NTSTATUS Status;

	Status = NtAllocateVirtualMemory(NtCurrentProcess(),&addr,0,
		(PULONG)&size,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
	return (NT_SUCCESS(Status)) ? addr : NULL;
}

/****f* zenwinx.memory/winx_virtual_free
* NAME
*    winx_virtual_free
* SYNOPSIS
*    winx_virtual_free(addr, size);
* FUNCTION
*    Release a region of pages within the
*    virtual address space of the calling process.
* INPUTS
*    addr - pointer to the base address
*           of the region of pages to be freed.
*    size - size of the region of memory
*           to be freed, in bytes.
* RESULT
*    This function does not return a value.
* EXAMPLE
*    See an example for the winx_virtual_alloc() function.
* NOTES
*    After this call you must not refer
*    to the specified memory again.
* SEE ALSO
*    winx_virtual_alloc
******/
void __stdcall winx_virtual_free(void *addr,ULONGLONG size)
{
	NtFreeVirtualMemory(NtCurrentProcess(),&addr,
		(PULONG)&size,MEM_RELEASE);
}
