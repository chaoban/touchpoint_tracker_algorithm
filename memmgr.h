/*++
    Copyright (c) Silicon Integrated Systems Corporation.  All rights reserved.

    Module Name:
        memmgr.h

    Abstract: 
        memory management

--*/

#ifndef _MEMMGR_H
#define _MEMMGR_H


//
// Key definitions
//


//
// Include
//
#ifdef __linux__
#include <linux/slab.h>
//#include <string.h>
#include <linux/types.h>
#else
#include <wdm.h>
#endif


//
// Constants
//
#ifdef __linux__ 
#else
#define HPEN_POOL_TAG                           'nepH'
#endif


//
// Macros
//
#ifdef __linux__ 
#define MEMSETZERO(addr, size)                  memset(addr, 0, size)
#define MEMMOVE(dst, src, size)                 memmove(dst, src, size)
#define MEMCOPY(dst, src, size)                 memcpy(dst, src, size)

#define MEMMALLOCPAGED(size)                    kmalloc(size, GFP_KERNEL)
#define MEMMALLOCNONPAGED(size)                 kmalloc(size, GFP_KERNEL)

#define MEMREMALLOCPAGED(addr, size)            krealloc(addr, GFP_KERNEL)
#define MEMREMALLOCNONPAGED(addr, size)         krealloc(addr, GFP_KERNEL)
#define MEMFREE(addr)                           kfree(addr)
#else
#define MEMSETZERO(addr, size)                  RtlZeroMemory(addr, size)
#define MEMMOVE(dst, src, size)                 RtlMoveMemory(dst, src, size)
#define MEMCOPY(dst, src, size)                 RtlCopyMemory(dst, src, size)

#define MEMMALLOCPAGED(size)                    ExAllocatePoolWithTag(PagedPool, size, HPEN_POOL_TAG)
#define MEMMALLOCNONPAGED(size)                 ExAllocatePoolWithTag(NonPagedPool, size, HPEN_POOL_TAG)
#define MEMREMALLOCPAGED(addr, size)            ReallocFun(addr, size, PagedPool)
#define MEMREMALLOCNONPAGED(addr, size)         ReallocFun(addr, size, NonPagedPool)
#define MEMFREE(addr)                           ExFreePool(addr)
                        
#endif


//
// Type Definitions
//


//
// Function prototypes
//
#ifdef __linux__
#else
__inline void* ReallocFun(void *addr, SIZE_T size, POOL_TYPE pool_type)
{
    void *new_addr = NULL;

    if (pool_type == NonPagedPool)
    {
        new_addr = (void *)MEMMALLOCNONPAGED(size);
    }
    else if (pool_type == PagedPool)
    {
        new_addr = (void *)MEMMALLOCPAGED(size);
    }
    
    MEMCOPY(new_addr, addr, size); 

    if (addr != NULL)
    {
        MEMFREE(addr);
    }
    return new_addr;
}
#endif


//
// Global Data Declarations
//


#endif // ~ _MEMMGR_H

