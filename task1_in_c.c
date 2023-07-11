#include <stdio.h>
#include <gmp.h>
#include <assert.h>
#include <time.h>

unsigned int l = 1024;

void rsaKeygen(mpz_t *p, mpz_t *q, mpz_t *N, mpz_t *e, mpz_t *d) {
    gmp_randstate_t rng;
    unsigned long seed = time(NULL);
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
    } while (!mpz_cmp_ui(gcd, 1));

    // calc d by doing extended gcd on e, phi_N:
    mpz_t gcd_result;
    mpz_init(gcd_result);
    mpz_gcdext(gcd, *d, gcd_result, *e, phi_N);

    mpz_clear(phi_N);
    mpz_clear(gcd);
    mpz_clear(p_minus_one);
    mpz_clear(q_minus_one);

}

int main() {
    mpz_t p, q, N, e, d;
    mpz_init(p);
    mpz_init(q);
    mpz_init(N);
    mpz_init(e);
    mpz_init(d);

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


    mpz_clear(p);
    mpz_clear(q);
    mpz_clear(N);
    mpz_clear(e);
    mpz_clear(d);
    return 0;
}