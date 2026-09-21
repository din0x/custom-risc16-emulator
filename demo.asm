    mov8    r3 5
    mov16   r4 1000
    add     r3 r4

    mov8    r6 100
    st      r6 r3
    ld      r5 r6

    mov8    r7 2
    mov8    r8 9
    brlt    r7 r8 after_branch

after_branch:
    calli   routine

    mov8    r3 1
    mov8    r4 11
    mov8    r5 1

print_loop:
    trap
    add     r3 r5
    brlt    r3 r4 print_loop
    halt

routine:
    mov8    r9 42
    ret
