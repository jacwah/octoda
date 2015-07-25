/* octoda - a disassembler for CHIP-8 programs
 * Copyright 2015 Jacob Wahlgren

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define PROGRAM_OFFSET 0x200
#define MAX_PROGRAM_SIZE (0x1000 - PROGRAM_OFFSET)
#define OPCODE_SIZE sizeof(uint16_t)
#define PRINT_ALIGN 6

/*** EXTRACT_XXXX: Extract macros ***
 * Extract part of an opcode and right shift it
 * Example: EXTRACT_X000(0x1234) -> 0x0001
 * Example: EXTRACT_0XX0(0x1234) -> 0x0023
 ***                              ***/
#define EXTRACT_X000(x) (((x) & 0xF000) >> 12)
#define EXTRACT_0XXX(x) ((x) & 0x0FFF)
#define EXTRACT_0X00(x) (((x) & 0x0F00) >> 8)
#define EXTRACT_00X0(x) (((x) & 0x00F0) >> 4)
#define EXTRACT_000X(x) ((x) & 0x000F)
#define EXTRACT_00XX(x) ((x) & 0x00FF)
#define EXTRACT_0XX0(x) (((x) & 0x0FF0) >> 4)

// Make string constant from macro expanstion
// More info at https://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define XSTR(s) #s
#define STR(s) XSTR(s)

#define MAX_NAME_LEN 8
#define MAX_ARG_LEN 4
#define MAX_ARG_COUNT 3

enum data_type { DATA = 0, CODE };

/* CHIP-8 object
 *
 * index: index of the first byte of the object in accomanying program array
 * size: DATA objects have variable length. CODE objects must have a size of
 *       OPCODE_SIZE
 */
struct c8obj {
    enum data_type type;
    int index;
    int size;
    char name[MAX_NAME_LEN];
    char args[MAX_ARG_COUNT][MAX_ARG_LEN];
};

static void print_ouput_header()
{
    printf("addr: [code]  inst  arg1, arg2\n"
           "-------------------------------\n");
}

static void print_ophead(uint16_t op, const char *name, int index)
{
    assert(strlen(name) < PRINT_ALIGN);

    printf("%04X: [%04X]  %s", index + PROGRAM_OFFSET, op, name);
    for (int i = 0; i < PRINT_ALIGN - strlen(name); i++)
        printf(" ");
}

static void print_noarg(uint16_t op, const char *name, int index)
{
    print_ophead(op, name, index);
    printf("\n");
}

static void print_addr(uint16_t op, const char *name, int index)
{
    print_ophead(op, name, index);
    printf("%X\n", op & 0x0FFF);
}

static void print_vx_nn(uint16_t op, const char *name, int index)
{
    print_ophead(op, name, index);
    printf("V%X, %X\n", (op & 0x0F00) >> 8, op & 0x00FF);
}

static void print_vx_vy(uint16_t op, const char *name, int index)
{
    print_ophead(op, name, index);
    printf("V%X, V%X\n", (op & 0x0F00) >> 8, (op & 0x00F0) >> 4);
}

static void print_vx_vy_n(uint16_t op, const char *name, int index)
{
    print_ophead(op, name, index);
    printf("V%X, V%X, %X\n", (op & 0x0F00) >> 8,
                             (op & 0x00F0) >> 4,
                              op & 0x000F);
}

static void print_vx(uint16_t op, const char *name, int index)
{
    print_ophead(op, name, index);
    printf("V%X\n", (op & 0x0F00) >> 8);
}

static void print_invalid(uint16_t op, int index)
{
    print_noarg(op, "INVALID", index);
}

/* Print a line to stdin. index + PROGRAM_OFFSET = address */
void print_opcode(uint16_t op, int index)
{
    switch ((op & 0xF000) >> 12) {
    case 0x0:
        if (op == 0x00E0)
            print_noarg(op, "CLS", index);
        else if (op == 0x00EE)
            print_noarg(op, "RET", index);
        else
            print_addr(op, "SYS", index);
        break;
    case 0x1:
        print_addr(op, "JMP", index);
        break;
    case 0x2:
        print_addr(op, "CALL", index);
        break;
    case 0x3:
        print_vx_nn(op, "SE", index);
        break;
    case 0x4:
        print_vx_nn(op, "SNE", index);
        break;
    case 0x5:
        if ((op & 0x000F) == 0)
            print_vx_vy(op, "SE", index);
        else
            print_invalid(op, index);
        break;
    case 0x6:
        print_vx_nn(op, "LD", index);
        break;
    case 0x7:
        print_vx_nn(op, "ADD", index);
        break;
    case 0x8:
        switch (op & 0x000F) {
        case 0x0:
            print_vx_vy(op, "LD", index);
            break;
        case 0x1:
            print_vx_vy(op, "OR", index);
            break;
        case 0x2:
            print_vx_vy(op, "AND", index);
            break;
        case 0x3:
            print_vx_vy(op, "XOR", index);
            break;
        case 0x4:
            print_vx_vy(op, "ADD", index);
            break;
        case 0x5:
            print_vx_vy(op, "SUB", index);
            break;
        case 0x6:
            print_vx_vy(op, "SHR", index);
            break;
        case 0x7:
            print_vx_vy(op, "SUBN", index);
            break;
        case 0xE:
            print_vx_vy(op, "SHL", index);
            break;
        default:
            print_invalid(op, index);
        }
        break;
    case 0x9:
        print_vx_vy(op, "SNE", index);
        break;
    case 0xA:
        print_addr(op, "LDI", index);
        break;
    case 0xB:
        print_addr(op, "JPO", index);
        break;
    case 0xC:
        print_vx_nn(op, "RND", index);
        break;
    case 0xD:
        print_vx_vy_n(op, "DRW", index);
        break;
    case 0xE:
        if ((op & 0x00FF) == 0x009E)
            print_vx(op, "SKP", index);
        else if ((op &0x00FF) == 0x00A1)
            print_vx(op, "SKNP", index);
        else
            print_invalid(op, index);
        break;
    case 0xF:
        print_ophead(op, "LD", index);
        printf("NOT IMPLEMENTED\n");
        break;
/*

Fx07 - LD Vx, DT
Set Vx = delay timer value.

The value of DT is placed into Vx.


Fx0A - LD Vx, K
Wait for a key press, store the value of the key in Vx.

All execution stops until a key is pressed, then the value of that key is stored in Vx.


Fx15 - LD DT, Vx
Set delay timer = Vx.

DT is set equal to the value of Vx.


Fx18 - LD ST, Vx
Set sound timer = Vx.

ST is set equal to the value of Vx.


Fx1E - ADD I, Vx
Set I = I + Vx.

The values of I and Vx are added, and the results are stored in I.


Fx29 - LD F, Vx
Set I = location of sprite for digit Vx.

The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx. See section 2.4, Display, for more information on the Chip-8 hexadecimal font.


Fx33 - LD B, Vx
Store BCD representation of Vx in memory locations I, I+1, and I+2.

The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.


Fx55 - LD [I], Vx
Store registers V0 through Vx in memory starting at location I.

The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.


Fx65 - LD Vx, [I]
Read registers V0 through Vx from memory starting at location I.

The interpreter reads values from memory starting at location I into registers V0 through Vx.
*/
    }
}


#define IS_JUMP(op) (((op) & 0xF000) == 0x1000)
#define IS_CALL(op) (((op) & 0xF000) == 0x2000)
#define IS_STOP(op) ((op) == 0x00EE)    // RET
#define IS_SKIP(op) (((op) & 0xF000) == 0x3000  /* SE   */ \
                  || ((op) & 0xF000) == 0x4000  /* SNE  */ \
                  || ((op) & 0xF00F) == 0x5000  /* SE   */ \
                  || ((op) & 0xF00F) == 0x9000  /* SNE  */ \
                  || ((op) & 0xF0FF) == 0xE09E  /* SKP  */ \
                  || ((op) & 0xF0FF) == 0xE0A1) /* SKNP */

/* Try to infer what is code and what is data.
 * Traverse code to figure out what is reachable by code.
 * The BNNN opcode makes this impossible, so this won't work for programs using
 * that particular code.
 */
int discover_data_types(enum data_type *types, uint8_t *program,
                        int length, int index)
{
    if (index < 0 || index >= MAX_PROGRAM_SIZE) {
        fprintf(stderr, "Index out of bounds! (%X)\n", index);
        return 1;
    }

    while (index < length
        && types[index] != CODE) {
        uint16_t op = program[index] << 8 | program[index + 1];

        types[index] = CODE;
        types[index + 1] = CODE;

        // Will crash if jump to something below PROGRAM_OFFSET
        if (IS_JUMP(op)) {
            index = EXTRACT_0XXX(op) - PROGRAM_OFFSET;
        } else if (IS_STOP(op)) {
            // Exit loop by not incrementing
        } else {
            if (IS_SKIP(op)) {
                discover_data_types(types, program, length,
                                    index + OPCODE_SIZE * 2);
            } else if (IS_CALL(op)) {
                discover_data_types(types, program, length,
                                    EXTRACT_0XXX(op) - PROGRAM_OFFSET);
            }

            index += OPCODE_SIZE;
        }
    }

    return 0;
}

/* Fill in a struct c8obj starting at index in program[].
 * Return the size of the object. (index + size == index of next object)
 */
int c8obj_create(struct c8obj *objp, const enum data_type *types,
                 const uint8_t *program, int index, int program_len)
{
    enum data_type type = types[index];
    int size = 0;

    assert(index >= 0 && index < program_len);

    objp->index = index;
    objp->type = type;

    if (type == DATA) {
        size = 0;

        while (index + size < program_len
            && types[index + size] == DATA) {
            size++;
        }

        objp->size = size;
        strcpy(objp->name, "DATA");
    } else {
        if (index + 1 >= program_len) {
            fprintf(stderr, "Partial opcode!\n");
            size = -1;
        } else {
            size = OPCODE_SIZE;
            objp->size = size;
            strcpy(objp->name, "CODE");
        }
    }

    return size;
}

void c8obj_data_print(const struct c8obj *objp, const uint8_t *program)
{
    int index = objp->index;
    int size = objp->size;

    assert(size > 0);

    printf("%04X: -DATA-  ", index + PROGRAM_OFFSET);

    for (int i = 0; i < size - 1; i++) {
        printf("%02X, ", program[index + i]);
    }

    printf("%02X\n", program[index + size - 1]);
}

void c8obj_print(const struct c8obj *objp, const uint8_t *program)
{
    if (objp->type == DATA) {
        c8obj_data_print(objp, program);
    } else {
        int index = objp->index;
        uint16_t value = program[index] << 8 | program[index + 1];
        const char *name = objp->name;

        // Left-align name with MAX_NAME_LEN spaces
        printf("%04X: [%04X]  %-" STR(MAX_NAME_LEN) "s\n",
               index + PROGRAM_OFFSET, value, name);
    }
}

void program_print(const enum data_type *types, const uint8_t *program,
                   int length)
{
    int index = 0;

    while (index < length) {
        struct c8obj obj;

        index += c8obj_create(&obj, types, program, index, length);
        c8obj_print(&obj, program);
    }
}

/* Read a program from a file on disk. Return number of opcodes read. */
int read_file(uint8_t *program, const char *fn)
{
    FILE *fp;
    int readcount;

    fp = fopen(fn, "rb");
    if (!fp) {
        fprintf(stderr, "Can't open '%s': %s\n", fn, strerror(errno));
        return -1;
    }

    readcount = fread(program, sizeof(uint8_t), MAX_PROGRAM_SIZE, fp);

    if (readcount < 1) {
        if (readcount == 0 && feof(fp))
            fprintf(stderr, "File empty.\n");
        else
            fprintf(stderr, "Failed read from '%s': %s\n", fn, strerror(errno));
        readcount = -1;
    } else if (!feof(fp)) {
        fprintf(stderr, "File too long. Maximum size is %d.\n",
                MAX_PROGRAM_SIZE);
        readcount = -1;
    }

    fclose(fp);

    return readcount;
}

int main(int argc, char **argv)
{
    uint8_t program[MAX_PROGRAM_SIZE] = { 0 };
    enum data_type types[MAX_PROGRAM_SIZE] = { DATA };
    int length = 0;

    if (argc != 2) {
        fprintf(stderr, "8da: CHIP-8 disassembler by Jacob Wahlgren\n"
                        "usage: %s <program>\n", argv[0]);
        return 1;
    }

    length = read_file(program, argv[1]);

    if (length < 0) {
        return 1;
    }

    print_ouput_header();
    discover_data_types(types, program, length, 0);
    program_print(types, program, length);

    return 0;
}
