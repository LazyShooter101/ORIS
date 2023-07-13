from task1 import gcd_extended, rsa_keygen, check_rsa_sign, attack
from task2_coprocessor import Coprocessor
import functools
import random
import math

P_ADR = 0
Q_ADR = 1
N_ADR = 2
D_ADR = 3
M_ADR = 4
ONE_ADR = 5
COMP_ADR_1 = 6
COMP_ADR_2 = 7
A_ADR = 7
B_ADR = 8
X_ADR = 9
Y_ADR = 10
C_ADR = 11

def power(coprocessor: Coprocessor, x, y, z, N):
    """
    Puts `y`^`z` into `x` (mod N)
    Uses left to right binary exp.
    """
    # https://cacr.uwaterloo.ca/hac/about/chap14.pdf, 14.79
    coprocessor.load_immediate(x, 1)
    for bit in bin(coprocessor.R[z])[2:]: # assuming that taking bits costs 0 clock cycles since stored in binary anyway
        coprocessor.mul(x, x, x, N)
        if bit == "1":
            coprocessor.mul(x, x, y, N)


def rsa_sign(p, q, N, d, n_bits, m, f=0):
    # find c == m^d (mod N)
    # using https://en.wikipedia.org/wiki/Chinese_remainder_theorem#Computation
    # realistic version using registers etc
    
    # init coprocessor
    c = Coprocessor(16, n_bits, f)
    c.empty_regs()
    c.reset_clock()
    
    # load args into coprocessor
    c.load_immediate(P_ADR, p)
    c.load_immediate(Q_ADR, q)
    c.load_immediate(N_ADR, N)
    c.load_immediate(D_ADR, d)
    c.load_immediate(M_ADR, m)

    # compute a == m^(d mod p-1) (mod p)
    # first compute p-1
    c.load_immediate(ONE_ADR, 1)
    c.sub(COMP_ADR_1, P_ADR, ONE_ADR, P_ADR)
    # then d mod p-1
    c.copy_mod(COMP_ADR_1, D_ADR, COMP_ADR_1)
    # now a = m^(d mod p-1) (mod p)
    power(c, A_ADR, M_ADR, COMP_ADR_1, P_ADR)

    # now we do similar for b == m^(d mod q-1) (mod q)
    c.sub(COMP_ADR_1, Q_ADR, ONE_ADR, Q_ADR)
    c.copy_mod(COMP_ADR_1, D_ADR, COMP_ADR_1)
    power(c, B_ADR, M_ADR, COMP_ADR_1, Q_ADR)

    # now we compute x = 1/p (mod q)
    # and y = 1/q (mod p)
    c.mul_inverse(X_ADR, P_ADR, Q_ADR)
    c.mul_inverse(Y_ADR, Q_ADR, P_ADR)
    # so C = b*x*p + a*y*q (mod N)
    c.mul(C_ADR, B_ADR, X_ADR, N_ADR)
    c.mul(C_ADR, C_ADR, P_ADR, N_ADR)
    c.mul(COMP_ADR_1, A_ADR, Y_ADR, N_ADR)
    c.mul(COMP_ADR_1, COMP_ADR_1, Q_ADR, N_ADR)
    c.add(C_ADR, C_ADR, COMP_ADR_1, N_ADR)

    return c.R[C_ADR], c.clock

def attack(D, N, e):
    print("Beginning attack!")
    m = random.randint(2, N-1)
    m_signed, clock_cycles = D(m, 0)

    p, q = -1, -1 # temporary
    for i in range(clock_cycles//2, clock_cycles):
        m_signed_faulty, _ = D(m, i)
        # now compute 'p'
        p = math.gcd(m_signed - m_signed_faulty, N)
        if p == 1: continue
        if p == N: continue
        if N % p != 0: continue
        q = N // p
        break
    assert(p*q == N)
    print("\tFound p,q!")
    
    # Find d to 'prove' we have broken in
    phi_n = (p-1)*(q-1)
    d, _, gcd = gcd_extended(e, phi_n)
    assert(gcd == 1)
    d %= phi_n
    print("\tFound d!")
    print("Finished attack!")
    return d

# # Main Code
if __name__ == '__main__':
    l = 1024
    p, q, N, e, d = rsa_keygen(l, verbosity=2)
    check_rsa_sign(lambda *args, **kwargs: rsa_sign(*args, **kwargs)[0], p, q, N, e, d, l, n_checks=100)

    D = functools.partial(rsa_sign, p, q, N, d, l)

    d2 = attack(D, N, e)
    assert(d == d2)