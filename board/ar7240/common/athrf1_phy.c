/* 
 * Copyright (c) 2008, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <config.h>
#include <linux/types.h>
#include <common.h>
#include <miiphy.h>
#include "phy.h"
#include <asm/addrspace.h>
#include "ar7240_soc.h"
#include "athrf1_phy.h"
#include "ar7240_s26_phy.h"

#define MODULE_NAME "ATHRSF1_PHY"

#define ATHR_LAN_PORT_VLAN          1
#define ATHR_WAN_PORT_VLAN          2
#define ENET_UNIT_LAN 1
#define ENET_UNIT_WAN 0

#define TRUE    1
#define FALSE   0

#define ATHR_PHY_MAX 5
#define ATHR_PHY0_ADDR   0x0
#define ATHR_PHY1_ADDR   0x1
#define ATHR_PHY2_ADDR   0x2
#define ATHR_PHY3_ADDR   0x3
#define ATHR_PHY4_ADDR   0x4

#define sysMsDelay(_x) udelay((_x) * 1000)

/*
 * Track per-PHY port information.
 */
typedef struct {
    BOOL   isEnetPort;       /* normal enet port */
    BOOL   isPhyAlive;       /* last known state of link */
    int    ethUnit;          /* MAC associated with this phy port */
    uint32_t phyBase;
    uint32_t phyAddr;          /* PHY registers associated with this phy port */
    uint32_t VLANTableSetting; /* Value to be written to VLAN table */
} athrPhyInfo_t;

/*
 * Per-PHY information, indexed by PHY unit number.
 */

static athrPhyInfo_t athrPhyInfo[] = {

    {TRUE,   /* port 1 -- LAN port 1 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY4_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* port 2 -- LAN port 2 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY1_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* port 3 -- LAN port 3 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY2_ADDR,
     ATHR_LAN_PORT_VLAN
    },

    {TRUE,   /* port 4 --  LAN port 4 */
     FALSE,
     ENET_UNIT_LAN,
     0,
     ATHR_PHY3_ADDR,
     ATHR_LAN_PORT_VLAN   /* Send to all ports */
    },

    {TRUE,  /* port 5 -- WAN Port 5 */
     FALSE,
     ENET_UNIT_WAN,
     0,
     ATHR_PHY0_ADDR,
     ATHR_LAN_PORT_VLAN    /* Send to all ports */
    },

    {FALSE,   /* port 0 -- cpu port 0 */
     TRUE,
     ENET_UNIT_LAN,
     0,
     0x00,
     ATHR_LAN_PORT_VLAN
    },

};

#define ATHR_IS_ENET_PORT(phyUnit) (athrPhyInfo[phyUnit].isEnetPort)
#define ATHR_IS_PHY_ALIVE(phyUnit) (athrPhyInfo[phyUnit].isPhyAlive)
#define ATHR_ETHUNIT(phyUnit) (athrPhyInfo[phyUnit].ethUnit)
#define ATHR_PHYBASE(phyUnit) (athrPhyInfo[phyUnit].phyBase)
#define ATHR_PHYADDR(phyUnit) (athrPhyInfo[phyUnit].phyAddr)
#define ATHR_VLAN_TABLE_SETTING(phyUnit) (athrPhyInfo[phyUnit].VLANTableSetting)

#define ATHR_IS_ETHUNIT(phyUnit, ethUnit) \
            (ATHR_IS_ENET_PORT(phyUnit) &&        \
            ATHR_ETHUNIT(phyUnit) == (ethUnit))

#define ATHR_IS_WAN_PORT(phyUnit) (!(ATHR_ETHUNIT(phyUnit)==ENET_UNIT_LAN))
 
/* Forward references */
BOOL athr_phy_is_link_alive(int ethUnit, int phyUnit);
// JamesJHYeh deleted extern int ag7240_check_link(ag7240_mac_t *);

void athr_enable_linkIntrs(int ethUnit) 
{
    return;
}

void athr_disable_linkIntrs(int ethUnit) 
{
	return;
}
void athr_auto_neg(int ethUnit,int phyUnit) 
{ 
    int timeout = 0;
    uint16_t phyHwStatus;
    
    phy_reg_write(ethUnit, ATHR_PHYADDR(phyUnit) , ATHR_AUTONEG_ADVERT, ATHR_ADVERTISE_ALL);
    phy_reg_write(ethUnit, ATHR_PHYADDR(phyUnit) , ATHR_1000BASET_CONTROL, (ATHR_ADVERTISE_1000FULL));
    phy_reg_write(ethUnit, ATHR_PHYADDR(phyUnit) , ATHR_PHY_CONTROL, ATHR_CTRL_AUTONEGOTIATION_ENABLE | ATHR_CTRL_SOFTWARE_RESET);

//    printf(MODULE_NAME": phy_reg_read(%d,%d,0x2)=0x%x,\n", ethUnit, ATHR_PHYADDR(phyUnit),ag7240_miiphy_read(ag7240_unit2name(ethUnit), ATHR_PHYADDR(phyUnit), 0x2));
//    printf(MODULE_NAME": phy_reg_read(%d,%d,0x3)=0x%x,\n", ethUnit, ATHR_PHYADDR(phyUnit),ag7240_miiphy_read(ag7240_unit2name(ethUnit), ATHR_PHYADDR(phyUnit), 0x3));
    printf(MODULE_NAME": PHY unit 0x%x, address 0x%x, ID 0x%x,\n", ethUnit, ATHR_PHYADDR(phyUnit),ag7240_miiphy_read(ag7240_unit2name(ethUnit), ATHR_PHYADDR(phyUnit), 0x3));
   /*
     * Wait up to 3 seconds for ALL associated PHYs to finish
     * autonegotiation.  The only way we get out of here sooner is
     * if ALL PHYs are connected AND finish autonegotiation.
     */
    timeout=20;
    for (;;) {
        phyHwStatus = phy_reg_read(ethUnit, ATHR_PHYADDR(phyUnit), ATHR_PHY_CONTROL);

        if (ATHR_RESET_DONE(phyHwStatus)) {
            printf(MODULE_NAME": Port %d, Neg Success\n", phyUnit);
            break;
        }
        if (timeout == 0) {
            printf(MODULE_NAME": Port %d, Negogiation timeout\n", phyUnit);
            break;
        }
        if (--timeout == 0) {
            printf(MODULE_NAME": Port %d, Negogiation timeout\n", phyUnit);
            break;
        }

        sysMsDelay(150);
    }

    printf(MODULE_NAME": unit %d port %x phy addr %x \n", ethUnit, phyUnit, ATHR_PHYADDR(phyUnit));
}

/******************************************************************************
*
* athr_phy_is_link_alive - test to see if the specified link is alive
*
* RETURNS:
*    TRUE  --> link is alive
*    FALSE --> link is down
*/
BOOL
athr_phy_is_link_alive(int ethUnit, int phyUnit)
{
	uint16_t phyHwStatus;
	uint32_t phyBase;
	uint32_t phyAddr;

	phyBase = ATHR_PHYBASE(phyUnit);
	phyAddr = ATHR_PHYADDR(phyUnit);
	phyHwStatus = phy_reg_read(ethUnit, phyAddr, ATHR_PHY_SPEC_STATUS);

	if (phyHwStatus & ATHR_STATUS_LINK_PASS) {
//    printf(MODULE_NAME": athr_phy_is_link_alive TRUE \n");
		return TRUE;
	}

//	printf(MODULE_NAME": athr_phy_is_link_alive FALSE \n");
	return FALSE;
}

/******************************************************************************
*
* athr_phy_setup - reset and setup the PHY associated with
* the specified MAC unit number.
*
* Resets the associated PHY port.
*
* RETURNS:
*    TRUE  --> associated PHY is alive
*    FALSE --> no LINKs on this ethernet unit
*/

BOOL
athr_phy_setup(int ethUnit,int phyUnit)
{
	int       liveLinks = 0;

	athr_auto_neg(ethUnit,phyUnit);

//	printf(MODULE_NAME": After athr_auto_neg(ethUnit,phyUnit)\n");

	if (athr_phy_is_link_alive(ethUnit, phyUnit)) {
		liveLinks++;
		ATHR_IS_PHY_ALIVE(phyUnit) = TRUE;
//    printf(MODULE_NAME": athr_phy_setup ATHR_IS_PHY_ALIVE(phyUnit) = TRUE \n");
	} else {
		ATHR_IS_PHY_ALIVE(phyUnit) = FALSE;
//    printf(MODULE_NAME": athr_phy_setup ATHR_IS_PHY_ALIVE(phyUnit) = FALSE \n");
	}
	return (liveLinks > 0);
}

/******************************************************************************
*
* athr_phy_is_fdx - Determines whether the phy ports associated with the
* specified device are FULL or HALF duplex.
*
* RETURNS:
*    1 --> FULL
*    0 --> HALF
*/
int
athr_phy_is_fdx(int ethUnit,int phyUnit)
{
    uint32_t  phyBase;
    uint32_t  phyAddr;
    uint16_t  phyHwStatus;
    int       ii = 200;

//    printf(MODULE_NAME": athr_phy_is_fdx \n");
    if (athr_phy_is_link_alive(ethUnit, phyUnit)) {

         phyBase = ATHR_PHYBASE(phyUnit);
         phyAddr = ATHR_PHYADDR(phyUnit);

         do {
                phyHwStatus = phy_reg_read(ethUnit, ATHR_PHYADDR(phyUnit),ATHR_PHY_SPEC_STATUS);
                sysMsDelay(10);
          } while((!(phyHwStatus & ATHR_STATUS_RESOVLED)) && --ii);
          
          if (phyHwStatus & ATHER_STATUS_FULL_DEPLEX) {
                return TRUE;
          }
    }
    return FALSE;
}

/******************************************************************************
*
* athr_phy_speed - Determines the speed of phy ports associated with the
* specified device.
*
* RETURNS:
*               AG7240_PHY_SPEED_10T, AG7240_PHY_SPEED_100TX;
*               AG7240_PHY_SPEED_1000T;
*/

int
athr_phy_speed(int ethUnit,int phyUnit)
{
    uint16_t  phyHwStatus;
    uint32_t  phyBase;
    uint32_t  phyAddr;
    int       ii = 200;

    if (athr_phy_is_link_alive(ethUnit, phyUnit)) {

        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);
        do {
            phyHwStatus = phy_reg_read(ethUnit, ATHR_PHYADDR(phyUnit),ATHR_PHY_SPEC_STATUS);
            sysMsDelay(10);
        } while((!(phyHwStatus & ATHR_STATUS_RESOVLED)) && --ii);

        phyHwStatus = ((phyHwStatus & ATHER_STATUS_LINK_MASK) >>
                       ATHER_STATUS_LINK_SHIFT);

        switch(phyHwStatus) {
        case 0:
//            printf(MODULE_NAME": athr_phy_speed _10BASET\n");
            return _10BASET;
        case 1:
//            printf(MODULE_NAME": athr_phy_speed _100BASET\n");
            return _100BASET;
        case 2:
//            printf(MODULE_NAME": athr_phy_speed _1000BASET\n");
            return _1000BASET;
        default:
            printf("Unkown speed read!\n");
        }
    }

    return _10BASET;
}

/*****************************************************************************
*
* athr_phy_is_up -- checks for significant changes in PHY state.
*
* A "significant change" is:
*     dropped link (e.g. ethernet cable unplugged) OR
*     autonegotiation completed + link (e.g. ethernet cable plugged in)
*
* When a PHY is plugged in, phyLinkGained is called.
* When a PHY is unplugged, phyLinkLost is called.
*/

int
athr_phy_is_up(int ethUnit, int phyUnit)
{
    uint16_t      phyHwStatus, phyHwControl;
    athrPhyInfo_t *lastStatus;
    int           linkCount   = 0;
    int           lostLinks   = 0;
    int           gainedLinks = 0;
    uint32_t      phyBase;
    uint32_t      phyAddr;

//    printf(MODULE_NAME": athr_phy_is_up \n");
    for (phyUnit=0; phyUnit < 1; phyUnit++) {

        phyBase = ATHR_PHYBASE(phyUnit);
        phyAddr = ATHR_PHYADDR(phyUnit);

        lastStatus = &athrPhyInfo[phyUnit];

        if (lastStatus->isPhyAlive) { /* last known link status was ALIVE */

             phyHwStatus = phy_reg_read(ethUnit, ATHR_PHYADDR(phyUnit),ATHR_PHY_SPEC_STATUS);

            /* See if we've lost link */
            if (phyHwStatus & ATHR_STATUS_LINK_PASS) { /* check realtime link */
                linkCount++;
            } else {
                phyHwStatus = phy_reg_read(ethUnit, ATHR_PHYADDR(phyUnit),ATHR_PHY_STATUS);
            /* If realtime failed check link in latch register before asserting link down. */
                if (phyHwStatus & ATHR_LATCH_LINK_PASS)     
                   linkCount++;
		else
                    lostLinks++;
                lastStatus->isPhyAlive = FALSE;
            }
        } else { /* last known link status was DEAD */

            /* Check for reset complete */

                phyHwStatus = phy_reg_read(ethUnit, ATHR_PHYADDR(phyUnit),ATHR_PHY_STATUS);

            if (!ATHR_RESET_DONE(phyHwStatus))
                continue;

                phyHwControl = phy_reg_read(ethUnit, ATHR_PHYADDR(phyUnit),ATHR_PHY_CONTROL);

            /* Check for AutoNegotiation complete */ 
           
            if ((!(phyHwControl & ATHR_CTRL_AUTONEGOTIATION_ENABLE)) 
                 || ATHR_AUTONEG_DONE(phyHwStatus)) {
                    phyHwStatus = phy_reg_read(ethUnit, ATHR_PHYADDR(phyUnit),ATHR_PHY_SPEC_STATUS);

                    if (phyHwStatus & ATHR_STATUS_LINK_PASS) {
                        gainedLinks++;
                        linkCount++;
                        lastStatus->isPhyAlive = TRUE;
                   }  
            }
        }
    }
    return (linkCount);

}

/******************************************************************************
*
* athr_reg_init - A Pseudo Function
*/

BOOL
athr_reg_init()
{
//	printf(MODULE_NAME": athr_reg_init \n");
}
