# Problem 1 – Trace of getpid() System Call

## 1. User-space stub
**File:** `user/usys.S` (generated from `user/usys.pl`)

The user program calls `getpid()` like a normal C function.

The generated assembly stub looks like:

```asm
.global getpid
getpid:
    li a7, SYS_getpid
    ecall
    ret
```

The stub performs two tasks:

1. Loads the syscall number (`SYS_getpid`) into register `a7`.
2. Executes the `ecall` instruction to enter the kernel.

---

## 2. The ecall instruction

`ecall` is a RISC-V instruction that causes an environment call exception.

When it executes:

- CPU changes from User mode to Supervisor mode.
- The current PC is saved into `sepc`.
- `scause` is set to indicate a User Environment Call.
- The CPU jumps to the address stored in `stvec`.

No xv6 C code executes yet. This behavior is handled by the RISC-V hardware.

---

## 3. First xv6 code that runs

**File:** `kernel/trampoline.S`

Execution begins at the label:

```asm
uservec:
```

This assembly routine:

- Saves all user registers into the process trapframe.
- Saves registers like `ra`, `sp`, `gp`, `tp`, `a0-a7`, etc.
- Switches from the user page table to the kernel page table.
- Calls `usertrap()`.

---

## 4. usertrap()

**File:** `kernel/trap.c`

The kernel enters:

```c
void
usertrap(void)
```

The important condition is:

```c
if(r_scause() == 8)
```

`8` means a system call from user mode.

When this condition is true:

- The kernel advances `sepc` by 4 bytes.
- Interrupts are enabled.
- `syscall()` is called.

---

## 5. syscall()

**File:** `kernel/syscall.c`

The syscall number is read from:

```c
num = p->trapframe->a7;
```

The dispatch table is:

```c
static uint64 (*syscalls[])(void)
```

The kernel executes:

```c
p->trapframe->a0 = syscalls[num]();
```

This calls the correct syscall implementation based on the syscall number.

---

## 6. sys_getpid()

**File:** `kernel/sysproc.c`

Implementation:

```c
uint64
sys_getpid(void)
{
    return myproc()->pid;
}
```

The function simply returns the current process ID.

The returned value is stored into:

```c
p->trapframe->a0
```

Later, this becomes register `a0` when execution returns to user space.

---

## 7. Returning to user mode

Control returns through two functions.

### usertrapret()

**File:** `kernel/trap.c`

This function:

- Prepares the trapframe.
- Restores user register values.
- Switches back to the user page table.
- Jumps into the trampoline.

### userret()

**File:** `kernel/trampoline.S`

This routine:

- Restores all user registers from the trapframe.
- Executes

```asm
sret
```

`sret` switches the CPU back to User mode.

Execution resumes immediately after the original `ecall`, where the assembly stub executes:

```asm
ret
```

The return value is already present in register `a0`, so the user program receives the PID exactly like a normal C function return value.