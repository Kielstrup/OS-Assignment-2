/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include <stdint.h>

#include "mm.h"



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
  uintptr_t aligned_memory_start = (memory_start + 7) & ~7;
  uintptr_t aligned_memory_end   = memory_end & ~7;
  BlockHeader* last;

  if (first == NULL) {
    /* Check that we have room for at least one free block and an end header */
    if (aligned_memory_start + 2*sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
      // Initialize the first block
      first = (BlockHeader*)aligned_memory_start;
      SET_NEXT(first, (BlockHeader*)(aligned_memory_end - sizeof(BlockHeader))); // Set the last block as next
      SET_FREE(first, 1);  // Mark first block as free


      // Initialize the dummy block (last block)
      last = (BlockHeader*)(aligned_memory_end - sizeof(BlockHeader));
      SET_NEXT(last, first);  // Make the list circular by pointing to the first block
      SET_FREE(last, 0);      // The dummy block is always considered allocated

      // Set the current block to the first block
      current = first;
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
    simple_init();
    if (first == NULL) return NULL;
  }

  size_t aligned_size = (size + 7) & ~7;;
  BlockHeader * search_start = current;

  do {

    printf("Checking block at %pm size: %zu, free: %d\n", current, SIZE(current), GET_FREE(current));
 
    if (GET_FREE(current)) {
      size_t block_size = SIZE(current);

      /* Check if free block is large enough */
      if (block_size >= aligned_size) {
        printf("Found a suitable block at %p with size %zu\n", current, block_size); 

         // Ensure we're not overwriting dummy block
         BlockHeader* next_block = GET_NEXT(current);
         if (next_block == first) {
          // Skip current block since it's a dummy block
          current = GET_NEXT(current);
          continue;
         }

      if (block_size - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
          BlockHeader* new_block = (BlockHeader*)((uintptr_t)current + sizeof(BlockHeader) + aligned_size);
          SET_NEXT(new_block, GET_NEXT(current));
          SET_FREE(new_block, 1);
          SET_NEXT(current, new_block);
      } 
        SET_FREE(current, 0);
        return (void *) (current->user_block);
      } 
    } 
      current = GET_NEXT(current);
  } while (current != search_start);

  printf("No suitable block found for size %zu\n", aligned_size); 

 /* None found */
  return NULL;
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
void simple_free(void * ptr) {
  /*
   if (ptr == NULL) return;

    BlockHeader block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));

    if (GET_FREE(block)) {
        return; // Block is already free, ignore the free
    }

    SET_FREE(block, 1); // Mark the block as free
    BlockHeader next_block = GET_NEXT(block);

    if (GET_FREE(next_block)) {
        SET_NEXT(block, GET_NEXT(next_block)); // Coalesce with next free block
    }
*/
  
  if (ptr == NULL) return;

  BlockHeader* block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));

  if ((uintptr_t)block < memory_start || (uintptr_t)block >= memory_end) {
    printf("Invalid pointer passed to free: %p\n", ptr);
    return; 
  }

  if (GET_FREE(block)) {
    printf("Double free detected at %p\n", ptr); 
    return;
  }

  //SET_NEXT(block, 1);
  SET_FREE(block, 1);

  BlockHeader* next_block = GET_NEXT(block);

  if ((uintptr_t)next_block >= (uintptr_t)first && (uintptr_t)next_block < memory_end) {
    if (GET_FREE(next_block)) {
      SET_NEXT(block, GET_NEXT(next_block));
    }
  }

  if (next_block >= first && next_block < memory_end) {
      if (GET_FREE(next_block)) {
          SET_NEXT(block, GET_NEXT(next_block));
      }
  } else {
        printf("Invalid next_block: %p\n", next_block); 
    }
    
}

/* Include test routines */

#include "mm_aux.c"
