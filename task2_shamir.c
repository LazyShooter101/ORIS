#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <x86intrin.h>

#define l 1024
#define GENERATE_KEY 1 // 0 or 1
#define RSA_SIGN_METHOD 3
#define SKIP_RSA_SIGN 0

#if GENERATE_KEY
void rsaKeygen(mpz_t p, mpz_t q, mpz_t N, mpz_t e, mpz_t d) {
    gmp_randstate_t rng;
    unsigned long seed = rand();
    gmp_randinit_default(rng);
    gmp_randseed_ui(rng, seed);

    // generate p, q primes by randomly generating numbers
    // until they are prime
    int isPrime;
    while (1) {
        mpz_urandomb(p, rng, l/2);
        isPrime = mpz_probab_prime_p(p, 50);
        if (isPrime >= 1) {
            break;
        }
    }
    while (1) {
        mpz_urandomb(q, rng, l/2);
        isPrime = mpz_probab_prime_p(q, 50);
        if (isPrime >= 1) {
            break;
        }
    }
    // N = pq
    mpz_mul(N, p, q);

    // find e by checking that its gcd with phi(N) == 1
    // note that phi(N) = (p-1)(q-1)
    mpz_t p_minus_one, q_minus_one, phi_N, gcd;
    mpz_init(p_minus_one);
    mpz_init(q_minus_one);
    mpz_init(phi_N);
    mpz_init(gcd);

    mpz_sub_ui(p_minus_one, p, 1);
    mpz_sub_ui(q_minus_one, q, 1);
    mpz_mul(phi_N, p_minus_one, q_minus_one);
    do {
        mpz_urandomm(e, rng, phi_N);
        mpz_gcd(gcd, e, phi_N);
    } while (mpz_cmp_ui(gcd, 1) != 0); // its 0 when they're the same

    // calc d by doing extended gcd on e, phi_N:
    mpz_t gcd_result; // dummy
    mpz_init(gcd_result);
    mpz_gcdext(gcd, d, gcd_result, e, phi_N);
    assert(mpz_cmp_ui(gcd, 1) == 0); // check that the gcd is 1
    mpz_mod(d, d, phi_N); // d %= phi(N)

    // check that our numbers work by checking that for arbitrary m,
    // (m^e)^d == m (mod N)
    mpz_t m, exp_result;
    mpz_init(m);
    mpz_init(exp_result);
    for (int i=0; i<100; ++i) {
        mpz_urandomb(m, rng, l);
        mpz_mod(m, m, N);
        mpz_powm(exp_result, m, e, N);
        mpz_powm(exp_result, exp_result, d, N);
        assert(mpz_cmp(m, exp_result) == 0); // check that they are equal
    }

    // clear temporary nums
    mpz_clears(m, exp_result, phi_N, gcd, p_minus_one, q_minus_one, gcd_result, NULL);
}
#endif

void rsaSign(mpz_t p, mpz_t q, mpz_t N, mpz_t d, mpz_t m, mpz_t q_inverse, mpz_t c, mpz_t t) {
#if SKIP_RSA_SIGN
    return;
#endif
#if RSA_SIGN_METHOD == 0
    // basic exponentiation
    mpz_powm(c, m, d, N);
#elif RSA_SIGN_METHOD == 1
    // CRT
    mpz_t prime_minus_one, d_mod, S_p, S_q;
    mpz_inits(prime_minus_one, d_mod, S_p, S_q, NULL);

    mpz_sub_ui(prime_minus_one, p, 1);
    mpz_mod(d_mod, d, prime_minus_one);
    mpz_powm(S_p, m, d_mod, p);

    mpz_sub_ui(prime_minus_one, q, 1);
    mpz_mod(d_mod, d, prime_minus_one);
    mpz_powm(S_q, m, d_mod, q);

    mpz_sub(prime_minus_one, S_p, S_q);
    mpz_mul(c, prime_minus_one, q_inverse);
    mpz_mod(c, c, p);
    mpz_mul(c, c, q);
    mpz_add(c, c, S_q);
    mpz_mod(c, c, N);

    mpz_clears(prime_minus_one, d_mod, S_p, S_q, NULL);

#elif RSA_SIGN_METHOD == 2

    mpz_t pt, qt, S_pt, S_qt, S_p, S_q;
    mpz_inits(pt, qt, S_pt, S_qt, S_p, S_q, NULL);
    mpz_mul(pt, p, t);
    mpz_mul(qt, q, t);

    mpz_powm(S_pt, m, d, pt);
    mpz_powm(S_qt, m, d, qt);
    // check that S_pt == S_qt (mod t)
    mpz_mod(S_p, S_pt, t);
    mpz_mod(S_q, S_qt, t);
    assert(mpz_cmp(S_p, S_q) == 0);

    mpz_mod(S_p, S_pt, p);
    mpz_mod(S_q, S_qt, q);

    mpz_sub(pt, S_p, S_q);
    mpz_mul(c, pt, q_inverse);
    mpz_mod(c, c, p);
    mpz_mul(c, c, q);
    mpz_add(c, c, S_q);
    mpz_mod(c, c, N);

    mpz_clears(pt, qt, S_pt, S_qt, S_p, S_q, NULL);
#elif RSA_SIGN_METHOD == 3
    mpz_t pt, qt, S_pt, S_qt, S_p, S_q, d_mod, phi, t_minus_one;
    mpz_inits(pt, qt, S_pt, S_qt, S_p, S_q, d_mod, phi, t_minus_one, NULL);

    mpz_sub_ui(t_minus_one, t, 1);
    mpz_mul(pt, p, t);
    mpz_mul(qt, q, t);

    mpz_sub_ui(phi, p, 1);
    mpz_mul(phi, phi, t_minus_one);
    mpz_mod(d_mod, d, phi);
    mpz_powm(S_pt, m, d_mod, pt);

    mpz_sub_ui(phi, q, 1);
    mpz_mul(phi, phi, t_minus_one);
    mpz_mod(d_mod, d, phi);
    mpz_powm(S_qt, m, d, qt);

    // check that S_pt == S_qt (mod t)
    mpz_mod(S_p, S_pt, t);
    mpz_mod(S_q, S_qt, t);
    assert(mpz_cmp(S_p, S_q) == 0);

    mpz_mod(S_p, S_pt, p);
    mpz_mod(S_q, S_qt, q);

    mpz_sub(pt, S_p, S_q);
    mpz_mul(c, pt, q_inverse);
    mpz_mod(c, c, p);
    mpz_mul(c, c, q);
    mpz_add(c, c, S_q);
    mpz_mod(c, c, N);

    mpz_clears(pt, qt, S_pt, S_qt, S_p, S_q, d_mod, phi, t_minus_one, NULL);
#endif
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

    mpz_t p, q, N, e, d, m, c;
    mpz_inits(p, q, N, e, d, m, c, NULL);


#if GENERATE_KEY
    printf("starting keygen\n");
    rsaKeygen(p, q, N, e, d);
    printf("finished keygen\n");
#else
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
#endif
    unsigned long long start_time, end_time, total_time;
    // init rng
    gmp_randstate_t rng;
    unsigned long seed = rand();
    gmp_randinit_default(rng);
    gmp_randseed_ui(rng, seed);
    // init check
    mpz_t check;
    mpz_init(check);
    // find q inverse (mod p)
    mpz_t q_inverse;
    mpz_init(q_inverse);
    mpz_gcdext(check, check, q_inverse, p, q); // doesn't matter what we set check to

    // gen t
    mpz_t t;
    mpz_init(t);
    int isPrime;
    size_t const n_bytes_of_T = 224;
    while (1) {
        mpz_urandomb(t, rng, n_bytes_of_T);
        isPrime = mpz_probab_prime_p(t, 50);
        if (isPrime >= 1) {
            break;
        }
    }
    printf("started timer\n");
    // start timer
    size_t n_cycles = 2000;
    start_time = __rdtsc();
    
    for (size_t i=0; i<n_cycles; ++i) {
        // randomise m
        mpz_urandomb(m, rng, l);
        mpz_mod(m, m, N);
        rsaSign(p, q, N, d, m, q_inverse, c, t);
#if SKIP_RSA_SIGN
        mpz_powm(check, m, e, N);
#else
        // mpz_powm(check, c, e, N);
        // assert(mpz_cmp(check, m) == 0);
#endif
    }

    // finish timer
    end_time = __rdtsc();
    total_time = (end_time - start_time);
    mpz_out_str(stdout, 10, c);
    printf("\nTotal time taken: %d clock cycles\n(%d per iteration)\n", total_time, total_time / n_cycles);

    mpz_clears(p, q, N, e, d, m, check, q_inverse, t, NULL);

    return 0;
}