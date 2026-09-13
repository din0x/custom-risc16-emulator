; ------------------------------------------------------------
; Demo program
; Prints the numbers 1 through 10 via syscall 0
; ------------------------------------------------------------

; -- immediate loads + add --
mov8 r3, 5
mov16 r4, 1000
add r3, r4

; -- store/load round trip through memory --
mov8 r6, 100
st r6, r3
ld r5, r6

; -- conditional branch --
mov8 r7, 2
mov8 r8, 9
br< r7, r8, after_branch

after_branch:

; -- call/ret --
calli routine

; ------------------------------------------------------------
; Print numbers 1 through 10
; ------------------------------------------------------------

mov8 r3, 1       ; r3 = current number
mov8 r4, 11      ; r4 = stopping value (11)
mov8 r5, 1       ; r5 = increment

print_loop:
trap             ; syscall 0: print r3
add r3, r5       ; r3 = r3 + 1
br< r3, r4, print_loop

halt

; ------------------------------------------------------------
; Subroutine
; ------------------------------------------------------------

routine:
mov8 r9, 42
ret
