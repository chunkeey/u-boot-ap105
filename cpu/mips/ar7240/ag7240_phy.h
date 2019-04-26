#ifndef _AG7240_PHY_H
#define _AG7240_PHY_H

#define ag7240_phy_setup(unit) do { \
    athrs26_phy_setup(unit); \
} while (0);

#define ag7240_phy_link(unit,link,fdx,speed) do { \
    link = athrs26_phy_is_up(unit); \
} while (0);

#define ag7240_phy_duplex(unit,duplex) do { \
    duplex =  athrs26_phy_is_fdx (unit); \
} while (0);

#define ag7240_phy_speed(unit,speed) do { \
        speed = athrs26_phy_speed (unit); \
} while (0);

#endif /*_AG7240_PHY_H*/
