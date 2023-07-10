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

def rsa_keygen(n_bits, verbosity=1):
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
    d, a, gcd = gcd_extended(e, phi_n)
    assert(gcd == 1)
    if verbosity >= 2: print("\tFound d")

    # now check that for some random message m,
    # we have that ((m^e)^d) == 1 (mod N)
    for _ in range(100):
        m = random.randint(2, N-1)
        assert(pow(pow(m, e, N), d, N) == m)
    if verbosity >= 2: print("\td,e,N passed checks")
    return p, q, N, e, d

def rsa_sign(p, q, N, d, m, f=0):
    raise NotImplementedError()

def attack(D, N, e):
    raise NotImplementedError()

# Main Code
if  __name__ == '__main__':
    p, q, N, e, d = rsa_keygen(1024, verbosity=2)

    D = functools.partial(rsa_sign, p, q, N, d)

    attack(D, N, e)