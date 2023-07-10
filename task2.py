import functools
import random
import math
from task1 import gcd_extended, rsa_keygen, check_rsa_sign, attack
from task2_coprocessor import Coprocessor

P_ADR = 0
Q_ADR = 1
N_ADR = 2
D_ADR = 3
M_ADR = 4
COMP_ADR_1 = 5
COMP_ADR_2 = 6




# def power(x, y, z, N):
#     """
#     Puts `y`^`z` into `x` (mod N)
#     Uses left to right binary exp.
#     """
#     # https://cacr.uwaterloo.ca/hac/about/chap14.pdf, 14.79
#     coprocessor_load_immediate(x, 1)
#     for bit in bin(R[z])[2:]: # assuming that taking bits costs 0 clock cycles since stored in binary anyway
#         coprocessor_mul(x, x, x, N)
#         if bit == "1":
#             coprocessor_mul(x, x, y)
    

def rsa_sign(p, q, N, d, n_bits, m, f=0):
    # find c == m^d (mod N)
    # using https://en.wikipedia.org/wiki/Chinese_remainder_theorem#Computation
    # realistic version using registers etc

    pass