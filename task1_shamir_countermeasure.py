import functools
from Crypto.Math import Primality
import random
import math
import timeit
from task1 import rsa_keygen, gcd_extended

def phi(n):
    tot = 0
    for i in range(n):
        if math.gcd(i, n) == 1:
            tot += 1
    return tot


def rsa_sign(p, q, N, d, n_bits, m, f=0):
    # find c == m^d (mod N)
    # using https://en.wikipedia.org/wiki/Chinese_remainder_theorem#Computation
    
    t = random.randint(20, 200) 
    phi_t = phi(t)

    # first compute a == m^d%p-1 (mod p)
    S_pt = pow(m, d%(phi_t*(p-1)), p*t)
    if f == 1:
        # flip a random bit
        S_pt ^= 1<<(random.randint(0, n_bits-1))

    # then compute b == m^d%q-1 (mod q)
    S_qt = pow(m, d%(phi_t*(q-1)), q*t)
    if f == 2:
        # flip a random bit
        S_qt ^= 1<<(random.randint(0, n_bits-1))    

    assert(S_pt%t == S_qt%t)

    # now c == a (mod p), c == b (mod q) can be computed in O(log(N)^2) time with CRT
    x, y, gcd = gcd_extended(p, q)
    assert(gcd == 1)
    # => c == bxp + ayq (mod N) because xp + yq == 1 (mod N)
    c = (((S_qt%q)*x*p % N) + ((S_pt%p)*y*q % N)) % N
    
    return c

def check_rsa_sign(sign_func, p, q, N, e, d, l, n_checks=1000):
    print("Checking rsa_sign")
    for _ in range(n_checks):
        m = random.randint(2, N-1)
        c = sign_func(p, q, N, d, l, m)
        assert(pow(c, e, N) == m)
    print("rsa_sign passed checks")


def attack(D, N, e):
    # print("Beginning attack!")
    # can call D(m, f)
    # using https://link.springer.com/article/10.1007/s001450010016:
    m = random.randint(2, N)

    S_faulty = D(m, 1) # just one call to D!

    p = math.gcd(m - pow(S_faulty, e, N), N)
    q = N//p
    assert(p*q == N)
    # print("\tFound p, q!")
    # now we have broken in and found p, q!

    # we can now find d to "prove" we have broken in
    phi_n = (p-1)*(q-1)
    d, _, gcd = gcd_extended(e, phi_n)
    assert(gcd == 1)
    d %= phi_n
    # print("\tFound d!")
    # print("Finished attack!")
    return d


# Main Code
if  __name__ == '__main__':
    l = 1024
    p, q, N, e, d = rsa_keygen(l, verbosity=2)
    # check_rsa_sign(rsa_sign, p, q, N, e, d, l)
    # def rsa_sign_timer_func():
        # _ = rsa_sign(p, q, N, d, l, random.randint(2, N), 0)
    # print(timeit.timeit(stmt=lambda: rsa_sign_timer_func(), number=10000))
    # m = random.randint(2, N-1)
    # c = rsa_sign(p, q, N, d, l, m, 0)
    # print(f"p: {p}\nq: {q}\nN: {N}\nd: {d}\nm: {m}\nf: {0}\n\nc: {c}")
    D = functools.partial(rsa_sign, p, q, N, d, l)

    n_passed = 0
    n_att = 100000
    for i in range(n_att):
        if (i % 1000 == 0):
            print(f"attacks {100*i/n_att :.3f}% done")
        try:
            d2 = attack(D, N, e)
            assert(d == d2)
            n_passed += 1
        except AssertionError:
            continue
    print(f"{n_passed}/{n_att} attacks successful ({100*n_passed / n_att :.4f}%)")