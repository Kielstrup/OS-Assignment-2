
#include <stdio.h>
#include <stdint.h>

#include "mm.h"


/** 
 * Test program that makes some simple allocations and enables
 * you to inspect the result.
 *
 * Elaborate on your own.
 */

int main(int argc, char ** argv) {

  /* Ensure that macros are working */
  int ret = simple_macro_test();
  if (ret > 0) {
    printf("Macro test returned %d\n", ret);
    return 1;
  }

  void * a = simple_malloc(0x200); // Block A (512 bytes)

  void * b = simple_malloc(0x100); // Block B (256 bytes)

  void * c = simple_malloc(0x300); // Block C (768 bytes)

  // Free block a and c
  simple_free(a);
  simple_free(c);

  simple_malloc(0x100);

  // Allocate a new block "D" of size 0x150 (336 bytes), smaller than both a and c
  // Next-fit should allocate D in the space of C, not A.
  void *d = simple_malloc(0x150); // Block D (336 bytes)

  simple_free(b);

  simple_block_dump(); 

  return 0;
}
