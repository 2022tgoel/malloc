/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  Except for a size header (needed to make realloc
 * work), a block is pure payload. There are no footers.  Blocks are
 * never coalesced or reused. Realloc is implemented directly using
 * mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high-level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "team",
    /* First member's full name */
    "Tarushii Goel",
    /* First member's email address */
    "tishi.tng@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/*
 * Choose the alignment, in bytes, that will be forced on all blocks
 * allocated.  All allocated blocks will be rounded up to a multiple
 * of this size; for that reason they will also automatically be
 * aligned on a boundary of this size.  (The latter follows only
 * because mem_sbrk returns aligend blocks.  On a real system, using
 * sbrk rather than mem_sbrk, blocks are aligned to an absurdly large
 * boundary, such as 4096 bytes.  However, in this lab the alignment
 * is guaranteed only to 4 or 8 bytes.)
 *
 * Normally, this should be set to the smallest value that will cause
 * all blocks to be aligned legally under the current compiler; i.e.,
 * 4 bytes on Linux, or 8 on Windows.
 */
#define ALIGNMENT 4

/*
 * The following typedef'd struct defines the header that is placed
 * before every block of allocated memory.  You may wish to expand
 * this header.  You can also use a similar declaration to define a
 * footer, if you choose.
 */
typedef struct {
    size_t size;                /* Size of this block, including header */
    
} header;

/*
 * The following inline function generates an expression that rounds
 * up to the next highest multiple of ALIGNMENT.  However, it only
 * works if ALIGNMENT is a power of 2.  It is worth your time to
 * analyze how and why this expression works.
 *
 * If "size" is already aligned, the function is a no-op.  This fact is
 * used in somme of the other functions.
 */
static inline size_t align(size_t size)
{
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

/*
 * The following macro generates an aligned (i.e., rounded-up) version
 * of the amount of space used for the header.  It will be used in
 * some subsequent macros.
 *
 * If you decide to create a footer, you will probably want to write
 * FOOTER_SIZE.  It may also be convenient to write OVERHEAD_SIZE as
 * the sum of HEADER_SIZE and FOOTER_SIZE (don't forget to
 * parenthesize the result!).
 *
 * DO NOT, repeat DO NOT, make the mistake of lazily using HEADER_SIZE
 * to calculate the size of the footer, just because your footer is
 * the same size as your current header.  Take the time to do things
 * carefully.
 */
#define HEADER_SIZE align(sizeof(header))

/*
 * The following function converts a pointer to a block header into a
 * pointer to its payload.  To do so, it first typecasts the pointer
 * to "char *" (so that pointer arithmetic will be done on a byte
 * basis), and then casts it back to "void *" (because that's what
 * malloc will return).
 *
 * As with align, it is worth your time to study this function so you can
 * understand what how it accomplishes its task.
 *
 * Even if you define a footer structure, there will be little need
 * for a FOOTER_TO_PAYLOAD macro.  However, you will probably want to
 * write HEADER_TO_FOOTER (below).
 */
static inline void *header_to_payload(header *p)
{
    return (void *)((char *)p + HEADER_SIZE);
}

size_t get_size(header *hdr) {
    return hdr->size & ~1;
}

size_t get_free(header *hdr) {
    return hdr->size & 1;
}

void set_size(header *hdr, size_t sz) {
    size_t free = get_free(hdr);
    hdr->size = sz | free;
}

void set_free(header *hdr, size_t free) {
    size_t sz = get_size(hdr);
    hdr->size = sz | free;
}

header* first() {
    if (mem_heap_hi() == mem_heap_lo())
        return NULL;
    else 
        return (header *) mem_heap_lo();
}

header* next(header *hdr) {
    void* nextAddr = (void*)((char *) hdr + get_size(hdr));
    if (nextAddr >= mem_heap_hi()) 
        return NULL;
    return (header*) nextAddr;
}

/*
 * If you write a HEADER_TO_FOOTER function, it will turn out to be a
 * Good Thing if it accepts a size argument.  Remove the #if and #endif
 * to enable this function.
 */
#if 0
static inline footer *header_to_footer(p, size_t size)
{
    return (footer*)error: Not written yet;
}
#endif

/*
 * The following function is the reverse of header_to_payload.  It takes
 * a pointer to a payload and turns it into a correctly typecast
 * pointer to the corresponding header.
 *
 * Note that there probably won't be a need for payload_to_footer, but
 * if you need it you can construct it from this function and
 * header_to_footer.
 */
static inline header *payload_to_header(void *p)
{
    return (header *)((char *)p - HEADER_SIZE);
}

/*
 * Actualy memory-allocation functions follow this point.
 */

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    /*
     * Simple first-fit scheme. Iterate through all the blocks. 
     * Once a block that is large enough and is marked free is found, use it. 
    */
    int newsize = align(size + HEADER_SIZE);
    header *hdr = first();
    while (hdr != NULL && !(get_free(hdr) && newsize <= get_size(hdr))) {
        hdr = next(hdr);
    }
    if (hdr == NULL) { 
        /* 
         * Did not find anything. 
         */
        header *hdr = (header *)mem_sbrk(newsize);
        if ((int)hdr == -1)
            return NULL;
        else {
            set_size(hdr, newsize);
            set_free(hdr, 0);
            return header_to_payload(hdr);
        }

    } else {
        set_free(hdr, 0);
        return header_to_payload(hdr);
    }
}

/*
 * mm_free - Freeing a block does nothing in this allocator.
 */
void mm_free(void *ptr)
{
    set_free(payload_to_header(ptr), 1);
}

/*
 * mm_realloc - Implemented simply and fairly stupidly in terms of mm_malloc
 * and mm_free.
 */
void *mm_realloc(void *ptr, size_t newSize)
{
    header *oldhdr = payload_to_header(ptr);
    void *newPayload;
    size_t copySize;
    
    /*
     * Handle corner cases in the specification.
     */
    if (newSize == 0) {
        if (ptr != NULL)
            mm_free(ptr);
        return NULL;
    }
    else if (ptr == NULL) {
        return mm_malloc(newSize);
    }

    /*
     * Allocate new space, and copy it over.
     */
    newPayload = mm_malloc(newSize);
    if (newPayload == NULL)
      return NULL;
    copySize = get_size(oldhdr) - HEADER_SIZE;
    if (newSize < copySize)
        copySize = newSize;
    memcpy(newPayload, ptr, copySize);
    mm_free(ptr);
    return newPayload;
}