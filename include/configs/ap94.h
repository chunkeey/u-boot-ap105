/*
 * This file contains the configuration parameters for the Aruba AP-104/5 Board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7100.h>

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS     1       /* max number of memory banks */
#define CFG_MAX_FLASH_SECT      256     /* max number of sectors on one chip */
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE          0x01000000 /* Total flash size */

#define CFG_FLASH_WORD_SIZE     unsigned short
#define CFG_FLASH_ADDR0         (0x5555)
#define CFG_FLASH_ADDR1         (0x2AAA)

/*
 * We boot from this flash
 */
#define CFG_FLASH_BASE          0xbf000000

#undef CONFIG_ROOTFS_RD
#undef CONFIG_ROOTFS_FLASH
#undef CONFIG_BOOTARGS_FL
#undef CONFIG_BOOTARGS_RD
#undef CONFIG_BOOTARGS
#undef  MTDPARTS_DEFAULT
#undef  MTDIDS_DEFAULT

#define CONFIG_ROOTFS_FLASH
#define CONFIG_BOOTARGS CONFIG_BOOTARGS_FL

#define CONFIG_BOOTARGS_FL     "console=ttyS0,9600 mtdparts=ar7100-nor0:256k(uboot),64k(env),16000k(firmware),64k(hwenv)"

#define MTDPARTS_DEFAULT    "mtdparts=ar7100-nor0:256k(u-boot),64k(env),16000k(firmware),64k(hwenv)"
#define MTDIDS_DEFAULT      "nor0=ar7100-nor0"

/*
 * The following #defines are needed to get flash environment right
 */
#define CFG_MONITOR_BASE    TEXT_BASE
#define CFG_MONITOR_LEN     (256 << 10)

#undef CFG_HZ
/*
 * MIPS32 24K Processor Core Family Software User's Manual
 *
 * 6.2.9 Count Register (CP0 Register 9, Select 0)
 * The Count register acts as a timer, incrementing at a constant
 * rate, whether or not an instruction is executed, retired, or
 * any forward progress is made through the pipeline.  The counter
 * increments every other clock, if the DC bit in the Cause register
 * is 0.
 */

#define CFG_HZ	(680000000/2)

/*
 * timeout values are in ticks
 */
#define CFG_FLASH_ERASE_TOUT    (2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT    (2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET  0x1000

#define CFG_ENV_IS_IN_FLASH    1
#undef CFG_ENV_IS_NOWHERE

/* Address and size of Primary Environment Sector   */
#define CFG_ENV_ADDR        0xbf040000
#define CFG_ENV_SIZE        0x10000

#define CONFIG_BOOTCOMMAND "run bootap"

/*
** Extra environmental variables useful for loading the board
*/

#define CONFIG_EXTRA_ENV_SETTINGS \
"loadUboot=tftpboot 0x80010000 u-boot.bin && erase 0xbf000000 +0x40000 && cp.b 0x80010000 0xbf000000 0x40000\0" \
"loadLinux=tftpboot 0x80010000 firmware.bin && erase 0xbf050000 +0xfa0000 && cp.b 0x80010000 0xbf050000 $filesize\0" \
"loadStock=tftpboot 0x80010000 ap-105-stock.bin && erase 0xbf000000 +0x1000000 && cp.b 0x80010000 0xbf000000 $filesize\0" \
"bootLinux=bootm 0xbf050000;\0" \
"bootap=run bootLinux; run loadLinux bootLinux; run bootap;\0"

#define CONFIG_NR_DRAM_BANKS    		2

/*
** Parameters defining the location of the calibration/initialization
** information for the two Merlin devices.
** NOTE: **This will change with different flash configurations**
*/

#define ATHEROS_PRODUCT_ID              136
#define CAL_SECTOR                      127

/*
** Configure the CPU clock settings
*/

#define CFG_PLL_FREQ	CFG_PLL_680_340_170
/*
** These values for DDR configuration came from the
** BDM configuration script
*/
#define CFG_DDR_32BIT	1

#define CFG_DDR_REFRESH_VAL     		0x461b
#ifdef CFG_DDR_32BIT
#define CFG_DDR_CONFIG_VAL      		0x77b8884e
#define CFG_DDR_CONFIG2_VAL             0x812cd6a8
#else
#define CFG_DDR_CONFIG_VAL      		0xefbc8cd0
#define CFG_DDR_CONFIG2_VAL             0x8e7156a2
#endif
#define CFG_DDR_MODE_VAL_INIT   		0x131
#define CFG_DDR_EXT_MODE_VAL    		0x0
#ifdef CFG_DDR_32BIT
#define CFG_DDR_MODE_VAL        		0x33
#else
#define CFG_DDR_MODE_VAL        		0x61
#endif
#define CFG_DDR_TRTW_VAL        		0x1f
#define CFG_DDR_TWTR_VAL        		0x1e
#ifdef CFG_DDR_32BIT
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL  0xff
#else
#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL  0xffff
#endif


#define CONFIG_NET_MULTI
#define CONFIG_MEMSIZE_IN_BYTES
/* AP-105 PCI seems to be broken */
//#define CONFIG_PCI


/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CONFIG_COMMANDS (( CONFIG_CMD_DFL        | CFG_CMD_MII   | CFG_CMD_PING  \
   | CFG_CMD_NET | CFG_CMD_PCI     | CFG_CMD_ENV | CFG_CMD_FLASH | CFG_CMD_LOADS \
   | CFG_CMD_RUN | CFG_CMD_LOADB   | CFG_CMD_ELF | CFG_CMD_BSP ))

/*
** Ethernet Configuration
** Set default values, and select the PHY to use
*/

#define CONFIG_IPADDR       		192.168.1.1
#define CONFIG_SERVERIP     		192.168.1.2
//#define CONFIG_ETHADDR      		00:03:7f:ff:ff:fe
#define CFG_FAULT_ECHO_LINK_DOWN 	1
#define CONFIG_PHY_GIGE       		1              /* GbE speed/duplex detect */

#define CONFIG_LZMA			1
#define CFG_VSC8601_PHY 		1
#define CFG_AG7100_NMACS 		1
//#define CFG_ATHRHDR_REG 		1
//#define CFG_SWITCH_FREQ		1

//#define CFG_ATHRHDR_EN 		1
//#define ATHRHDR_LEN   		2
//#define ATHRHDR_MAX_DATA  		20

#define CFG_PHY_ADDR			0  /* Port 0 */
#define CFG_GMII     			0
#define CFG_MII0_RMII			0
#define CFG_MII1_RMII			0
#define CFG_AG7100_GE0_RGMII		1

/*
** Configure Parser
*/

#define CFG_BOOTM_LEN   (16 << 20) /* 16 MB */
#define DEBUG
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "hush>"

#include <cmd_confdefs.h>

#define GPIO_PWR_RED_LED 2
#define GPIO_PWR_GREEN_LED 3
#define GPIO_11BGN_RED_LED 8
#define GPIO_11AN_RED_LED 11

#endif  /* __CONFIG_H */
