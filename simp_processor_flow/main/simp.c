#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <string.h>

#include "simp.h"

/*
 * ============================================================
 * SIMP architecture constants
 * ============================================================
 */

#define MEM_SIZE        4096
#define NUM_REGS        16
#define IOREG_SIZE      23

#define PC_MASK         0xFFFu

#define MONITOR_WIDTH   256
#define MONITOR_HEIGHT  256


/*
 * ============================================================
 * SIMP opcodes
 * ============================================================
 */

typedef enum
{
    op_add  = 0,
    op_sub  = 1,
    op_mul  = 2,
    op_mac  = 3,

    op_and  = 4,
    op_or   = 5,
    op_xor  = 6,

    op_sll  = 7,
    op_sra  = 8,
    op_srl  = 9,

    op_beq  = 10,
    op_bne  = 11,
    op_blt  = 12,
    op_bgt  = 13,
    op_ble  = 14,
    op_bge  = 15,

    op_jal  = 16,

    op_lw   = 17,
    op_sw   = 18,

    op_reti = 19,

    op_in   = 20,
    op_out  = 21,

    op_halt = 22

} Opcode;


/*
 * ============================================================
 * SIMP simulated hardware state
 * ============================================================
 */

/*
 * Main SIMP memory.
 *
 * 4096 words.
 */
uint32_t memory[MEM_SIZE] = {0};


/*
 * SIMP register file.
 *
 * register 0 = $zero
 * register 1 = $imm1
 * register 2 = $imm2
 */
uint32_t registers[NUM_REGS] = {0};


/*
 * SIMP I/O registers.
 *
 * Important monitor registers:
 *
 * 20 = monitoraddr
 * 21 = monitordata
 * 22 = monitorcmd
 */
uint32_t IORegister[IOREG_SIZE] = {0};


/*
 * SIMP virtual monitor.
 *
 * 256 x 256 pixels.
 * Each pixel is one byte.
 */
uint8_t monitor[MONITOR_WIDTH * MONITOR_HEIGHT] = {0};


/*
 * Program Counter.
 */
uint32_t PC = 0;


/*
 * Becomes true when SIMP executes HALT.
 */
bool halted = false;


/*
 * Tracks the first cycle of a two-word instruction
 * that uses $imm2.
 */
bool first_cycle_imm2 = false;


/*
 * ============================================================
 * Instruction decoding
 * ============================================================
 *
 * SIMP instruction format:
 *
 * 31          24 23  20 19  16 15  12 11           0
 * +-------------+------+------+------+---------------+
 * |   opcode    |  rd  |  rs  |  rt  |     imm12     |
 * +-------------+------+------+------+---------------+
 */

#define OPCODE(i)  (((i) >> 24) & 0xFFu)
#define RD(i)      (((i) >> 20) & 0xFu)
#define RS(i)      (((i) >> 16) & 0xFu)
#define RT(i)      (((i) >> 12) & 0xFu)
#define IMM12(i)   ((i) & 0xFFFu)


/*
 * Sign-extend a 12-bit immediate to 32 bits.
 */
#define IMM12_SIGN_EXTEND(v) \
    (((v) & 0x800u) ? ((v) | 0xFFFFF000u) : (v))


/*
 * A SIMP instruction uses a second 32-bit word
 * whenever $imm2 (register 2) appears in rd, rs or rt.
 */
#define USES_IMM2(i) \
    (RD(i) == 2 || RS(i) == 2 || RT(i) == 2)


/*
 * ============================================================
 * Function declarations
 * ============================================================
 */

uint32_t mask_ioreg_write(uint32_t addr, uint32_t value);

void out_opcode(int rs, int rt, int rd);

void fetch_decode_execute(void);


/*
 * ============================================================
 * SIMP I/O register write masking
 * ============================================================
 */

uint32_t mask_ioreg_write(uint32_t addr, uint32_t value)
{
    switch (addr)
    {
        /*
         * 1-bit registers
         */
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 11:
        case 17:
        case 22:
            return value & 0x1u;

        /*
         * 12-bit address registers
         */
        case 6:
        case 7:
        case 16:
            return value & PC_MASK;

        /*
         * diskcmd
         *
         * Disk is not implemented in our ESP32 V1,
         * but keeping the original register width is harmless.
         */
        case 14:
            return value & 0x3u;

        /*
         * disksector
         */
        case 15:
            return value & 0x7Fu;

        /*
         * monitoraddr
         */
        case 20:
            return value & 0xFFFFu;

        /*
         * monitordata
         */
        case 21:
            return value & 0xFFu;

        default:
            return value;
    }
}


/*
 * ============================================================
 * Execute SIMP OUT instruction
 * ============================================================
 */

void out_opcode(int rs, int rt, int rd)
{
    /*
     * Calculate which SIMP I/O register
     * the OUT instruction is writing.
     */
    uint32_t addr =
        (registers[rs] + registers[rt]) % IOREG_SIZE;


    /*
     * Get the value that will be written.
     */
    uint32_t value =
        mask_ioreg_write(addr, registers[rd]);


    /*
     * Write to the simulated SIMP I/O register.
     */
    IORegister[addr] = value;


    /*
     * monitorcmd = I/O register 22.
     *
     * Writing 1 means:
     *
     * "write the pixel now"
     */
    if (addr == 22 && value == 1)
    {
        /*
         * Hardware automatically clears monitorcmd.
         */
        IORegister[22] = 0;


        /*
         * monitoraddr = I/O register 20
         * monitordata = I/O register 21
         *
         * So:
         *
         * monitor[address] = pixel data
         */
        monitor[
            IORegister[20] %
            (MONITOR_WIDTH * MONITOR_HEIGHT)
        ] = IORegister[21] & 0xFFu;
    }
}


/*
 * ============================================================
 * Fetch / Decode / Execute
 *
 * One call represents one SIMP clock cycle.
 * ============================================================
 */

void fetch_decode_execute(void)
{
    /*
     * --------------------------------------------------------
     * FETCH
     * --------------------------------------------------------
     */

    uint32_t old_pc = PC & PC_MASK;

    PC = old_pc;

    uint32_t inst = memory[old_pc];


    /*
     * --------------------------------------------------------
     * DECODE
     * --------------------------------------------------------
     */

    uint32_t opcode = OPCODE(inst);

    uint32_t rd = RD(inst);
    uint32_t rs = RS(inst);
    uint32_t rt = RT(inst);


    /*
     * Does this instruction use $imm2?
     */
    bool imm2 = USES_IMM2(inst);


    /*
     * $imm1 comes from the lower 12 bits
     * of the instruction.
     */
    uint32_t imm1_value =
        IMM12_SIGN_EXTEND(IMM12(inst));


    /*
     * If $imm2 is used, the next memory word
     * contains the full 32-bit immediate.
     */
    uint32_t imm2_value =
        imm2
        ? memory[(old_pc + 1) & PC_MASK]
        : 0;


    /*
     * Set the three special registers.
     */
    registers[0] = 0;
    registers[1] = imm1_value;
    registers[2] = imm2_value;


    /*
     * --------------------------------------------------------
     * Two-word instruction timing
     * --------------------------------------------------------
     */

    /*
     * First cycle:
     *
     * detect the second word,
     * but do NOT execute yet.
     */
    if (!first_cycle_imm2 && imm2)
    {
        first_cycle_imm2 = true;
        return;
    }


    /*
     * Second cycle:
     *
     * now the instruction may execute.
     */
    if (first_cycle_imm2)
    {
        first_cycle_imm2 = false;
    }


    /*
     * Temporary address used by IN.
     */
    uint32_t addr;


    /*
     * Normal instruction:
     *
     * PC + 1
     *
     * Two-word instruction:
     *
     * PC + 2
     */
    uint32_t next_pc =
        (old_pc + (imm2 ? 2u : 1u)) & PC_MASK;


    /*
     * Default PC behavior.
     *
     * Branches and jumps may override this.
     */
    PC = next_pc;


    /*
     * --------------------------------------------------------
     * EXECUTE
     * --------------------------------------------------------
     */

    switch (opcode)
    {
        /*
         * Arithmetic
         */

        case op_add:
            if (rd > 2)
            {
                registers[rd] =
                    registers[rs] + registers[rt];
            }
            break;


        case op_sub:
            if (rd > 2)
            {
                registers[rd] =
                    registers[rs] - registers[rt];
            }
            break;


        case op_mul:
            if (rd > 2)
            {
                registers[rd] =
                    registers[rs] * registers[rt];
            }
            break;


        case op_mac:
            if (rd > 2)
            {
                registers[rd] =
                    registers[rs] * registers[rt]
                    + registers[rd];
            }
            break;


        /*
         * Logical operations
         */

        case op_and:
            if (rd > 2)
            {
                registers[rd] =
                    registers[rs] & registers[rt];
            }
            break;


        case op_or:
            if (rd > 2)
            {
                registers[rd] =
                    registers[rs] | registers[rt];
            }
            break;


        case op_xor:
            if (rd > 2)
            {
                registers[rd] =
                    registers[rs] ^ registers[rt];
            }
            break;


        /*
         * Shifts
         */

        case op_sll:
            if (rd > 2)
            {
                registers[rd] =
                    registers[rs]
                    << (registers[rt] & 31u);
            }
            break;


        case op_sra:
            if (rd > 2)
            {
                registers[rd] =
                    (uint32_t)(
                        ((int32_t)registers[rs])
                        >> (registers[rt] & 31u)
                    );
            }
            break;


        case op_srl:
            if (rd > 2)
            {
                registers[rd] =
                    registers[rs]
                    >> (registers[rt] & 31u);
            }
            break;


        /*
         * Branches
         */

        case op_beq:
            PC =
                (registers[rs] == registers[rt])
                ? (registers[rd] & PC_MASK)
                : next_pc;
            break;


        case op_bne:
            PC =
                (registers[rs] != registers[rt])
                ? (registers[rd] & PC_MASK)
                : next_pc;
            break;


        case op_blt:
            PC =
                ((int32_t)registers[rs] <
                 (int32_t)registers[rt])
                ? (registers[rd] & PC_MASK)
                : next_pc;
            break;


        case op_bgt:
            PC =
                ((int32_t)registers[rs] >
                 (int32_t)registers[rt])
                ? (registers[rd] & PC_MASK)
                : next_pc;
            break;


        case op_ble:
            PC =
                ((int32_t)registers[rs] <=
                 (int32_t)registers[rt])
                ? (registers[rd] & PC_MASK)
                : next_pc;
            break;


        case op_bge:
            PC =
                ((int32_t)registers[rs] >=
                 (int32_t)registers[rt])
                ? (registers[rd] & PC_MASK)
                : next_pc;
            break;


        /*
         * Jump and link
         */

        case op_jal:
            if (rd > 2)
            {
                registers[rd] = next_pc;
            }

            PC = registers[rs] & PC_MASK;
            break;


        /*
         * Load
         */

        case op_lw:
            if (rd > 2)
            {
                registers[rd] =
                    memory[
                        (registers[rs] + registers[rt])
                        & PC_MASK
                    ];
            }
            break;


        /*
         * Store
         */

        case op_sw:
            memory[
                (registers[rs] + registers[rt])
                & PC_MASK
            ] = registers[rd];
            break;


        /*
         * Return from interrupt.
         *
         * Full interrupt support is not implemented
         * in our ESP32 V1.
         *
         * We keep the opcode behavior so the SIMP ISA
         * itself remains recognizable.
         */
        case op_reti:
            PC = IORegister[7] & PC_MASK;
            break;


        /*
         * Read SIMP I/O register.
         */

        case op_in:
            addr =
                (registers[rs] + registers[rt])
                % IOREG_SIZE;

            if (rd > 2)
            {
                registers[rd] = IORegister[addr];
            }

            break;


        /*
         * Write SIMP I/O register.
         */

        case op_out:
            out_opcode(rs, rt, rd);
            break;


        /*
         * Stop SIMP execution.
         */

        case op_halt:
            halted = true;
            break;


        /*
         * Invalid instruction.
         */

        default:
            printf(
                "Unknown SIMP opcode %lu at PC %03lX\n",
                (unsigned long)opcode,
                (unsigned long)old_pc
            );

            halted = true;
            break;
    }


    /*
     * --------------------------------------------------------
     * Protect special registers
     * --------------------------------------------------------
     */

    registers[0] = 0;
    registers[1] = imm1_value;
    registers[2] = imm2_value;
}
void simp_reset(void)
{
    /*
     * Clear SIMP memory.
     */
    memset(
        memory,
        0,
        sizeof(memory)
    );


    /*
     * Clear all registers.
     */
    memset(
        registers,
        0,
        sizeof(registers)
    );


    /*
     * Clear the simulated I/O registers.
     */
    memset(
        IORegister,
        0,
        sizeof(IORegister)
    );


    /*
     * Clear the virtual 256x256 monitor.
     */
    memset(
        monitor,
        0,
        sizeof(monitor)
    );


    /*
     * Reset CPU state.
     */
    PC = 0;
    halted = false;
    first_cycle_imm2 = false;
}


void simp_load_program(
    const uint32_t *program,
    size_t word_count
)
{
    /*
     * Don't allow a program larger
     * than SIMP's 4096-word memory.
     */
    if (word_count > MEM_SIZE)
    {
        word_count = MEM_SIZE;
    }


    /*
     * Copy machine-code words
     * into SIMP memory starting at address 0.
     */
    memcpy(
        memory,
        program,
        word_count * sizeof(uint32_t)
    );
}


int simp_run(uint32_t max_cycles)
{
    uint32_t cycles = 0;


    while (!halted && cycles < max_cycles)
    {
        /*
         * Execute one simulated
         * SIMP clock cycle.
         */
        fetch_decode_execute();


        /*
         * I/O register 8 is SIMP's
         * simulated clock counter.
         */
        IORegister[8]++;


        cycles++;
    }


    /*
     * Did the SIMP program actually HALT?
     */
    return halted ? 1 : 0;
}


uint32_t simp_get_cycles(void)
{
    return IORegister[8];
}


const uint8_t *simp_get_monitor(void)
{
    return monitor;
}