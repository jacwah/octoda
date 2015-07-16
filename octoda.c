#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <arpa/inet.h>

#define PROGRAM_OFFSET 0x200
#define MAX_PROGRAM_SIZE (0x1000 - PROGRAM_OFFSET)
#define PRINT_ALIGN 6

static void print_ouput_header()
{
    printf("addr:  [code]  inst  arg1, arg2\n"
           "-------------------------------\n");
}

static void print_ophead(uint16_t op, const char *name, int index)
{
    assert(strlen(name) < PRINT_ALIGN);

    printf("%04X: [%04X]  %s", index * 2 + PROGRAM_OFFSET, op, name);
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

/* Print a line to stdin. index * 2 + PROGRAM_OFFSET = address */
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

/* Read a program from a file on disk. Return number of opcodes read. */
int read_file(uint16_t *program, const char *fn)
{
    FILE *fp;
    int readcount;

    fp = fopen(fn, "rb");
    if (!fp) {
        fprintf(stderr, "Can't open '%s': %s\n", fn, strerror(errno));
        return -1;
    }

    readcount = fread(program, sizeof(uint16_t), MAX_PROGRAM_SIZE, fp);
    if (readcount < 0)
        fprintf(stderr, "Failed read from '%s': %s\n", fn, strerror(errno));

    if (!feof(fp)) {
        fprintf(stderr, "File too long. Maximum size is %d.\n",
                MAX_PROGRAM_SIZE);
        readcount = -1;
    }
    fclose(fp);

    // Fix endianness issue on LE
    for (int i = 0; i < readcount; i++) {
        program[i] = htons(program[i]);
    }

    return readcount;
}

int main(int argc, char **argv)
{
    uint16_t program[MAX_PROGRAM_SIZE];
    int length;

    if (argc != 2) {
        fprintf(stderr, "8da: CHIP-8 disassembler by Jacob Wahlgren\n"
                        "usage: %s <program>\n", argv[0]);
        return 1;
    }

    memset(program, 0, sizeof(program));
    length = read_file(program, argv[1]);

    if (length < 0) {
        return 1;
    }

    print_ouput_header();
    for (int i = 0; i < length; i++) {
        print_opcode(program[i], i);
    }

    return 0;
}
