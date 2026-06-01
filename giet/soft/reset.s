#################################################################################
#    File : reset.s
#    Author : Alain Greiner
#################################################################################
#       This is an improved boot code for a bi-processor architecture.
#    Depending on the proc_id, each processor
#       - initializes the interrupt vector.
#       - initializes the ICU MASK registers.
#       - initializes the TIMER .
#       - initializes the Status Register.
#       - initializes the stack pointer.
#       - initializes the EPC register, and jumps to the user code.
#################################################################################
        
    .section .reset,"ax",@progbits

    .extern seg_stack_base
    .extern seg_data_base

    .extern seg_timer_base
    .extern seg_tty_base
    .extern seg_icu_base

    .extern _interrupt_vector
    .extern _isr_timer
    .extern _isr_tty_get

    .func   reset
    .type   reset, %function

reset:
    .set noreorder

    # get the processor id
    mfc0  $27,    $15,    1      # get the proc_id
    andi  $27,    $27,    0x1
    bne   $27,    $0,     proc1

proc0:
    # initialises interrupt vector entries for PROC[0]
    la    $8,      _interrupt_vector
    la    $9,      _isr_timer
    sw    $9,      8($8)
    la    $10,     _isr_tty_get
    sw    $10,     12($8)         # _interrupt_vector[3] <=> TTY[0] assigné à _isr_tty_get

    # initializes the ICU[0] MASK register
    la    $8,     seg_icu_base
#    ori   $9,     $0,     0x00000004
    ori   $9,     $0,     0x00000008
    sw    $9,     8($8)

    # initializes TIMER[0] PERIOD and RUNNING registers
    la    $8,    seg_timer_base
    li    $9,    50000           # 50k cycles
    sw    $9,    8($8)           # period set 
    ori   $10,   $10,     0b11
    sw    $10,   4($8)

    # initializes stack pointer for PROC[0]
    la    $29,    seg_stack_base
    li    $27,    0x10000        # stack size = 64K
    addu  $29,    $29,    $27    # $29 <= seg_stack_base + 64K

    # initializes SR register for PROC[0]
    li    $26,    0x0000FF13    
    mtc0  $26,    $12            # SR <= 0x0000FF13

    # jump to main in user mode: main[0]
    la    $26,    seg_data_base
    lw    $26,    0($26)         # $26 <= main[0]
    mtc0  $26,    $14            # write it in EPC register
    eret

proc1:
    # initialises interrupt vector entries for PROC[1]
    la    $8,     _interrupt_vector
    la    $9,     _isr_timer
    sw    $9,     16($8)
    la    $10,     _isr_tty_get
    sw    $10,     20($8)         # _interrupt_vector[5] <=> TTY[1] assigné à _isr_tty_get

    # initializes the ICU[1] MASK register
    la    $8,     seg_icu_base
    addi  $8,     $8,     32 # Registres ICU du proc 1
#    ori   $9,     $0,     0x00000010
    ori   $9,     $0,     0x00000020
    sw    $9,     8($8)

    # initializes TIMER[1] PERIOD and RUNNING registers
    la    $8,    seg_timer_base
    addiu $8,    $8,     16
    li    $9,    100000           # 50k cycles
    sw    $9,    8($8)           # period set 
    ori   $10,   $10,     0b11
    sw    $10,   4($8)

    # initializes stack pointer for PROC[1]
    la    $29,    seg_stack_base
    li    $27,    0x20000        # stack size = 64K
    addu  $29,    $29,    $27    # $29 <= seg_stack_base + 2*64K

    # initializes SR register for PROC[1]
    li    $26,    0x0000FF13    
    mtc0  $26,    $12            # SR <= 0x0000FF13

    # jump to main in user mode: main[1]
    la    $26,    seg_data_base
    lw    $26,    4($26)         # $26 <= main[1]
    mtc0  $26,    $14            # write it in EPC register
    eret

#proc2:
    # initializes stack pointer for PROC[2]
#    la    $29,    seg_stack_base
#    li    $27,    0x30000        # stack size = 64K
#    addu  $29,    $29,    $27    # $29 <= seg_stack_base + 3*64K

    # initializes SR register for PROC[1]
#    li    $26,    0x0000FF13    
#    mtc0  $26,    $12            # SR <= 0x0000FF13

    # jump to main in user mode: main[1]
#    la    $26,    seg_data_base
#    lw    $26,    4($26)         # $26 <= main[1]
#    mtc0  $26,    $14            # write it in EPC register
#    eret

    .set reorder

    .set reorder

    .endfunc
    .size    reset, .-reset

