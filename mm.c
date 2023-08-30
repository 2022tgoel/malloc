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
struct header;

typedef struct header header;

struct header {
    size_t size;
    header *prev; // the previous FREE block
    header *next; // the next FREE block
};

typedef struct {
    size_t size;
    
} footer;

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
#define FOOTER_SIZE align(sizeof(footer))
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

static inline size_t payload_sz_to_block_sz(size_t sz){
    return align(sz + HEADER_SIZE + FOOTER_SIZE);
}

static inline size_t block_sz_to_payload_sz(size_t sz){
    return sz - HEADER_SIZE - FOOTER_SIZE;
}

static inline size_t get_size_hdr(header *hdr) {
    return hdr->size & ~1;
}

static inline size_t get_size_ftr(footer *ftr) {
    return ftr->size;
}

static inline size_t get_free(header *hdr) {
    return hdr->size & 1;
}

static inline void set_size(header *hdr, size_t sz) {
    size_t free = get_free(hdr);
    hdr->size = sz | free;
}

static inline void set_free(header *hdr, size_t free) {
    size_t sz = get_size_hdr(hdr);
    hdr->size = sz | free;
}


static header* freeList = NULL;
/*
 * If you write a HEADER_TO_FOOTER function, it will turn out to be a
 * Good Thing if it accepts a size argument.  Remove the #if and #endif
 * to enable this function.
 */

static inline header *footer_to_header(footer *p) {
    size_t size = get_size_ftr(p);
    return (header *)((char *)p + FOOTER_SIZE - size);
}

#if 1
static inline footer *header_to_footer(header *hdr)
{
    return (footer*)((char *) hdr + get_size_hdr(hdr) - FOOTER_SIZE);
}
#endif

static void update_size(header *hdr, size_t newsize) {
    assert(newsize >= payload_sz_to_block_sz(ALIGNMENT));
    set_size(hdr, newsize);
    header_to_footer(hdr)->size = newsize;
}

static header* first() {
    if (mem_heap_hi() == mem_heap_lo())
        return NULL;
    else 
        return (header *) mem_heap_lo();
}

static header* next(header *hdr) {
    void* nextAddr = (void*)((char *) hdr + get_size_hdr(hdr));
    if (nextAddr >= mem_heap_hi()) 
        return NULL;
    return (header*) nextAddr;
}

static header* prev(header *hdr){
    footer *prevFooter = (footer *)((char *)hdr - FOOTER_SIZE);
    return footer_to_header(prevFooter);
}

static void add_to_freelist(header *hdr) {
    set_free(hdr, 1);
    // append to the beginning of the list
    if (freeList->next != NULL) {
        freeList->next->prev = hdr;
    }
    hdr->next = freeList->next;
    freeList->next = hdr;
    hdr->prev = freeList;
    
}

static void remove_from_freelist(header *hdr) {
    hdr->prev->next = hdr->next;
    if (hdr->next != NULL) 
        hdr->next->prev = hdr->prev;
}

static void split(header* hdr, size_t newsize) {
    /*
     * Take an allocated block and crop it to newsize
     * the remaining portion can be freed
    */
    assert(get_free(hdr) == 0); 
    size_t othersize = get_size_hdr(hdr) - newsize;
    if (othersize < payload_sz_to_block_sz(ALIGNMENT)) // block can't even store one byte of data
        return;
    header* otherhdr = (header*) ((char*) hdr + newsize);
    update_size(hdr, newsize);
    update_size(otherhdr, othersize);
    add_to_freelist(otherhdr);
}

static void merge(header* hdr) {
    header *nextBlock = next(hdr);
    if (nextBlock != NULL && get_free(hdr) && get_free(nextBlock)){
        update_size(hdr, get_size_hdr(hdr) + get_size_hdr(nextBlock));
        remove_from_freelist(nextBlock);
    }
    header *prevBlock = prev(hdr);
    if (prevBlock != NULL && get_free(hdr) && get_free(prevBlock)){
        update_size(prevBlock, get_size_hdr(hdr) + get_size_hdr(prevBlock));
        remove_from_freelist(hdr);
    }
}
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

#if 0
#define MM_CHECK() mm_check()
#else
#define MM_CHECK()
#endif

int mm_check(void) {
    header *hdr = freeList;
    int i = 0;
    #define MAX 1e9
    while (hdr->next != NULL && i++ < MAX) {
        header *nxt = hdr->next;
        if (nxt == hdr) {
            printf("\nLinked list has infinite loop of length 1\n");
            fflush(stdout);
            assert(0);
        }
        if (nxt->prev != hdr || hdr->next != nxt) {
            printf("\nLinked list connected corrupted.\n");
            fflush(stdout);
            assert(0);
        }
        if (get_free(nxt) != 1) {
            printf("\nBlock on free list isn't free\n");
            fflush(stdout);
            assert(0);
        }
        hdr = nxt;
    }
    if (i >= MAX) {
        printf("\nLinked list has infinite loop\n");
        fflush(stdout);
        assert(0);
    }
    hdr = first();
    if (get_size_hdr(hdr) != payload_sz_to_block_sz(ALIGNMENT) || get_free(hdr) != 0) {
        printf("\nFirst Block not set correctly.\n");
        fflush(stdout);
        assert(0);
    }
    while (next(hdr) != NULL) {
        header *oldhdr = hdr;
        hdr = next(hdr);
        if ((char *)oldhdr + get_size_hdr(oldhdr) != (char *)hdr) {
            printf("Block sizes are not correct. Size should be %d, and is %d\n", (char *)hdr - (char *)oldhdr, oldhdr->size);
            fflush(stdout);
            assert(0);
        }
        if (align(get_size_hdr(hdr)) != get_size_hdr(hdr)) {
            printf("Size is not aligned");
            fflush(stdout);
            assert(0);
        }
        if (hdr->size <= 0) {
            printf("Block not valid, size is %d, pointer is %p\n", hdr->size, hdr);
            fflush(stdout);
            assert(0);
        }
        if (hdr->prev == NULL) {
            printf("Previous block not valid\n");
            fflush(stdout);
            assert(0);
        }
        if (hdr->prev->size <= ALIGNMENT) {
            printf("Previous block not valid, size is %d, pointer is %p\n", hdr->prev->size, hdr->prev);
            fflush(stdout);
            assert(0);
        }
    }
    return 0;
}
/*
 * Actualy memory-allocation functions follow this point.
 */

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    size_t size = payload_sz_to_block_sz(ALIGNMENT);
    header *hdr = (header *)mem_sbrk(size);
    if ((int)hdr == -1)
        return -1;
    update_size(hdr, size);
    set_free(hdr, 0); // It's in the free list, even though, it's allocated? This is weird, but guarantees we can always use it as the head.
    hdr->prev = NULL;
    hdr->next = NULL;
    freeList = hdr;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    MM_CHECK();
    if (size == 0)
        return NULL;

    /*
     * Simple first-fit scheme. Iterate through all the blocks. 
     * Once a block that is large enough and is marked free is found, use it. 
    */
    int newsize = payload_sz_to_block_sz(size);
    header *hdr = freeList;
    while (hdr->next != NULL && !(get_free(hdr) && newsize <= get_size_hdr(hdr))) {
        hdr = hdr->next;
    }
    if (!(get_free(hdr) && newsize <= get_size_hdr(hdr))) { 
        /* 
         * Did not find anything. 
         */
        header *newhdr = (header *)mem_sbrk(newsize);
        if ((int)newhdr == -1)
            return NULL;
        else {
            update_size(newhdr, newsize);
            set_free(newhdr, 0);
            return header_to_payload(newhdr);
        }

    } else {
        set_free(hdr, 0);
        remove_from_freelist(hdr);
        split(hdr, newsize);
        return header_to_payload(hdr);
    }
}

/*
 * mm_free - Freeing a block does nothing in this allocator.
 */
void mm_free(void *ptr)
{   
    header* hdr = payload_to_header(ptr);
    add_to_freelist(hdr);
    merge(hdr);
}

/*
 * mm_realloc - Implemented simply and fairly stupidly in terms of mm_malloc
 * and mm_free.
 */
void *mm_realloc(void *ptr, size_t newSize)
{
    header *oldhdr = payload_to_header(ptr);
    void *newPayload;
    size_t payloadSize;
    size_t copySize;
    size_t allocSize = payload_sz_to_block_sz(newSize);
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
    
    payloadSize = block_sz_to_payload_sz(get_size_hdr(oldhdr));
    if (newSize <= payloadSize) {
        split(oldhdr, payload_sz_to_block_sz(newSize));
        return header_to_payload(oldhdr);
    }

    if (next(oldhdr) != NULL && get_free(next(oldhdr))) {
        header *nxt = next(oldhdr);
        if (get_size_hdr(nxt) + get_size_hdr(oldhdr) >= allocSize) {
            set_free(nxt, 0);
            remove_from_freelist(nxt);
            set_size(oldhdr, get_size_hdr(nxt) + get_size_hdr(oldhdr));
            split(oldhdr, allocSize);
            return header_to_payload(oldhdr);
        }
    }
    /*
     * Allocate new space, and copy it over.
     */
    newPayload = mm_malloc(newSize);
    if (newPayload == NULL)
      return NULL;
    copySize = payloadSize;
    memcpy(newPayload, ptr, copySize);
    mm_free(ptr);
    return newPayload;
}

