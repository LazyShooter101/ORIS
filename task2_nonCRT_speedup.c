#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#define n 512

void doBCalc(mpz_t m, mpz_t d_known, mpz_t N, mpz_t S_hat, mpz_t e) {
    mpz_t a, q, b, exp;
    mpz_inits(a, q, b, exp, NULL);
    mpz_powm(exp, m, d_known, N);
    mpz_set_ui(b, 1);
    size_t pass = 0;
    for (int i=0; i<n; ++i) {
        mpz_mul(q, b, exp);
        mpz_mod(q, q, N);
        mpz_add(a, S_hat, q);
        mpz_powm(a, a, e, N);
        if (mpz_cmp(m, a) == 0) {
            pass = 1;
            break;
        }
        mpz_sub(a, S_hat, q);
        mpz_powm(a, a, e, N);
        if (mpz_cmp(m, a) == 0) {
            pass = 1;
            break;
        }
        mpz_mul_ui(b, b, 2);
    }
    mpz_clears(a, q, b, exp, NULL);
    printf("%d", pass);
}

int main() {
    mpz_t m, d_known, N, S_hat, e;
    mpz_inits(m, d_known, N, S_hat, e, NULL);

    char inputStr[1024];
    int flag; // for parsing str to mpz_t
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(m, inputStr, 10);
    assert(flag == 0);
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(d_known, inputStr, 10);
    assert(flag == 0);
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(N, inputStr, 10);
    assert(flag == 0);
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(S_hat, inputStr, 10);
    assert(flag == 0);
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(e, inputStr, 10);
    assert(flag == 0);

    doBCalc(m, d_known, N, S_hat, e);

    mpz_clears(m, d_known, N, S_hat, e, NULL);
    return 0;
}