/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include <stdint.h>
#include <stdlib.h>  // Only included for EXIT_FAILURE

#include "mm.h"

extern const uintptr_t memory_start;
extern const uintptr_t memory_end;

/* Proposed data structure elements */

typedef struct header {
  struct header * next;     // Bit 0 is used to indicate free block 
  uint64_t user_block[0];   // Standard trick: Empty array to make sure start of user block is aligned
} BlockHeader;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
#define GET_NEXT(p)    (void *)((uintptr_t)(p->next) & ~0x1)  // Mask out the least significant bit to get the actual pointer
#define SET_NEXT(p, n) p->next = (void *)((uintptr_t)(n) | ((uintptr_t)(p->next) & 0x1))  // Preserve the free flag
#define GET_FREE(p)    (uint8_t)((uintptr_t)(p->next) & 0x1)  // Get the least significant bit to determine free status
#define SET_FREE(p, f) p->next = (void *)(((uintptr_t)(p->next) & ~0x1) | (f & 0x1))  // Set or clear the free flag
#define SIZE(p)        (size_t)((uintptr_t)GET_NEXT(p) - (uintptr_t)(p) - sizeof(BlockHeader))  // Calculate block size
#define MIN_SIZE (8)   // A block should have at least 8 bytes available for the user

static BlockHeader * first = NULL;
static BlockHeader * current = NULL;

/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 *
 */
void simple_init() {
    uintptr_t aligned_memory_start = (memory_start + 7) & ~0x7; // Align to 8 bytes
    uintptr_t aligned_memory_end = memory_end; // No need to align the end

    if (first == NULL) {
        if (aligned_memory_start + sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
            // Initialize the first block
            first = (BlockHeader *)aligned_memory_start;
            SET_NEXT(first, (BlockHeader *)(aligned_memory_end - sizeof(BlockHeader))); // Last block
            SET_FREE(first, 1); // Mark the first block as free

            // Initialize the last block (dummy block)
            BlockHeader *last = (BlockHeader *)(aligned_memory_end - sizeof(BlockHeader));
            SET_NEXT(last, first); // Circular reference to first block
            SET_FREE(last, 0); // Last block is always considered allocated

            current = first; // Set the current pointer to the first block
        } else {
            fprintf(stderr, "Not enough memory to initialize\n");
            exit(EXIT_FAILURE);
        }
    }
}


/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory and return a pointer to the first byte.
 *
 * This function should behave similar to a normal malloc implementation. 
 *
 * @param size_t size Number of bytes to allocate.
 * @retval Pointer to the start of the allocated memory or NULL if not possible.
 *
 */

void* simple_malloc(size_t size) {
    if (first == NULL) {
        simple_init(); // Initialize memory if not already done
        if (first == NULL) return NULL;
    }

    size_t aligned_size = (size + 7) & ~0x7; // Align requested size
    BlockHeader *search_start = current;

    do {
        if (GET_FREE(current)) {
            size_t block_size = SIZE(current);

            // Check if the free block is large enough
            if (block_size >= aligned_size) {
                // Check if we can split the block
                if (block_size - aligned_size >= sizeof(BlockHeader) + MIN_SIZE) {
                    BlockHeader *new_block = (BlockHeader *)((uintptr_t)current + sizeof(BlockHeader) + aligned_size);
                    SET_NEXT(new_block, GET_NEXT(current));
                    SET_FREE(new_block, 1); // New block is free

                    SET_NEXT(current, new_block); // Update current block to point to the new block
                    SET_FREE(current, 0); // Mark the current block as used
                    printf("Allocating %zu bytes at %p\n", aligned_size, (void*)current->user_block); // Print when allocating
                } else {
                    SET_FREE(current, 0); // Mark the current block as used
                    printf("Allocating %zu bytes at %p (no split)\n", aligned_size, (void*)current->user_block); // Print when allocating without splitting
                }

                void *allocated_memory = (void *)(current->user_block); // Return pointer to user block
                current = GET_NEXT(current); // Update current to the next block
                return allocated_memory;
            }
        }
        current = GET_NEXT(current); // Move to the next block
    } while (current != search_start); // Loop until we return to the starting block
    
    printf("Allocation failed for %zu bytes\n", aligned_size); // Print if allocation fails
    return NULL; // No suitable block found
}


/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 *
 * This function should behave similar to a normal free implementation. 
 *
 * @param void *ptr Pointer to the memory to free.
 *
 */

void simple_free(void* ptr) {
    if (ptr == NULL) return;

    BlockHeader *block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));

    if (GET_FREE(block)) {
        return; // Already free
    }

    SET_FREE(block, 1); // Mark the block as free

    // Attempt to merge with the next block if it's free and not the dummy block
    BlockHeader *next_block = GET_NEXT(block);
    while (GET_FREE(next_block) && next_block != first) {
        SET_NEXT(block, GET_NEXT(next_block)); // Link to the block after next
        next_block = GET_NEXT(block); // Update next_block to the new next block
        printf("Freeing block at %p and merging with next block\n", (void*)block);
    }

    printf("Freeing block at %p\n", (void*)block);
}


/* Include test routines */

#include "mm_aux.c"
