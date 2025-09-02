__attribute__((noinline)) void knowncall()
{
    __asm__ volatile(
        "fadd.s fa0, fa1, fa2" ::: "fa0", "fa1", "fa2");
}
