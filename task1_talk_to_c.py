from subprocess import Popen, PIPE
from task1 import rsa_keygen, attack
import random
import time

if __name__ == "__main__":
    l = 1024
    p, q, N, e, d = rsa_keygen(l)

    def D(m, f):
        with Popen('task1_in_c.exe', stdin=PIPE, stdout=PIPE, universal_newlines=True) as prog:
            print(p, file=prog.stdin, flush=True)
            print(q, file=prog.stdin, flush=True)
            print(N, file=prog.stdin, flush=True)
            print(d, file=prog.stdin, flush=True)
            print(m, file=prog.stdin, flush=True)
            print(f, file=prog.stdin, flush=True)
            c = int(prog.stdout.readline())
        return c

    attack(D, N, e)