#include <sys/mman.h>
#include <sys/stat.h>
#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * SpeedyFx algorithm
 * ==================
 *
 * Tokenize/hash large amount of strings efficiently.
 * Original Java implementation: http://www.hpl.hp.com/techreports/2008/HPL-2008-91R1.pdf
 * Ported to C by Stanislaw Pusep (https://github.com/creaktive)
 * Compile with:
 *      clang -lm -O3 -o speedyfx speedyfx.c
 * or:
 *      gcc -lm -O3 -o speedyfx speedyfx.c
 * Then use as:
 *      ./speedyfx enwik9 > fv.bin
 * To generate 128KB feature vector for enwik9 text file.
 *
 * Benchmark
 * =========
 *
 * Test data: https://cs.fit.edu/~mmahoney/compression/enwik9.bz2
 * Hardware: Intel(R) Xeon(R) CPU E5620 @ 2.40GHz
 * Average feature vector build speed: 213.83 MB/s
 */

#define MAP_SIZE 256

static unsigned int length = MAP_SIZE;
static unsigned int code_table[MAP_SIZE];

void speedyfx_init(unsigned int seed) {
    unsigned int i;
    unsigned int fold_table[MAP_SIZE];
    unsigned int rand_table[MAP_SIZE];

    fold_table[0] = 0;
    rand_table[0] = seed;
    for (i = 1; i < length; i++) {
        fold_table[i] = isalnum(i) ? tolower(i) : 0;
        rand_table[i]
            = (
                rand_table[i - 1]
                * 0x10a860c1
            ) % 0xfffffffb;
    }

    for (i = 0; i < length; i++)
        if (fold_table[i])
            code_table[i] = rand_table[fold_table[i]];
}

#define SetBit(a, b) (((unsigned char *) a)[(b) >> 3] |= (1 << ((b) & 7)))

unsigned char *speedyfx_fv(const unsigned char *s, unsigned int n) {
    unsigned int code, c;
    unsigned int wordhash = 0;
    unsigned char *fv;

    fv = calloc(ceil((float) n / 8.0), sizeof(unsigned char));

    while (*s) {
        c = *s++;
        if ((code = code_table[c % length]) != 0)
            wordhash = (wordhash >> 1) + code;
        else if (wordhash) {
            SetBit(fv, wordhash % n);
            wordhash = 0;
        }
    }

    if (wordhash)
        SetBit(fv, wordhash % n);

    return fv;
}

int main(int argc, char **argv) {
    int fd = -1;
    size_t fv_length = 1024 * 1024;
    unsigned char *data, *fv;
    struct stat statbuf;

    if (argc != 2) {
        printf("usage: %s filename\n", argv[0]);
        return 1;
    }

    if ((fd = open(argv[1], O_RDONLY, 0)) == -1)
        err(1, "can't open() %s", argv[1]);

    if (fstat(fd, &statbuf) < 0)
        err(1, "can't fstat() %s", argv[1]);

    if ((data = mmap(NULL, statbuf.st_size + 1, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0)) == MAP_FAILED)
        err(1, "can't mmap() %s", argv[1]);

    speedyfx_init(0xdeadbeef);
    fv = speedyfx_fv(data, fv_length);

    fwrite(fv, fv_length >> 3, 1, stdout);

    free(fv);
    munmap(data, statbuf.st_size + 1);
    close(fd);
    return 0;
}
