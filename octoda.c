#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_PROGRAM_SIZE (0x1000 - 0x200)

void print_opcode(uint16_t op)
{
    printf("%X\n", op);
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
        fprintf(stderr, "usage: %s <program>\n", argv[0]);
        return 1;
    }

    memset(program, 0, sizeof(program));
    length = read_file(program, argv[1]);

    if (length < 0) {
        return 1;
    }

    for (int i = 0; i < length; i++) {
        print_opcode(program[i]);
    }

    return 0;
}
