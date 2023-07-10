from task1 import gcd_extended

class Coprocessor:
    N_REGISTERS: int
    R: list[int]
    clock: int

    def __init__(self, n_registers: int) -> None:
        self.N_REGISTERS = n_registers
        self.R = [0 for _ in range(self.N_REGISTERS)]
        self.clock = 0

    def add(self, x, y, z, N):
        """
        sums registers `y`+`z` into `x` (mod `N`)
        """
        self.clock += 1
        self.R[x] = (self.R[y] + self.R[z]) % self.R[N]

    def sub(self, x, y, z, N):
        """
        subs registers `y`-`z` into `x` (mod `N`)
        """
        self.clock += 1
        self.R[x] = (self.R[y] - self.R[z]) % self.R[N]

    def mul(self, x, y, z, N):
        """
        muls registers `y`*`z` into `x` (mod `N`)
        """
        self.clock += 1
        self.R[x] = (self.R[y] * self.R[z]) % self.R[N]

    def mul_inverse(self, x, y, N):
        """
        puts 1/`y` into `x` (mod `N`) if it exists
        otherwise puts 0 into `x`
        """
        self.clock += 1
        # https://en.wikipedia.org/wiki/Modular_multiplicative_inverse#Extended_Euclidean_algorithm
        # find R[y]*a + N*_ == 1 (mod N)
        a, _, gcd = gcd_extended(self.R[y], self.R[N])
        # therefore a = 1/R[y] (mod N)
        if gcd == 1:
            self.R[x] = a % self.R[N]
            return
        self.R[x] = 0
        
    def add_inverse(self, x, y, N):
        """
        puts -`y` into `x` (mod `N`)    
        """
        self.clock += 1    
        self.R[x] = (- self.R[y]) % self.R[N]

    def copy_mod(self, x, y, N):
        """
        puts `y` into `x` (mod `N`)
        """
        self.clock += 1
        self.R[x] = self.R[y] % self.R[N]

    def empty_regs(self):
        """
        sets all coprocessor registers to 0
        (doesnt use a clock cycle)
        """
        for i in range(self.N_REGISTERS):
            self.R[i] = 0

    def load_immediate(self, x, val):
        """
        puts `val` directly into location `x`
        """
        self.clock += 1
        self.R[x] = val

    def reset_clock(self):
        self.clock = 0

    def print_regs(self):
        for i, r in enumerate(self.R):
            print(f"{str(i).zfill(2)}: {hex(r)[2:].zfill(32)} | {r}")