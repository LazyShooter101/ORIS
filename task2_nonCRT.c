#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#define N_REGS 16
#define P_ADR 0
#define Q_ADR 1
#define N_ADR 2
#define D_ADR 3
#define M_ADR 4
#define ONE_ADR 5
#define COMP_ADR_1 6
#define A_ADR 7
#define B_ADR 8
#define X_ADR 9
#define Y_ADR 10
#define C_ADR 11
#define POW_ADR 12
#define POW_ADR2 13

#define l 512

unsigned long copro_clock;
unsigned long f;
mpz_t R[N_REGS];

/*
All of these are pretty much identical to
those found in task2_coprocessor.py
*/

void copro_init() {
    copro_clock = 0;
    for (int i=0; i<N_REGS; ++i) {
        mpz_init(R[i]);
    }
}

void copro_completeCycle(int reg) {
    copro_clock++;
    if (copro_clock == f) {
        // flip a random bit
        mpz_t flip_mask;
        mpz_init(flip_mask);
        mpz_setbit(flip_mask, rand()%l);
        mpz_xor(R[reg], R[reg], flip_mask);
        mpz_clear(flip_mask);
    }
}

void copro_add(int x, int y, int z, int N) {
    mpz_add(R[x], R[y], R[z]);
    mpz_mod(R[x], R[x], R[N]);
    copro_completeCycle(x);
}

void copro_sub(int x, int y, int z, int N) {
    mpz_sub(R[x], R[y], R[z]);
    mpz_mod(R[x], R[x], R[N]);
    copro_completeCycle(x);
}

void copro_mul(int x, int y, int z, int N, int dont_use_clock) {
    mpz_mul(R[x], R[y], R[z]);
    mpz_mod(R[x], R[x], R[N]);
    if (dont_use_clock) {
        return;
    }
    copro_completeCycle(x);
}

void copro_mul_inverse(int x, int y, int N) {
    mpz_t gcd, dummy;
    mpz_init(gcd);
    mpz_init(dummy);
    mpz_gcdext(gcd, R[x], dummy, R[y], R[N]);
    if (mpz_cmp_ui(gcd, 1) == 0) { // gcd == 1
        mpz_mod(R[x], R[x], R[N]);
    } else {
        mpz_set_ui(R[x], 0);
    }

    mpz_clear(dummy);
    mpz_clear(gcd);
    copro_completeCycle(x);
}

void copro_add_inverse(int x, int y, int N) {
    mpz_mod(R[y], R[y], R[N]);
    mpz_sub(R[x], R[N], R[y]);
    copro_completeCycle(x);
}

void copro_copy_mod(int x, int y, int N) {
    mpz_mod(R[x], R[y], R[N]);
    copro_completeCycle(x);
}

void copro_copy(int x, int y, int skip) {
    mpz_set(R[x], R[y]);
    if (skip) {return;}
    copro_completeCycle(x);
}

void copro_load_immediate(int x, mpz_t *val) {
    mpz_set(R[x], *val);
    copro_completeCycle(x);
}

void copro_print_adr(int x) {
    mpz_out_str(stdout, 10, R[x]);
    printf("\n");
}

void pow_on_copro(int x, int y, int z, int N) {
    // uses left to right binary exp again
    copro_copy(POW_ADR, ONE_ADR, 0);
    copro_copy(POW_ADR2, y, 0);
    for (int k=0; k<l; ++k) { // loop thru bits
        if (mpz_tstbit(R[z], k) == 1) {
            copro_mul(POW_ADR, POW_ADR, POW_ADR2, N, 1);
        }
        copro_mul(POW_ADR2, POW_ADR2, POW_ADR2, N, 1);
        copro_completeCycle(POW_ADR);
    }
    f = 999999;
    copro_copy(x, POW_ADR, 1);
}

void rsaSign(mpz_t *p, mpz_t *q, mpz_t *N, mpz_t *d, mpz_t *m, mpz_t *c) {
    // "realistic" version

    // initialisation and loading args into 'coprocessor'
    copro_init();

    copro_load_immediate(N_ADR, N);
    copro_load_immediate(D_ADR, d);
    copro_load_immediate(M_ADR, m);

    mpz_t one;
    mpz_init_set_ui(one, 1);
    copro_load_immediate(ONE_ADR, &one);
    mpz_clear(one);

    // compute c = m^d (mod N)
    pow_on_copro(C_ADR, M_ADR, D_ADR, N_ADR);

    mpz_set(*c, R[C_ADR]);
}

int main() {
    srand(time(NULL));
    int i;
    for (int j=0; j<3; ++j) {
        int stop = 10+rand()%100;
        for (i=0; i<stop; ++i) {
            rand();
        }
    }

    mpz_t p, q, N, e, d, m, f_mpz, c;
    mpz_inits(p, q, N, e, d, m, f_mpz, c);

    char inputStr[1024];
    int flag; // for parsing str to mpz_t
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(p, inputStr, 10);
    assert(flag == 0);
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(q, inputStr, 10);
    assert(flag == 0);
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(N, inputStr, 10);
    assert(flag == 0);
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(d, inputStr, 10);
    assert(flag == 0);
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(m, inputStr, 10);
    assert(flag == 0);
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(f_mpz, inputStr, 10);
    assert(flag == 0);

    f = mpz_get_ui(f_mpz);

    rsaSign(&p, &q, &N, &d, &m, &c);
    mpz_out_str(stdout, 10, c);
    printf("\n%d\n", copro_clock);

    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(N);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(m);
    mpz_clear(f_mpz);

    return 0;
}