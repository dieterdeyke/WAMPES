#ifdef linux

#ifdef __x86_64__

asm(".global setstack\n\t"
    "setstack:\n\t"
    "mov %rsp, %rbp\n\t"
    "mov newstackptr(%rip), %rsp\n\t"
    "jmp *(%rbp)");

#else

asm(".global setstack\n\t"
    "setstack:\n\t"
    "movl %esp, %ebp\n\t"
    "movl newstackptr, %esp\n\t"
    "jmp *(%ebp)");

#endif

#endif
