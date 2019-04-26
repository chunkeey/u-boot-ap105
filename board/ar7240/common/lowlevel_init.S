#include <config.h>
#include <version.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <ar7240_soc.h>

/*
 * Helper macros.
 * These Clobber t7, t8 and t9
 */
#define clear_mask(_reg, _mask)                     \
    li  t7, KSEG1ADDR(_reg);                                   \
    lw  t8, 0(t7);                                  \
    li  t9, ~_mask;                                 \
    and t8, t8, t9;                                 \
    sw  t8, 0(t7)            

#define set_val(_reg, _mask, _val)                  \
    li  t7, KSEG1ADDR(_reg);                                   \
    lw  t8, 0(t7);                                  \
    li  t9, ~_mask;                                 \
    and t8, t8, t9;                                 \
    li  t9, _val;                                   \
    or  t8, t8, t9;                                 \
    sw  t8, 0(t7)            

#define get_val(_reg, _mask, _shift, _res_reg)      \
    li  t7, KSEG1ADDR(_reg);                                   \
    lw  t8, 0(t7);                                  \
    li  t9, _mask;                                  \
    and t8, t8, t9;                                 \
    srl _res_reg, t8, _shift                        \

#define pll_clr(_mask)                              \
    clear_mask(AR7240_CPU_PLL_CONFIG, _mask)

#define pll_set(_mask, _val)                        \
    set_val(AR7240_CPU_PLL_CONFIG,  _mask, _val)

#define pll_get(_mask, _shift, _res_reg)            \
    get_val(AR7240_CPU_PLL_CONFIG, _mask, _shift, _res_reg)

#define clk_clr(_mask)                               \
    clear_mask(AR7240_CPU_CLOCK_CONTROL, _mask)

#define clk_set(_mask, _val)                         \
    set_val(AR7240_CPU_CLOCK_CONTROL,  _mask, _val)

#define clk_get(_mask, _shift, _res_reg)              \
    get_val(AR7240_CPU_CLOCK_CONTROL, _mask, _shift, _res_reg)


/******************************************************************************
 * first level initialization:
 * 
 * 0) If clock cntrl reset switch is already set, we're recovering from 
 *    "divider reset"; goto 3.
 * 1) Setup divide ratios.
 * 2) Reset.
 * 3) Setup pll's, wait for lock.
 * 
 *****************************************************************************/

.globl lowlevel_init

lowlevel_init:

    /*
     * The code below is for the real chip. Wont work on FPGA
     */
    /* jr ra  */
#ifndef CONFIG_AR7240_EMU

    /*
     * Check if the PLL is already set and CPU is Reset
     */
    clk_get(CLOCK_CONTROL_RST_SWITCH_MASK, CLOCK_CONTROL_RST_SWITCH_SHIFT, t6)
    bne zero, t6, initialize_pll
    nop

init_pll_values:

    pll_clr(PLL_CONFIG_PLL_RESET_MASK)
    pll_set( (PLL_CONFIG_DDR_DIV_MASK | PLL_CONFIG_AHB_DIV_MASK | PLL_CONFIG_PLL_NOPWD_MASK | PLL_CONFIG_PLL_REF_DIV_MASK | PLL_CONFIG_PLL_DIV_MASK) ,( PLL_CONFIG_PLL_REF_DIV_VAL|PLL_CONFIG_PLL_DIV_VAL|PLL_CONFIG_AHB_DIV_VAL|PLL_CONFIG_DDR_DIV_VAL|PLL_CONFIG_PLL_NOPWD_VAL))
    pll_clr(PLL_CONFIG_PLL_BYPASS_MASK) 

wait_for_pll_update:
    pll_get(PLL_CONFIG_PLL_UPDATE_MASK, PLL_CONFIG_PLL_UPDATE_SHIFT, t6)
    bne zero, t6, wait_for_pll_update
    nop

    /*
     * Will cause a reset
     * The RESET_SWITCH need to be set first and then 
     * set the CLOCK_SWITCH for the CPU to boot properly
     * after RESET.
     */
pll_locked:
    clk_set(CLOCK_CONTROL_RST_SWITCH_MASK, 0x2)
    clk_set(CLOCK_CONTROL_CLOCK_SWITCH_MASK, 0x1)
    nop

    /*
     * When the PLL is already set and CPU is RESET
     * The code will jump here
     */
initialize_pll:
    clk_clr(CLOCK_CONTROL_RST_SWITCH_MASK)
    clk_clr(CLOCK_CONTROL_CLOCK_SWITCH_MASK)
#endif
    jr ra
    nop

