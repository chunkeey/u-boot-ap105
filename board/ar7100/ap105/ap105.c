/*****************************************************************************/
/*! file ap105.c
** /brief Boot support for AP105 board - based on AP96
**
**  This provides the support code required for the AP96 board in the U-Boot
**  environment.  This board is a Hydra based system with two Merlin WLAN
**  interfaces and S16 GigE interfaces.
**
**  Code derived from AP94
**
**  AP96 is similar to AP94 in all respects except the ethernet interfaces.
**  AP94 has S26, while AP96 uses S16. PB45 also uses S16, but PB45 has a
**  32 bit wide memory bus and two mini-PCI slots whereas AP94 and AP96
**  have a 16 bit wide memory bus and WLAN interfaces on the board. So,
**  while PB45 doesn't have a caldata partition in flash, AP94 and AP96
**  will have the radio calibration of the WLAN interfaces in Flash ROM.
**
*/

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include "ar7100_soc.h"

extern flash_info_t flash_info[];

int board_gpio_config(void)
{
    /* Enable UART on GPIO9-10 */
    ar7100_reg_wr(AR7100_GPIO_FUNC, 1<<8);
    /* Turn off RED Leds (GPIO 2,8,11) */
    ar7100_reg_wr(AR7100_GPIO_OUT, 0x00000904);
    /* Now enable GPIO0-5,8, 10-11 as outputs */
    ar7100_reg_wr(AR7100_GPIO_OE,  0x00000d3f);

    return 0;
}

/******************************************************************************/
/*!
**  \brief ar7100_mem_config
**
**  This is a "C" level implementation of the memory configuration interface
**  for the PB45.
**
**  \return RAM size in bytes
*/

int
ar7100_mem_config(void)
{
    /* XXX - should be set based board configuration */
    *(volatile unsigned int *)0xb8050004 = 0x50C0;
    udelay(10);
    *(volatile unsigned int *)0xb8050018 = 0x1313;
    udelay(10);
    *(volatile unsigned int *)0xb805001c = 0x67;
    udelay(10);
    *(volatile unsigned int *)0xb8050010 = 0x1099;
    udelay(10);

    return (ar7100_ddr_find_size());
}

/******************************************************************************/
/*!
**  \brief ar7100_usb_initial_config
**
**  -- Enter Detailed Description --
**
**  \param param1 Describe Parameter 1
**  \param param2 Describe Parameter 2
**  \return Describe return value, or N/A for void
*/

long int initdram(int board_type)
{
#ifdef DEBUG
    printf("b8050000: 0x%x\n",* (unsigned long *)(0xb8050000));
#endif
    return (ar7100_mem_config());
}

/* GPIO 6 is our reset */
#define GPIO_RESET  (1<<6)

int check_reset_button(void)
{
    if((ar7100_reg_rd(AR7100_GPIO_IN ) & GPIO_RESET)) {
        printf("RESET is un-pushed\n");
    } else {
        printf("RESET is pushed\n");
        /* Flash red WIFI LEDs to let people know we are in reset */
        ar7100_reg_wr(AR7100_GPIO_OUT, 0x00000004); /* Red WIFI LEDs On */
        udelay(600000);
        ar7100_reg_wr(AR7100_GPIO_OUT, 0x00000904); /* Red WIFI LEDs Off */
        udelay(600000);
        run_command("run resetcmd", 1);
        }
    return 0;
}

int checkboard (void)
{
    printf("AP-10x (ar7100) U-boot " ATH_AP96_UBOOT_VERSION "\n");
    return 0;
}

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long
flash_get_geom (flash_info_t *flash_info)
{
    int i;

    /* XXX this is hardcoded until we figure out how to read flash id */

    flash_info->flash_id  = FLASH_25L12845;
    flash_info->size = 16*1024*1024; /* bytes */
    flash_info->sector_count = flash_info->size/CFG_FLASH_SECTOR_SIZE;

    for (i = 0; i < flash_info->sector_count; i++) {
        flash_info->start[i] = CFG_FLASH_BASE + (i * CFG_FLASH_SECTOR_SIZE);
        flash_info->protect[i] = 0;
    }
#ifdef DEBUG
    printf ("flash size 16MB, sector count = %d\n", flash_info->sector_count);
#endif
    return (flash_info->size);

}
