#ifdef linux

asm(".global setstack\n\t"
    "setstack:\n\t"
    "movl %esp, %ebp\n\t"
    "movl newstackptr, %esp\n\t"
    "jmp *(%ebp)");

#endif
