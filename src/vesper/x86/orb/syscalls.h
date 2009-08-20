inline void kernel_syscall(int code)
{
    asm volatile("mov %0, %%eax\n"
                 "sysenter", "a"(code));
}

inline void kernel_interface_syscall()
{
    kernel_syscall(1);
}
