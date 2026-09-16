#pragma once

#include <stddef.h>
#include <stdint.h>


/*
 * Reset the entire simulated SIMP machine.
 */
void simp_reset(void);


/*
 * Copy a SIMP machine-code program
 * into the simulated memory.
 */
void simp_load_program(
    const uint32_t *program,
    size_t word_count
);


/*
 * Execute SIMP instructions.
 *
 * max_cycles protects us from accidentally
 * running forever if the program never HALTs.
 *
 * Returns:
 * 1 = program reached HALT
 * 0 = cycle limit reached
 */
int simp_run(uint32_t max_cycles);


/*
 * Get the SIMP clock count.
 */
uint32_t simp_get_cycles(void);


/*
 * Get the completed virtual 256x256 monitor.
 */
const uint8_t *simp_get_monitor(void);