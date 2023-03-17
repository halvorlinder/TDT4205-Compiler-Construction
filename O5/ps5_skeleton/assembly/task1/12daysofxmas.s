.globl main

.section .rodata
intro: .string "On day %ld of Christmas my true love sent to me\n"
and_s: .string "and "

str01: .string "%ld partridge in a pear tree\n"
str02: .string "%ld turtle doves\n"
str03: .string "%ld french hens\n"
str04: .string "%ld calling birds\n"
str05: .string "%ld gold rings\n"
str06: .string "%ld geese a-laying\n"
str07: .string "%ld swans a-swimming\n"
str08: .string "%ld maids a-milking\n"
str09: .string "%ld ladies dancing\n"
str10: .string "%ld lords a-leaping\n"
str11: .string "%ld pipers piping\n"
str12: .string "%ld drummers drumming\n"

lines:
    .quad str01, str02, str03, str04, str05, str06
    .quad str07, str08, str09, str10, str11, str12

.section .text
main:
    pushq %rbp
    movq %rsp, %rbp

    movq $1, %r12          # Outer loop counter 
outer_loop:
    cmpq $13, %r12                      # 13th iteration? 
    jge outer_loop_end              

    movq %r12, %rsi         # Load count into arg2
    movq $intro, %rdi       # Load intro string into arg 1
    call printf             # Print

    movq $1, %r13           # Inner loop counter
inner_loop:
    cmpq %r12, %r13         # Looped over all days?
    jge inner_loop_end              
    movq %r13, %rsi         # Load inner count into arg2
    movq $lines, %rdi               # Reading address of lines pointer
    dec %r13                        # Offset day number by one
    movq (%rdi, %r13, 8), %rdi      # Read nth string
    inc %r13                        # Reset inner counter
    call printf             # Print 
    inc %r13                # Increment inner count
    jmp inner_loop          
inner_loop_end:
    cmpq $1, %r12           # Check for first day, and skip "and" part
    je final_print
    movq $and_s, %rdi       # Load and into arg 1
    call printf             # Print
final_print:
    movq %r13, %rsi         # Load arg 2
    movq $lines, %rdi               # Reading address of lines pointer
    dec %r13                        # Offset day number by one
    movq (%rdi, %r13, 8), %rdi      # Read nth string
    call printf                     # Print 


    inc %r12                # Increment outer counter
    movq $10, %rdi          # Load \n into arg 1
    call putchar            # Print newline
    jmp outer_loop
outer_loop_end:    

    leave                   # Done with main: reverting pointers and pop from stack 
    ret                     # Return from main
