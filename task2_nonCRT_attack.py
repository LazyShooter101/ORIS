from subprocess import Popen, PIPE
from task1 import rsa_keygen, check_rsa_sign
import random
import time
import math as maths
import itertools
import numpy as np

n = 512

def moduloMultiplication(a, b, mod):
    res = 0; # Initialize result
    a = a % mod;
    while (b):
        if (b & 1):
            res = (res + a) % mod
        a = (a << 1) % mod
        b >>= 1
    return res

def attack2(D, N, e):
    d_known = ""
    _, t0 = D(random.randint(2, N), 0)
    print("finding d...")
    for i in range(n):
        broke = False
        digits = ["0", "1"]
        random.shuffle(digits)
        for extra in digits:
            d_known_pow = int((d_known+extra).ljust(n, "0"), 2)
            # print(f"d_known_pow = {bin(d_known_pow)[2:].zfill(n)}")
            m = random.randint(2, N)
            S_hat, _ = D(m, t0-len(d_known)-1)
            # instead of doing this:
            # passed = False
            # temp = pow(m, d_known_pow, N)
            # for b in range(n):
            #     q = moduloMultiplication(1<<b, temp, N)
            #     X = S_hat + q
            #     Y = S_hat - q
            #     if pow(X, e, N) == m or pow(Y, e, N) == m:
            #         passed = True
            #         break
            
            # , use the c file which does it MUCH faster
            with Popen('task2_nonCRT_speedup.exe', stdin=PIPE, stdout=PIPE, universal_newlines=True) as prog:
                print(m,           file=prog.stdin, flush=True)
                print(d_known_pow, file=prog.stdin, flush=True)
                print(N,           file=prog.stdin, flush=True)
                print(S_hat,       file=prog.stdin, flush=True)
                print(e,           file=prog.stdin, flush=True)
                passed = bool(int((prog.stdout.readline())))
            if passed: 
                d_known = d_known + extra
                print(f"{d_known}...({n-i})")
                broke = True
                break
        if broke: continue
        print("couldn't find d-digit")
    return int(d_known, 2)


def attack(D, N, e):
    # return attack2(D, N, e)
    m = 8 # choose 1 <= m <= n
    l = maths.ceil((n/m) * maths.log2(2*n))
    # get the length of computation
    _, t = D(random.randint(2, N), 0)
    # let M = [random messages]
    M = [random.randint(2, N) for _ in range(l)]
    print("computing S_hat...")
    S_hat = []
    fs = []
    for i in range(l):
        f =  random.randint(0, t-1)
        fs.append((f, i))
        S_hat.append(D(M[i], f)[0])
        if (i % 128 == 0): print(f"{100 * i/l : .1f}% done")
    fs.sort(key = lambda q: -q[0])
    print(fs)
    print("done")
    k = [0 for _ in range(n)]
    d = ["_" for _ in range(n)]
    i = n+1
    for _ in range(n):
        for r in range(1, m+1):
            a = k[i-1] if i <= n else n
            c = a-r
            print(f"trying r = {r}, a={a}, c={c}")
            print(f"a-r = {a-r}")
            u_passed = False
            for u in range(2**r -1, -1, -1):
                u <<= a-r
                # print(f"\ttrying u = {bin(u)}")
                bin_u = bin(u)[2:].zfill(n)[::-1]
                w = 0
                for j in range(a, n):
                    w += d[j]*(2**j)
                for j in range(a-r, a):
                    w += int(bin_u[j])*(1 << j)
                print(f"\tTrying w = {bin(w)[2:].zfill(n)}")
                any_messages_pass = False
                for j in range(l):
                    Sj = S_hat[j]
                    Mj = M[j]
                    
                    if j%64 == 0: print(f"\t\tTrying j={j}/{l}")
                    # for b in range(n+1):
                    #     q = ((2**b)*pow(Mj, w, N))%N
                    #     X = Sj + q
                    #     Y = Sj - q
                    #     if (pow(X, e, N) == Mj):
                    #         any_messages_pass = True
                    #         break
                    #     if (pow(Y, e, N) == Mj):
                    #         any_messages_pass = True
                    #         break
                    # if any_messages_pass: break

                    with Popen('task2_nonCRT_speedup.exe', stdin=PIPE, stdout=PIPE, universal_newlines=True) as prog:
                        print(Mj,    file=prog.stdin, flush=True)
                        print(w,     file=prog.stdin, flush=True)
                        print(N,     file=prog.stdin, flush=True)
                        print(Sj, file=prog.stdin, flush=True)
                        print(e,     file=prog.stdin, flush=True)
                        passed = bool(int((prog.stdout.readline())))
                    if passed: 
                        any_messages_pass = True
                        break
                
                if any_messages_pass:
                    print("this u works!!!!!")
                    print(f"u = {u}")
                    k[i-2] = a - r
                    for o in range(a-r, a):
                        d[o] = int(bin_u[o])

                    print(f"new d = {''.join([str(x) for x in d[::-1]])}")
                    i -= 1
                    u_passed = True
                    break
            if u_passed: break
                
                

    print(t)
    print(l)
    d = 1
    return d

def check_rsa_sign(sign_func, n_checks=1000):
    print("Checking rsa_sign")
    for _ in range(n_checks):
        m = random.randint(2, N-1)
        c = sign_func(m, 0)
        assert(pow(c, e, N) == m)
    print("rsa_sign passed checks")

if __name__ == "__main__":
    p, q, N, e, d = rsa_keygen(n)

    def D(m, f):
        with Popen('task2_nonCRT.exe', stdin=PIPE, stdout=PIPE, universal_newlines=True) as prog:
            print(p, file=prog.stdin, flush=True)
            print(q, file=prog.stdin, flush=True)
            print(N, file=prog.stdin, flush=True)
            print(d, file=prog.stdin, flush=True)
            print(m, file=prog.stdin, flush=True)
            print(f, file=prog.stdin, flush=True)
            c = int(prog.stdout.readline())
            time = int(prog.stdout.readline())
        return c, time

    # check_rsa_sign(lambda *args, **kwargs: D(*args, **kwargs)[0])

    print(f"true d: {bin(d)[2:].zfill(n)}")
    d2 = attack(D, N, e)
    if d == d2:
        print("Attack successful")
    else:
        print("Attack failed")