#include <common.h>
#include <command.h>
#include <ar7100_soc.h>
#include <asm-mips/addrspace.h>
#include <miiphy.h>



#define	STATUS_LED_OFFSET	1
#define	WLAN_LED_OFFSET		6
#define	PWR_LED_OFFSET		14
#define	SOUT_OFFSET			10
#define	SIN_OFFSET			13

#define	RAM_TEST_START		0x80000000
#define RAM_TEST_END		0x80f00000

#define	FLASH_TEST_START	0xbf290000
#define FLASH_TEST_END		0xbf2affff

#define MEM_TEMP_ADDR		0x80060000
#define MANU_START_ADDR		0xbf7e0000
#define ART_START_ADDR		0xbf7f0000
#define ART_SIZE			0x10000
#define MANU_SIZE			0x10000
#define MAC0_OFFSET			0x1000

#define GIGA_NORMAL			(0x1<<13 | 0x1<<14 | 0x1<<15)
#define GIGA_TEST_1			(0x1<<13)
#define GIGA_TEST_2			(0x1<<14)
#define GIGA_TEST_3			(0x1<<13|0x1<<14)
#define GIGA_TEST_4			(0x1<<15)

#define GPIO_PWR_RED_LED 2
#define GPIO_PWR_GREEN_LED 3
#define GPIO_11BGN_RED_LED 8
#define GPIO_11AN_RED_LED 11

struct gpio_op{
	int offset;
	void (*output)(unsigned int reg_base, unsigned int gpio_offset, int value);
};


extern flash_info_t flash_info[CFG_MAX_FLASH_BANKS];
extern int  ag7100_miiphy_read(char *devname, unsigned char phaddr,
		unsigned char reg, unsigned short *data);
extern int  ag7100_miiphy_write(char *devname, unsigned char phaddr,
		unsigned char reg, unsigned short data);



void GpioSet(unsigned int reg_base, unsigned int gpio_offset, int value)
{
	unsigned int port_value;
	unsigned long gpio_reg = KSEG1ADDR(reg_base);

	/*prohibit to operate on rx or tx pin*/
	if (gpio_offset == SOUT_OFFSET || gpio_offset == SIN_OFFSET){
		return;
	}

	//printf("gpio_offset %d\n",gpio_offset);
	port_value = *((volatile unsigned long*)gpio_reg);
	//printf("1 portvalue %x\n",port_value);

	if (value){
		port_value |= (0x01 << gpio_offset);
	} else{
		port_value &= ~(0x01 << gpio_offset);
	}

	//printf("2 portvalue %x\n",port_value);
	*((volatile unsigned long *)gpio_reg) = port_value;
}

int gpio_read(unsigned int gpio_offset)
{
	unsigned long value;

	/*prohibit to operate on rx or tx pin*/
	if (gpio_offset == SOUT_OFFSET || gpio_offset == SIN_OFFSET){
		return -1;
	}

	GpioSet (AR7100_GPIO_OE, gpio_offset, 0);

	value = *(volatile unsigned long *)KSEG1ADDR(AR7100_GPIO_IN);

	return (value >> gpio_offset)&0x01;


}


int do_gpio_test (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long offset;
	if (argc < 3){
		printf("%s\n",cmdtp->help);
		return -1;
	}

	offset = simple_strtoul(argv[1],NULL,10);
	GpioSet (AR7100_GPIO_OE, offset, 1);
	if (!strncmp(argv[2],"off",3)){
		GpioSet (AR7100_GPIO_OUT, offset, 1);
	} else if(!strncmp(argv[2],"on",2)){
		GpioSet (AR7100_GPIO_OUT, offset, 0);
	}
	return 0;
}



U_BOOT_CMD(
	gpio,    3,    1,    do_gpio_test,
	"gpio  - gpio testing command\n",
	"gpio [which] on\n\tlight gpio on\ngpio [which] off\n\tlight gpio off\n"
);


void lit_gpio_flow(void)
{
	int i, j;
	int num;
	struct gpio_op led_gpio_pin[] = {
			{GPIO_PWR_RED_LED,	GpioSet},
			{GPIO_PWR_GREEN_LED, 	GpioSet},
			{GPIO_11BGN_RED_LED, 	GpioSet},
			{GPIO_11AN_RED_LED, 	GpioSet},
	};

	num = sizeof(led_gpio_pin)/sizeof(struct gpio_op);

	for (j = 0 ; j < 10 ; j++){

		for (i = 0; i < num; i++){
			led_gpio_pin[i].output(AR7100_GPIO_OUT, led_gpio_pin[i].offset, 0);
		}
		led_gpio_pin[1].output(AR7100_GPIO_OUT, led_gpio_pin[1].offset, (j+1)%2);
		udelay(400000);
		for (i = 0; i < num; i++){
			led_gpio_pin[i].output(AR7100_GPIO_OUT, led_gpio_pin[i].offset, 1);
		}
		udelay(400000);
	}
}

void test_gpio(void)
{
	int flag = 0;
	unsigned char state1 = 0x00;
	unsigned char state2 = 0x00;

#if 0
	do{
		state1 = gpio_read(GPIO_RST_BTN);
		udelay(1000);
		state2 = gpio_read(GPIO_RST_BTN);
		if (state1 == !state2){
			flag |= 0x10;
			printf("reset button was pressed down\n");
			lit_gpio_flow();
			if (flag == 0x11)
				return;
		}

#if 0
		state1 = gpio_read(GPIO_WPS_BTN);
		udelay(1000);
		state2 = gpio_read(GPIO_WPS_BTN);

		if (state1 == !state2){
			flag |= 0x01;
			printf("wps button was pressed down\n");
			lit_gpio_flow();
			if (flag == 0x11)
				return;
		}
#endif

		if(ctrlc()){
			puts("Abort!\n");
			return ;
		}

	}while(1);
#endif
}

/*
*	time----minutes
*/
void test_ram(unsigned long time)
{
	unsigned long *start = (unsigned long *)RAM_TEST_START;
	unsigned long *end = (unsigned long *)RAM_TEST_END;
	unsigned long pdata = 0x12345678;
	unsigned long readback;
	volatile unsigned long *p;
	//int i = 0;
	//start = RAM_TEST_START;
	//end = RAM_TEST_END;
	ulong last,now,during;

	during = time*60*CFG_HZ;
	//reset_timer();
	last = get_timer(0);

	do{
		now = get_timer(0);
		printf("checking ram from %x to %x\n", start, end);
		for(p= start; p <= end; p++){
			*p = pdata;
		}
		printf("Verifying data in the ram...\n");
		for(p=start; p <= end; p++){
			readback = *p;
			if (readback != pdata){
				printf("\n\tRam testing failure @%x expected %x actual %x\n\n", p, pdata, readback);
				return;
			}
		}
	}while(now-last <= during);

	printf("\n\tRam testing passed for %d minutes\n\n", time);



}
/*
*	time----minutes
*/
void test_flash(unsigned long time)
{
	int s_first,s_last;
	unsigned long pdata = 0x12345678;
	volatile unsigned long *p;
	unsigned long readback;
	unsigned int i;
	ulong last,now,during;

	during = time*60*CFG_HZ;
	//reset_timer();
	last = get_timer(0);

	p = (unsigned long *)MEM_TEMP_ADDR;
	/*fill the test data to RAM (128K)*/
	for (i = 0 ; i < (128*1024)/4; i++){
		*p++ = pdata;
	}
	do{
		now = get_timer(0);
		printf("checking flash from %x to %x\n", FLASH_TEST_START, FLASH_TEST_END);
		s_first = (FLASH_TEST_START - CFG_FLASH_BASE)/CFG_FLASH_SECTOR_SIZE;
		s_last = ((FLASH_TEST_END + 1 - CFG_FLASH_BASE)/CFG_FLASH_SECTOR_SIZE) - 1;
		flash_erase(&flash_info[0],s_first,s_last);
		flash_write((uchar *)MEM_TEMP_ADDR, FLASH_TEST_START, 128*1024);

		/*verifying the data in flash*/
		p = (unsigned long *)FLASH_TEST_START;
		for (i = 0 ; i < (128*1024)/4; i++){
			readback = *p++;
			if (readback != pdata){
				printf("\n\tFlash testing failure @%x expected %x actual %x\n\n", p, pdata, readback);
				return;
			}
		}
	}while(now-last <= during);

	printf("\n\tFlash checking passed for %d mimutes\n\n", time);

}


int do_mem_test (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long time;
	if (argc < 3){
		printf("%s\n",cmdtp->help);
		return -1;
	}

	time = simple_strtoul(argv[2], NULL, 10);

	if (!strcmp( "ram", argv[1])){
		test_ram(time);
	} else if (!strcmp("flash", argv[1])){
		test_flash(time);
	}

	return 0;

}



U_BOOT_CMD(
	memtest,    4,    1,    do_mem_test,
	"memtest  - memory testing command\n",
	"memtest ram|flash time\n"
);


void do_delay(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf("Current CFG setting %d\n", CFG_HZ);
	udelay(5000000);
}

U_BOOT_CMD(
	delay,    2,    1,    do_delay,
	"delay  - delay \n",
	"dealy [n]\n"
);

static unsigned char x2c(unsigned char *what)
{
	register char digit;
	char low;
	char high;

	high = what[0];
	low = what[1];

	if (high >= 'A' && high <= 'Z'){
		high += 32;
	}

	if (low >= 'A' && low <= 'Z'){
		low += 32;
	}

	digit = high>='a'?((high)-'a') + 10 :(high-'0');
	digit = digit << 4;
	digit += low>='a'?((low)-'a') + 10 :(low-'0');
	return digit;
}


static int stringToHex(unsigned char *string,unsigned char *hexCode)
{
	int i;
	unsigned char *strPtr;
	unsigned char *hexPtr;

	if (!string||!hexCode){
		return -1;
	}

	strPtr = string;
	hexPtr = hexCode;

	for (i = 0; i < 6; i++){
		*hexPtr = x2c((unsigned char *)strPtr);
		strPtr += 2;
		hexPtr ++;
	}

	return 0;

}

int macaddr_sanity_check(char * mac_str)
{
	int i, len;
	int flag_zero, flag_f;
	char *p;

	len = strlen(mac_str);

	if (len != 12){
		return -1;
	}

	p = mac_str;
	flag_zero = flag_f = 0;

	for (i = 0; i < 12; i++){
		if(*p == '0'){
			flag_zero += 1;
		} else if (*p == 'f' ||*p == 'F' ){
			flag_f +=1;
		}
		p++;
	}

	if (flag_zero == 12 || flag_f == 12){
		return -1;
	}else{
		return 0;
	}


}

DECLARE_GLOBAL_DATA_PTR;
