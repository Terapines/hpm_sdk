/*
 * Copyright (c) 2021 HPMicro
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * NOTE: This test only supported in hpm6750evkmini.
 * This file contains test for irq save/restore feature in zcc.
 * 1. isr_test_fadd: call a function with float instruction
 * 2. isr_test_fadd_inlineasm: use inline asm with float instruction
 * 3. isr_test_fcsr: use float instruction directly in isr
 * 4. isr_test_fcsr_inlineasm: use inline asm with float instruction directly in isr
 * 5. isr_test_nofcsr: use add instruction directly in isr
 * 6. isr_test_tp: use tp register in a function
 * 7. isr_test_recursive: call a recursive function
 * 8. isr_test_add: call a function without float instruction
 * 9. isr_test_add_inlineasm: use inline asm without float instruction
 * 10. isr_test_fadd_outer: call a function which calls another function with float instruction
 * 11. isr_test_fadd_outer_inlineasm: call a function which calls another function with float instruction in inline asm
 * 12. isr_test_add_outer: call a function which calls another function with add instruction
 * 13. isr_test_add_outer_inlineasm: call a function which calls another function with add instruction in inline asm
 * 14. isr_test_nocall: no function call, only use add and fadd instruction directly in isr
 * 15. isr_test_unknowncall: call an unknown function
 * 16. isr_test_unknowncall: call an known external function
 * 17. isr_test_unknowncall_fp: call an unknown function through function pointer
 * 18. isr_test_ucode: call a function with dsp instruction
 * 19. isr_test_ucode_inlineasm: use inline asm with dsp instruction
 */

#include <stdio.h>
#include "board.h"
#include "hpm_debug_console.h"

#define LED_FLASH_PERIOD_IN_MS 300

float a, b, c;
int d, e, f;
float f_result;
int i_result;

__attribute__((noinline)) float test_fadd(float a, float b)
{
    f_result = a + b;
    return f_result;
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPIO0_Y, isr_test_fadd)
void isr_test_fadd(void)
{
    f_result = test_fadd(3.0, 4.0);
}

__attribute__((noinline)) float test_fadd_inlineasm(float a, float b)
{
    float result;
    __asm__ volatile(
        "fadd.s fa0, fa1, fa2" ::: "fa0", "fa1", "fa2");
    return result;
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPIO0_A, isr_test_fadd_inlineasm)
void isr_test_fadd_inlineasm(void)
{
    f_result = test_fadd_inlineasm(3.0, 4.0);
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPIO0_B, isr_test_fcsr)
void isr_test_fcsr(void)
{
    a = b + c;
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPIO0_C, isr_test_fcsr_inlineasm)
void isr_test_fcsr_inlineasm(void)
{
    __asm__ volatile(
        "fadd.s fa0, fa1, fa2" ::: "fa0", "fa1", "fa2");
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPIO0_D, isr_test_nofcsr)
void isr_test_nofcsr(void)
{
    d = e + f;
}

__attribute__((noinline)) void test_tp()
{
    __asm__ volatile("add a0, a0, tp" ::: "a0", "tp");
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPIO0_E, isr_test_tp)
void isr_test_tp(void)
{
    test_tp();
}

__attribute__((noinline)) float test_recursive(float a, float b, int n)
{
    if (n <= 0)
    {
        return a + b;
    }

    float tmp = (a * 0.9f) + (b * 1.1f);
    return tmp + test_recursive(a, b, n - 1);
}

// Recursive function will save all registers now.
SDK_DECLARE_EXT_ISR_M(IRQn_GPIO0_F, isr_test_recursive)
void isr_test_recursive(void)
{
    f_result = test_recursive(3.0, 4.0, 10);
}

__attribute__((noinline)) int test_add(int a, int b)
{
    i_result = a + b;
    return i_result;
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPIO0_X, isr_test_add)
void isr_test_add(void)
{
    i_result = test_add(3.0, 4.0);
}

__attribute__((noinline)) int test_add_inlineasm(int a, int b)
{
    int result;
    __asm__ volatile(
        "add a0, a1, a2" ::: "a0", "a1", "a2");
    return result;
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPTMR0, isr_test_add_inlineasm)
void isr_test_add_inlineasm(void)
{
    i_result = test_add_inlineasm(3.0, 4.0);
}

__attribute__((noinline)) void test_fadd_inner()
{
    a = b + c;
}

__attribute__((noinline)) void test_fadd_outer()
{
    test_fadd_inner();
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPTMR1, isr_test_fadd_outer)
void isr_test_fadd_outer(void)
{
    test_fadd_outer();
}

__attribute__((noinline)) void test_fadd_inner_inlineasm()
{
    __asm__ volatile(
        "fadd.s fa0, fa1, fa2" ::: "fa0", "fa1", "fa2");
}

__attribute__((noinline)) void test_fadd_outer_inlineasm()
{
    test_fadd_inner_inlineasm();
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPTMR2, isr_test_fadd_outer_inlineasm)
void isr_test_fadd_outer_inlineasm(void)
{
    test_fadd_outer_inlineasm();
}

__attribute__((noinline)) void test_add_inner()
{
    d = e + f;
}

__attribute__((noinline)) void test_add_outer()
{
    test_add_inner();
}

SDK_DECLARE_EXT_ISR_M(IRQn_GPTMR3, isr_test_add_outer)
void isr_test_add_outer(void)
{
    test_add_outer();
}

__attribute__((noinline)) void test_add_inner_inlineasm()
{
    __asm__ volatile(
        "add a0, a1, a2" ::: "a0", "a1", "a2");
}

__attribute__((noinline)) void test_add_outer_inlineasm()
{
    test_add_inner_inlineasm();
}

SDK_DECLARE_EXT_ISR_M(IRQn_UART0, isr_test_add_outer_inlineasm)
void isr_test_add_outer_inlineasm(void)
{
    test_add_outer_inlineasm();
}

SDK_DECLARE_EXT_ISR_M(IRQn_UART2, isr_test_nocall)
void isr_test_nocall(void)
{
    a = b + c;
    d = e + f;
}

extern void unknowncall();
SDK_DECLARE_EXT_ISR_M(IRQn_UART3, isr_test_unknowncall)
void isr_test_unknowncall(void)
{
    unknowncall();
}

extern void knowncall();
SDK_DECLARE_EXT_ISR_M(IRQn_UART7, isr_test_knowncall)
void isr_test_knowncall(void)
{
    knowncall();
}

typedef void (*foo_t)();
foo_t fp;
SDK_DECLARE_EXT_ISR_M(IRQn_UART4, isr_test_unknowncall_fp)
void isr_test_unknowncall_fp(void)
{
    fp();
}

#ifdef __riscv_dsp
SDK_DECLARE_EXT_ISR_M(IRQn_UART5, isr_test_ucode)
void isr_test_ucode(void)
{
    d = __builtin_riscv_mulsr64(e, f);
}

// Unable to analyze p instruction in inline asm, so ucode is not saved.
SDK_DECLARE_EXT_ISR_M(IRQn_UART6, isr_test_ucode_inlineasm)
void isr_test_ucode_inlineasm(void)
{
    __asm__ volatile(
        "smal a0, a0, a2" ::: "a0", "a1", "a2", "a3");
}
#endif

int main(void)
{
    int u;
    board_init();
    board_init_led_pins();

    board_timer_create(LED_FLASH_PERIOD_IN_MS, board_led_toggle);

    printf("hello world\n");
    while (1)
    {
        u = getchar();
        if (u == '\r')
        {
            u = '\n';
        }
        printf("%c", u);
    }
    return 0;
}
