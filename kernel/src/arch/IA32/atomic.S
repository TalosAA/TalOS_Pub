

.section .text
.global AtomicTestSetAndWait
.type AtomicTestSetAndWait, @function
AtomicTestSetAndWait:
        pushl     %ebp
        mov       %esp, %ebp
        pushl     %ebx
        movl      8(%ebp), %ebx
__acquire:
        lock btsl $0,(%ebx)
        jc        __spin_pause
        popl      %ebx
        leave
        ret
__spin_pause:
        pause
        testl     $1,(%ebx)
        jnz       __spin_pause
        jmp       __acquire


