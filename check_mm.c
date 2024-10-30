/**
 * @file   check_mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Unit tests and suite for the memory management sub system.
 *
 * Implements one suite of unit tests using the check
 * unit testing framework.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <check.h>
#include "mm.h"

#define MALLOC simple_malloc
#define FREE   simple_free

uint32_t sum_block(uint32_t *data, uint32_t size) {
    uint32_t sum = 0;
    uint32_t n;
    for (n = 0; n < (size) >> 2; n++) {
        sum ^= data[n];
    }
    return sum;
}

START_TEST(test_simple_allocation) {
    int *ptr1;

    ptr1 = MALLOC(10 * sizeof(int));
    ck_assert(ptr1 != NULL);
    FREE(ptr1);
}
END_TEST

START_TEST(test_simple_unique_addresses) {
    int *ptr1;
    int *ptr2;

    ptr1 = MALLOC(10 * sizeof(int));
    ptr2 = MALLOC(10 * sizeof(int));
    ck_assert(ptr1 != ptr2);

    FREE(ptr1);
    FREE(ptr2);
}
END_TEST

START_TEST(test_zero_allocation) {
    void *ptr = MALLOC(0);
    ck_assert(ptr == NULL);
}
END_TEST

START_TEST(test_double_free) {
    int *ptr = MALLOC(10 * sizeof(int));
    FREE(ptr);
    ck_assert_msg(ptr != NULL, "Double free detected!");
    FREE(ptr);  // Should not crash or produce an error
}
END_TEST

START_TEST(test_memory_exerciser) {
    uint32_t iterations = 1000;
    struct {
        void *addr;
        uint32_t *data;
        uint32_t size;
        uint32_t crc;
    } blocks[16] = {0};

    uint32_t clock = 0;

    while (iterations--) {
        uint32_t block_size = (rand() % (24 * 1024 * 1024 - 1)) + 1; // Random size from 1 to 24MB
        char *addr = MALLOC(block_size);
        ck_assert_msg(addr != NULL, "Memory allocation failed!");

        if ((uintptr_t)addr & 0x07) {
            printf("Unaligned address %p returned!\n", addr);
            ck_assert(0);
        }

        blocks[clock].data = (uint32_t *)addr;
        blocks[clock].size = block_size;

        // Fill memory and calculate checksum
        for (uint32_t n = 0; n < (block_size >> 2); n++) {
            blocks[clock].data[n] = rand();
        }
        blocks[clock].crc = sum_block(blocks[clock].data, blocks[clock].size);

        // Verify checksums for existing blocks
        for (int n = 0; n < 16; n++) {
            if (blocks[n].addr != NULL) {
                uint32_t sum = sum_block(blocks[n].data, blocks[n].size);
                ck_assert_msg(blocks[n].crc == sum, "Checksum mismatch for block %d", n);
            }
        }

        FREE(addr);
        blocks[clock].addr = NULL; // Mark block as freed
        clock = (clock + 1) % 16; // Move to the next block
    }

    // Final free check for all remaining blocks
    for (clock = 0; clock < 16; clock++) {
        if (blocks[clock].addr != NULL) {
            FREE(blocks[clock].addr);
        }
    }
}
END_TEST



Suite* simple_malloc_suite() {
    Suite *s = suite_create("simple_malloc");
    TCase *tc_core = tcase_create("Core tests");
    tcase_set_timeout(tc_core, 120);

    tcase_add_test(tc_core, test_simple_allocation);
    tcase_add_test(tc_core, test_simple_unique_addresses);
    tcase_add_test(tc_core, test_zero_allocation);
    tcase_add_test(tc_core, test_double_free);
    tcase_add_test(tc_core, test_memory_exerciser);

    suite_add_tcase(s, tc_core);
    return s;
}

int main() {
    int number_failed;
    Suite *s = simple_malloc_suite();
    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? 0 : 1;
}
