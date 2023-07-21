import functools
from Crypto.Math import Primality
import random
import math

def gcd_extended(a, b):
    # https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm#Pseudocode
    old_r, r = a, b
    old_s, s = 1, 0
    old_t, t = 0, 1
    while r != 0:
        quotient = old_r // r
        old_r, r = r, old_r - quotient*r
        old_s, s = s, old_s - quotient*s 
        old_t, t = t, old_t - quotient*t 
    # old_s * a  +  old_t * b  ==  old_r
    #   x   * a  +    y   * b  ==  gcd(a,b)
    assert(old_s*a + old_t*b == old_r)
    return old_s, old_t, old_r


def rsa_keygen(n_bits, verbosity=1, n_checks=100):
    if verbosity >= 1: print("Starting rsa_keygen")
    # randomise p, q primes with n_bits/2 bits
    p = int(Primality.generate_probable_prime(exact_bits=n_bits//2))
    q = int(Primality.generate_probable_prime(exact_bits=n_bits//2))
    if p == q:
        return rsa_keygen(n_bits) # no opportunity for infinite loop due to ~0 chance
    if verbosity >= 2: print("\tFound primes p,q")

    # N is a large composite number with n_bits bits
    N = p*q
    phi_n = (p-1)*(q-1)
    e = random.randint(3, phi_n-1)
    while math.gcd(e, phi_n) != 1:
        e = random.randint(3, phi_n-1)
    if verbosity >= 2: print("\tFound e")
    
    # now find d such that d*e == 1 (mod phi_n)
    # this can be done by finding d, a such that
    # d*e + a*phi_n == 1
    # this is why we ensured gcd(e, phi_n) == 1
    # this can be done by the extended euclidean alg.
    d, _, gcd = gcd_extended(e, phi_n)
    assert(gcd == 1)
    if verbosity >= 2: print("\tFound d")
    d %= phi_n

    # now check that for some random message m,
    # we have that ((m^e)^d) == m (mod N)
    for _ in range(n_checks):
        m = random.randint(2, N-1)
        assert(pow(pow(m, e, N), d, N) == m)
    if verbosity >= 2: print("\td,e,N passed checks")
    if verbosity >= 1: print("Finished rsa_keygen")
    return p, q, N, e, d


def rsa_sign(p, q, N, d, n_bits, m, f=0):
    # find c == m^d (mod N)
    # using https://en.wikipedia.org/wiki/Chinese_remainder_theorem#Computation
    
    # first compute a == m^d%p-1 (mod p)
    a = pow(m, d%(p-1), p)
    if f == 1:
        # flip a random bit
        a ^= 1<<(random.randint(0, n_bits-1))

    # then compute b == m^d%q-1 (mod q)
    b = pow(m, d%(q-1), q)
    if f == 2:
        # flip a random bit
        b ^= 1<<(random.randint(0, n_bits-1))

    # now c == a (mod p), c == b (mod q) can be computed in O(log(N)^2) time with CRT
    x, y, gcd = gcd_extended(p, q)
    assert(gcd == 1)
    # => c == bxp + ayq (mod N) because xp + yq == 1 (mod N)
    c = ((b*x*p % N) + (a*y*q % N)) % N
    
    return c

def check_rsa_sign(sign_func, p, q, N, e, d, l, n_checks=1000):
    print("Checking rsa_sign")
    for _ in range(n_checks):
        m = random.randint(2, N-1)
        c = sign_func(p, q, N, d, l, m)
        assert(pow(c, e, N) == m)
    print("rsa_sign passed checks")


def attack(D, N, e):
    print("Beginning attack!")
    # can call D(m, f)
    # using https://link.springer.com/article/10.1007/s001450010016:
    m = random.randint(2, N)

    S_faulty = D(m, 1) # just one call to D!

    p = math.gcd(m - pow(S_faulty, e, N), N)
    q = N//p
    assert(p*q == N)
    print("\tFound p, q!")
    # now we have broken in and found p, q!

    # we can now find d to "prove" we have broken in
    phi_n = (p-1)*(q-1)
    d, _, gcd = gcd_extended(e, phi_n)
    assert(gcd == 1)
    d %= phi_n
    print("\tFound d!")
    print("Finished attack!")
    return d

# Main Code
if  __name__ == '__main__':
    l = 1024
    p, q, N, e, d = rsa_keygen(l, verbosity=2)
    check_rsa_sign(rsa_sign, p, q, N, e, d, l)
    # m = random.randint(2, N-1)
    # c = rsa_sign(p, q, N, d, l, m, 0)
    # print(f"p: {p}\nq: {q}\nN: {N}\nd: {d}\nm: {m}\nf: {0}\n\nc: {c}")
    D = functools.partial(rsa_sign, p, q, N, d, l)

    d2 = attack(D, N, e)
    assert(d == d2)