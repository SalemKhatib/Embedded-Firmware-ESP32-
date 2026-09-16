/*
 * Clean SIMP core reference.
 *
 * IMPORTANT:
 * This file is intentionally NOT included in the current ESP-IDF build.
 * It is the stripped processor core being studied before integration.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define MEM_SIZE        4096
#define NUM_REGS        16
#define IOREG_SIZE      23
#define PC_MASK         0xFFFu
#define MONITOR_WIDTH   256
#define MONITOR_HEIGHT  256

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

uint32_t memory[MEM_SIZE] = {0};
uint32_t registers[NUM_REGS] = {0};
uint32_t IORegister[IOREG_SIZE] = {0};
uint8_t monitor[MONITOR_WIDTH * MONITOR_HEIGHT] = {0};

uint32_t PC = 0;
bool halted = false;
bool first_cycle_imm2 = false;

#define OPCODE(i)  (((i) >> 24) & 0xFFu)
#define RD(i)      (((i) >> 20) & 0xFu)
#define RS(i)      (((i) >> 16) & 0xFu)
#define RT(i)      (((i) >> 12) & 0xFu)
#define IMM12(i)   ((i) & 0xFFFu)

#define IMM12_SIGN_EXTEND(v) \
    (((v) & 0x800u) ? ((v) | 0xFFFFF000u) : (v))

#define USES_IMM2(i) \
    (RD(i) == 2 || RS(i) == 2 || RT(i) == 2)

uint32_t mask_ioreg_write(uint32_t addr, uint32_t value)
{
    switch (addr)
    {
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

        case 6:
        case 7:
        case 16:
            return value & PC_MASK;

        case 14:
            return value & 0x3u;

        case 15:
            return value & 0x7Fu;

        case 20:
            return value & 0xFFFFu;

        case 21:
            return value & 0xFFu;

        default:
            return value;
    }
}

void out_opcode(int rs, int rt, int rd)
{
    uint32_t addr =
        (registers[rs] + registers[rt]) % IOREG_SIZE;

    uint32_t value =
        mask_ioreg_write(addr, registers[rd]);

    IORegister[addr] = value;

    if (addr == 22 && value == 1)
    {
        IORegister[22] = 0;

        monitor[
            IORegister[20] %
            (MONITOR_WIDTH * MONITOR_HEIGHT)
        ] = IORegister[21] & 0xFFu;
    }
}

void fetch_decode_execute(void)
{
    uint32_t old_pc = PC & PC_MASK;
    PC = old_pc;

    uint32_t inst = memory[old_pc];
    uint32_t opcode = OPCODE(inst);
    uint32_t rd = RD(inst);
    uint32_t rs = RS(inst);
    uint32_t rt = RT(inst);
    bool imm2 = USES_IMM2(inst);

    uint32_t imm1_value =
        IMM12_SIGN_EXTEND(IMM12(inst));

    uint32_t imm2_value =
        imm2 ? memory[(old_pc + 1) & PC_MASK] : 0;

    registers[0] = 0;
    registers[1] = imm1_value;
    registers[2] = imm2_value;

    if (!first_cycle_imm2 && imm2)
    {
        first_cycle_imm2 = true;
        return;
    }

    if (first_cycle_imm2)
    {
        first_cycle_imm2 = false;
    }

    uint32_t addr;
    uint32_t next_pc =
        (old_pc + (imm2 ? 2u : 1u)) & PC_MASK;

    PC = next_pc;

    switch (opcode)
    {
        case op_add:
            if (rd > 2) registers[rd] = registers[rs] + registers[rt];
            break;
        case op_sub:
            if (rd > 2) registers[rd] = registers[rs] - registers[rt];
            break;
        case op_mul:
            if (rd > 2) registers[rd] = registers[rs] * registers[rt];
            break;
        case op_mac:
            if (rd > 2) registers[rd] = registers[rs] * registers[rt] + registers[rd];
            break;
        case op_and:
            if (rd > 2) registers[rd] = registers[rs] & registers[rt];
            break;
        case op_or:
            if (rd > 2) registers[rd] = registers[rs] | registers[rt];
            break;
        case op_xor:
            if (rd > 2) registers[rd] = registers[rs] ^ registers[rt];
            break;
        case op_sll:
            if (rd > 2) registers[rd] = registers[rs] << (registers[rt] & 31u);
            break;
        case op_sra:
            if (rd > 2) registers[rd] = (uint32_t)(((int32_t)registers[rs]) >> (registers[rt] & 31u));
            break;
        case op_srl:
            if (rd > 2) registers[rd] = registers[rs] >> (registers[rt] & 31u);
            break;
        case op_beq:
            PC = (registers[rs] == registers[rt]) ? (registers[rd] & PC_MASK) : next_pc;
            break;
        case op_bne:
            PC = (registers[rs] != registers[rt]) ? (registers[rd] & PC_MASK) : next_pc;
            break;
        case op_blt:
            PC = ((int32_t)registers[rs] < (int32_t)registers[rt]) ? (registers[rd] & PC_MASK) : next_pc;
            break;
        case op_bgt:
            PC = ((int32_t)registers[rs] > (int32_t)registers[rt]) ? (registers[rd] & PC_MASK) : next_pc;
            break;
        case op_ble:
            PC = ((int32_t)registers[rs] <= (int32_t)registers[rt]) ? (registers[rd] & PC_MASK) : next_pc;
            break;
        case op_bge:
            PC = ((int32_t)registers[rs] >= (int32_t)registers[rt]) ? (registers[rd] & PC_MASK) : next_pc;
            break;
        case op_jal:
            if (rd > 2) registers[rd] = next_pc;
            PC = registers[rs] & PC_MASK;
            break;
        case op_lw:
            if (rd > 2) registers[rd] = memory[(registers[rs] + registers[rt]) & PC_MASK];
            break;
        case op_sw:
            memory[(registers[rs] + registers[rt]) & PC_MASK] = registers[rd];
            break;
        case op_reti:
            PC = IORegister[7] & PC_MASK;
            break;
        case op_in:
            addr = (registers[rs] + registers[rt]) % IOREG_SIZE;
            if (rd > 2) registers[rd] = IORegister[addr];
            break;
        case op_out:
            out_opcode(rs, rt, rd);
            break;
        case op_halt:
            halted = true;
            break;
        default:
            printf(
                "Unknown SIMP opcode %lu at PC %03lX\n",
                (unsigned long)opcode,
                (unsigned long)old_pc
            );
            halted = true;
            break;
    }

    registers[0] = 0;
    registers[1] = imm1_value;
    registers[2] = imm2_value;
}
