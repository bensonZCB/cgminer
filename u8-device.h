#ifndef  __U8_DEVICE__H__
#define  __U8_DEVICE__H__
#include <libgen.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/rtc.h>
#include <time.h>
#include "config.h"
#include "spi-context.h"
#include "miner.h"
#include "elist.h"

#define spiFreq	0x01
#define CMD_AUTO_ADDR		0xa5
#define CMD_WRITE_JOB		0x33
#define CMD_READ_RESULT	0x44
#define CMD_READ_REG		0x6b
#define CMD_WRITE_REG		0x86
#define ADDR_BROADCAST		0x00

#define ASIC_RESULT_LEN		8
#define READ_RESULT_LEN		(ASIC_RESULT_LEN+2)

#define ASIC_CHAIN_NUM		2
#define ASIC_CHIP_NUM		2

/*2+82+2(crc)*/
#define JOB_LENGTH		86
#define MAX_CMD_LENGTH		(JOB_LENGTH + ASIC_CHIP_NUM*2*2)
#define spiDiv 0x01

struct work_ent {
	struct work *work;
	struct list_head head;
};

struct work_queue {
	int num_elems;
	struct list_head head;
};

struct u8_chip 
{
	struct work *work[2];
	int hw_errors;
	int stales;
	int chip_nonces_found;
	int chip_accept;
	int chip_reject;
};

struct u8_chain 
{
	int fd;
	int hw_errors;
	char *devname;
	uint32_t  set_job_cnt;
	int stales;
	int nonces_found;
	int nonce_ranges_done;
	int chain_id;
	int num_chips;
	int num_cores;
	int num_active_chips;
	bool need_flush_job;
	bool need_reset_board;
	
	uint32_t current_HTarget6;
 
	pthread_mutex_t lock;
	
	struct u8_chip *chips;
	struct spi_ctx *spi_ctx;
	struct cgpu_info *cgpu;
	struct work_queue active_wq;
	struct timeval last_set_work_t;
	struct timeval last_get_nonce_t;
};

#define I2C0_BUS		"/dev/i2c-0"
#define I2C1_BUS		"/dev/i2c-1"
#define I2C2_BUS		"/dev/i2c-2"
#define I2C3_BUS		"/dev/i2c-3"
#define SYSFS_GPIO_EXPORT	"/sys/class/gpio/export"
#define SYSFS_GPIO_DIR_STR	"/sys/class/gpio/gpio%d/direction"
#define SYSFS_GPIO_VAL_STR	"/sys/class/gpio/gpio%d/value"

#define SYSFS_SPI_EXPORT "/sys/devices/soc0/amba/f8007000.devcfg/fclk_export"
#define SYSFS_SPI_VAL_STR "/sys/devices/soc0/amba/f8007000.devcfg/fclk/fclk1/set_rate"

#define SYSFS_GPIO_DIR_OUT	"out"
#define SYSFS_GPIO_DIR_IN	"in"

/*ADD SEYMOUR LED POWER*/
#define LED_DISABLE "0"
#define LED_ENABLE "1"

/*

lnhere 风扇
Duty_cycle        R.P.M
100            6300(+/-)10%

period(300hz-60khz)              duty_cycle  是period的10%-100%之间，不能超出这个范围，不然会出现无效的参数，无法设置；
1000000（1kHz）                    100000
500000（2kHz）
250000（4khz）
.
.
40000(25khz)
*/

#define SYSFS_PWM_EXPORT "/sys/class/pwm/pwmchip%d/export" // hw pwm5 , fan2, J1103
#define SYSFS_PWM_PWM0 "/sys/class/pwm/pwmchip%d/pwm0"
#define SYSFS_PWM_ENABLE "/sys/class/pwm/pwmchip%d/pwm0/enable" 
#define SYSFS_PWM_PERIOD "/sys/class/pwm/pwmchip%d/pwm0/period" 
#define SYSFS_PWM_DUTY_CYCLE "/sys/class/pwm/pwmchip%d/pwm0/duty_cycle"

#define SYSFS_AD_IN1_GPIO 11  //FAN2 GPIO1_11
#define SYSFS_AD_IN2_GPIO 12  //FAN1 GPIO1_12
#define ENABLE "1"
#define DISABLE "0"

#define PWM "0"
#define FAN_PWM_FREQ_TARGET 500000 //(2Khz) //40000 // (25Khz)
#define FAN_PWM_FREQ 1000000 //1Khz
#define fanid 1


/*
led 

*/
#define SYSFS_LED_BT "/sys/class/leds/user%d/brightness"
#define SYSFS_LED_TR "/sys/class/leds/user%d/trigger"
	
#define SYSFS_LED_TIMER "timer"
#define SYSFS_LED_HEARTBEAT "heartbeat"


#define SYSFS_GPIO_VAL_LOW	"0"
#define SYSFS_GPIO_VAL_HIGH	"1"

extern int SPI_PIN_POWER_EN[4];
extern int SPI_PIN_RESET[4];

extern void swap_data(uint8_t *data, int len);
extern bool spi_poll_result(struct spi_ctx *ctx, uint8_t cmd, uint8_t chip_id, uint8_t *buff, int len);
extern bool cmd_auto_address(struct spi_ctx *ctx, uint8_t chip_id);
extern bool cmd_write_register_1(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pll0, uint32_t pll1,uint32_t pll2,uint32_t pll3);
extern bool cmd_write_register_2(struct spi_ctx *ctx, uint8_t chip_id, uint8_t spdEn, uint8_t spdVid, uint8_t glbSpd);
extern bool cmd_write_register_3(struct spi_ctx *ctx, uint8_t chip_id, uint8_t crcEn, uint8_t RollTimeEn);
extern bool cmd_write_register_4(struct spi_ctx *ctx, uint8_t chip_id, uint32_t nonceTarget);
extern bool cmd_write_register_5(struct spi_ctx *ctx, uint8_t chip_id, uint8_t spi_div);
extern bool cmd_write_register_6(struct spi_ctx *ctx, uint8_t chip_id, uint8_t restProtect, uint8_t cfgRest,
						uint8_t spdGo,uint8_t sycNum, uint32_t spdSetupT);
extern bool cmd_write_register_7(struct spi_ctx *ctx, uint8_t chip_id);
extern bool cmd_write_register_8(struct spi_ctx *ctx, uint8_t chip_id);
#endif
