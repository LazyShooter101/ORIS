#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

#define GENERATE_KEY 0 // 0 or 1

unsigned int l = 1024;

#if GENERATE_KEY
void rsaKeygen(mpz_t *p, mpz_t *q, mpz_t *N, mpz_t *e, mpz_t *d) {
    gmp_randstate_t rng;
    unsigned long seed = rand();
    gmp_randinit_default(rng);
    gmp_randseed_ui(rng, seed);

    // generate p, q primes by randomly generating numbers
    // until they are prime
    int isPrime;
    while (1) {
        mpz_urandomb(*p, rng, l/2);
        isPrime = mpz_probab_prime_p(*p, 50);
        if (isPrime >= 1) {
            break;
        }
    }
    while (1) {
        mpz_urandomb(*q, rng, l/2);
        isPrime = mpz_probab_prime_p(*q, 50);
        if (isPrime >= 1) {
            break;
        }
    }
    // N = pq
    mpz_mul(*N, *p, *q);

    // find e by checking that its gcd with phi(N) == 1
    // note that phi(N) = (p-1)(q-1)
    mpz_t p_minus_one, q_minus_one, phi_N, gcd;
    mpz_init(p_minus_one);
    mpz_init(q_minus_one);
    mpz_init(phi_N);
    mpz_init(gcd);

    mpz_sub_ui(p_minus_one, *p, 1);
    mpz_sub_ui(q_minus_one, *q, 1);
    mpz_mul(phi_N, p_minus_one, q_minus_one);
    do {
        mpz_urandomm(*e, rng, phi_N);
        mpz_gcd(gcd, *e, phi_N);
    } while (mpz_cmp_ui(gcd, 1) != 0); // its 0 when they're the same

    // calc d by doing extended gcd on e, phi_N:
    mpz_t gcd_result; // dummy
    mpz_init(gcd_result);
    mpz_gcdext(gcd, *d, gcd_result, *e, phi_N);
    assert(mpz_cmp_ui(gcd, 1) == 0); // check that the gcd is 1
    mpz_mod(*d, *d, phi_N); // d %= phi(N)

    // check that our numbers work by checking that for arbitrary m,
    // (m^e)^d == m (mod N)
    mpz_t m, exp_result;
    mpz_init(m);
    mpz_init(exp_result);
    for (int i=0; i<100; ++i) {
        mpz_urandomb(m, rng, l);
        mpz_mod(m, m, *N);
        mpz_powm(exp_result, m, *e, *N);
        mpz_powm(exp_result, exp_result, *d, *N);
        assert(mpz_cmp(m, exp_result) == 0); // check that they are equal
    }

    // clear temporary nums
    mpz_clear(m);
    mpz_clear(exp_result);
    mpz_clear(phi_N);
    mpz_clear(gcd);
    mpz_clear(p_minus_one);
    mpz_clear(q_minus_one);
    mpz_clear(gcd_result);

}
#endif

void rsaSign(mpz_t *p, mpz_t *q, mpz_t *N, mpz_t *d, mpz_t *m, mpz_t *f) {
    // init nums used for computation
    mpz_t a, b, x, y, prime_minus_one, d_mod, gcd, c, flip_mask, flip_index;
    mpz_init(a);
    mpz_init(b);
    mpz_init(prime_minus_one);
    mpz_init(d_mod);
    mpz_init(x);
    mpz_init(y);
    mpz_init(gcd);
    mpz_init(c);
    mpz_init_set_ui(flip_mask, 0);

    // compute the flip_mask, for flipping one random bit of a or b
    mpz_setbit(flip_mask, rand()%l);

    // compute a = m^(d % p-1) (mod p)
    mpz_sub_ui(prime_minus_one, *p, 1);
    mpz_mod(d_mod, *d, prime_minus_one);
    mpz_powm(a, *m, d_mod, *p);
    if (mpz_cmp_ui(*f, 1) == 0) { // flip a bit of `a` if f=1
        mpz_xor(a, a, flip_mask);
    }

    // compute b = m^(d % q-1) (mod q)
    mpz_sub_ui(prime_minus_one, *q, 1);
    mpz_mod(d_mod, *d, prime_minus_one);
    mpz_powm(b, *m, d_mod, *q);
    if (mpz_cmp_ui(*f, 2) == 0) { // flip a bit of `b` if f=2
        mpz_xor(b, b, flip_mask);
    }

    // compute x, y such that xp + qy == 1
    mpz_gcdext(gcd, x, y, *p, *q);
    assert(mpz_cmp_ui(gcd, 1) == 0); // check that the gcd is 1

    // c == bxp + ayq (mod N)
    mpz_mul(c, b, x);
    mpz_mod(c, c, *N);
    mpz_mul(c, c, *p);
    mpz_mod(c, c, *N);
    mpz_mul(gcd, a, y); // using gcd as a dummy variable
    mpz_mod(gcd, gcd, *N);
    mpz_mul(gcd, gcd, *q);
    mpz_mod(gcd, gcd, *N);
    mpz_add(c, c, gcd);
    mpz_mod(c, c, *N);

    // finally output c
    mpz_out_str(stdout,10,c);
    printf("\n");

    // (clear temporary nums)
    mpz_clear(a);
    mpz_clear(b);
    mpz_clear(prime_minus_one);
    mpz_clear(d_mod);
    mpz_clear(x);
    mpz_clear(y);
    mpz_clear(gcd);
    mpz_clear(c);
    mpz_clear(flip_mask);
}

int main() {
    srand(time(NULL));
    for (int j=0; j<3; ++j) {
        int stop = 10+rand()%100;
        for (int i=10; i<stop; ++i) {
            rand();
        }
    }

    mpz_t p, q, N, e, d, m, f;
    mpz_init(p);
    mpz_init(q);
    mpz_init(N);
    mpz_init(e);
    mpz_init(d);
    mpz_init(m);
    mpz_init(f);

#if GENERATE_KEY
    rsaKeygen(&p, &q, &N, &e, &d);
    mpz_out_str(stdout,10,p);
    printf("\n");
    mpz_out_str(stdout,10,q);
    printf("\n");
    mpz_out_str(stdout,10,N);
    printf("\n");
    mpz_out_str(stdout,10,e);
    printf("\n");
    mpz_out_str(stdout,10,d);
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
    scanf("%1023s" , inputStr);
    flag = mpz_set_str(f, inputStr, 10);
    assert(flag == 0);

#endif

    rsaSign(&p, &q, &N, &d, &m, &f);

    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(N);
    mpz_clear(e);
    mpz_clear(d);
    mpz_clear(m);
    mpz_clear(f);
    return 0;
}