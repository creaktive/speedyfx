SpeedyFx algorithm
==================

Tokenize/hash large amount of strings efficiently.

Original Java implementation: http://www.hpl.hp.com/techreports/2008/HPL-2008-91R1.pdf

Ported to C by Stanislaw Pusep (https://github.com/creaktive)

Compile with:

    clang -lm -O2 -o speedyfx speedyfx.c

or:

    gcc -lm -O2 -o speedyfx speedyfx.c

Benchmark
=========

Test data: https://cs.fit.edu/~mmahoney/compression/enwik9.bz2

Hardware: Intel(R) Xeon(R) CPU E5620 @ 2.40GHz

Average *feature vector* build speed: **158.95 MB/s**
