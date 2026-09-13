mov8 r3 1
mov8 r4 11
mov8 r5 1
loop:
trap
add r3 r5
brlt r3 r4 loop
halt
