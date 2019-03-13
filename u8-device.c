#include "u8-device.h"
#include "spi-context.h"

 void flush_spi(struct spi_ctx *ctx)
{
	uint8_t buffer[ASIC_CHIP_NUM*4] = {0};
	spi_send_data(ctx, buffer, ASIC_CHIP_NUM*4);
}

unsigned short CRC16 (unsigned char* pchMsg, unsigned short wDataLen)
{
        unsigned short wCRC = 0xFFFF;
        unsigned short i;
        unsigned char chChar;

        unsigned char tmp;
        unsigned short crcAccum = 0xFFFF;

        for (i = 0; i < wDataLen; i++)
        {
                chChar = *pchMsg++;

                tmp = chChar ^ (unsigned char)(crcAccum &0xff);
                tmp ^= (tmp<<4);
                crcAccum = (crcAccum>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);

        }
		
	return crcAccum;
}

void swap_data(uint8_t *data, int len)
{	
	int i;
	uint8_t tmp;
	for (i=0; i<len; i+=2)
	{
		tmp = data[i];
		data[i] = data[i+1];
		data[i+1] = tmp;
	}
}

bool spi_poll_result(struct spi_ctx *ctx, uint8_t cmd, uint8_t chip_id, uint8_t *buff, int len)
{
	int ret1, ret2;
	int tx_len;
	int tmp_len;
	int index,ret;
	uint8_t spi_tx[MAX_CMD_LENGTH];
	uint8_t spi_rx[MAX_CMD_LENGTH];
	
	memset(spi_tx, 0, sizeof(spi_tx));
	memset(spi_rx, 0, sizeof(spi_rx));

	tx_len = ASIC_CHIP_NUM*4+len+4;	

	for(tmp_len = 0; tmp_len < tx_len; tmp_len += 2)
	{
		if(!spi_read_data(ctx, spi_rx, 2))
		{
			applog(LOG_WARNING, "poll result: transfer fail !");
			return false;
		}
		
		if(spi_rx[1] == cmd)
		{
			index = 0;	
			do{
				ret = spi_read_data(ctx, spi_rx + 2 + index, 2);
				if(!ret)
				{
					return false;
				}					
				index = index + 2;
				
			}while(index < len);

			memcpy(buff, spi_rx, len);
			return true;
		}
		cgsleep_us(2);
	}

	return false;
}

bool cmd_auto_address(struct spi_ctx *ctx, uint8_t chip_id)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	
	memset(spi_tx, 0, sizeof(spi_tx));

	spi_tx[0] = chip_id;
	spi_tx[1] = CMD_AUTO_ADDR;
	spi_tx[2] = 0x3a;
	spi_tx[3] = 0x5c;

	applog(LOG_ERR, "send 0x%02x 0x%02x 0x%02x 0x%02x", spi_tx[0], spi_tx[1], spi_tx[2],spi_tx[3]);
	
	if(spi_write_data(ctx, spi_tx, 4))
	{

	}
	else
	{
		applog(LOG_ERR, "send command fail !");
		return false;
	}

	cgsleep_ms(5);

	memset(spi_rx, 0, sizeof(spi_rx));
	if(!spi_poll_result(ctx, CMD_AUTO_ADDR, chip_id, spi_rx, 4))
	{
		return false;
	}

	applog(LOG_ERR, "auto address result: 0x%02x 0x%02x 0x%02x 0x%02x", spi_rx[0], spi_rx[1], spi_rx[2], spi_rx[3]);

	return true;
}

bool cmd_write_register_0(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pllRstT, uint32_t pllChgT)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x00;

	memcpy(spi_tx+4, pllRstT, 4);
	memcpy(spi_tx+8, pllChgT, 4);

	swap_data(spi_tx, 12);
	crc = CRC16(spi_tx, 12);
	
	spi_tx[12] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[13] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 14);
	
}

bool cmd_write_register_1(struct spi_ctx *ctx, uint8_t chip_id, uint32_t *pllx)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;
	uint8_t bankVal[16] = {0x3e, 0x92, 0x81, 0x00, 0x3e, 0x92, 0x81, 0x00,
							0x3e, 0x92, 0x81, 0x00,0x3e, 0x92, 0x81, 0x00};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x01;
	
	if (NULL != pllx)
	{
		applog(LOG_ERR, "pll0~3:%d %d %d %d", pllx[0], pllx[1], pllx[2], pllx[3]);
		//pll0
		bankVal[0] = *((uint8_t*)(pllx+0));
		bankVal[1] = *((uint8_t*)(pllx+0)+1);
		bankVal[2] = *((uint8_t*)(pllx+0)+2);
		bankVal[3] = *((uint8_t*)(pllx+0)+3);
		//pll1
		bankVal[4] = *((uint8_t*)(pllx+1));
		bankVal[5] = *((uint8_t*)(pllx+1)+1);
		bankVal[6] = *((uint8_t*)(pllx+1)+2);
		bankVal[7] = *((uint8_t*)(pllx+1)+3);
		//pll2
		bankVal[8] = *((uint8_t*)(pllx+2));
		bankVal[9] = *((uint8_t*)(pllx+2)+1);
		bankVal[10] = *((uint8_t*)(pllx+2)+2);
		bankVal[11] = *((uint8_t*)(pllx+2)+3);
		//pll3
		bankVal[12] = *((uint8_t*)(pllx+3));
		bankVal[13] = *((uint8_t*)(pllx+3)+1);
		bankVal[14] = *((uint8_t*)(pllx+3)+2);
		bankVal[15] = *((uint8_t*)(pllx+3)+3);
		
	}

	memcpy(spi_tx+4, bankVal, 16);

	swap_data(spi_tx, 20);
	crc = CRC16(spi_tx, 20);

	spi_tx[20] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[21] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 22);
	
}

bool cmd_write_register_2(struct spi_ctx *ctx, uint8_t chip_id)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;
	uint8_t bankVal[16] = {0x00, 0x00, 0x00,0x30,0x00,0x01,0x00,0xa0, 0x80, 0x00, 0x00, 0x7c, 0x00, 0x00,0x00, 0x00};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x02;

	memcpy(spi_tx+4, bankVal, 16);

	swap_data(spi_tx, 20);
	crc = CRC16(spi_tx, 20);

	spi_tx[20] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[21] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 22);
}

bool cmd_write_register_3(struct spi_ctx *ctx, uint8_t chip_id)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;
	uint8_t bankVal[8]={0x10, 0x0f, 0xa0, 0x00, 0x00, 0x00,0x00, 0x64};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x03;

	memcpy(spi_tx+4, bankVal, 8);

	swap_data(spi_tx, 12);
	crc = CRC16(spi_tx, 12);

	spi_tx[12] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[13] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 14);
}

bool cmd_write_register_4(struct spi_ctx *ctx, uint8_t chip_id)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;
	uint8_t bankVal[8]={0x00, 0x00, 0x00, 0x00, 0xc0, 0x00,0x00, 0x00};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x04;

	memcpy(spi_tx+4, bankVal, 8);

	swap_data(spi_tx, 12);
	crc = CRC16(spi_tx, 12);

	spi_tx[12] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[13] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 14);
}


bool cmd_write_register_5(struct spi_ctx *ctx, uint8_t chip_id)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;
	uint8_t bankVal[8]={0x00, 0x01, 0x27, 0xf0, 0x00, 0x00, 0x00, 0x00};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x05;

	memcpy(spi_tx+4, bankVal, 8);

	swap_data(spi_tx, 12);
	crc = CRC16(spi_tx, 12);

	spi_tx[12] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[13] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 14);
}

bool cmd_write_register_6(struct spi_ctx *ctx, uint8_t chip_id)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;
	uint8_t bankVal[8]={0x00, 0x00, 0x0f, 0xff, 0x10, 0x00, 0x00, 0x08};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x06;

	memcpy(spi_tx+4, bankVal, 8);

	swap_data(spi_tx, 12);
	crc = CRC16(spi_tx, 12);

	spi_tx[12] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[13] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 14);
}

bool cmd_write_register_7(struct spi_ctx *ctx, uint8_t chip_id)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;
	uint8_t bankVal[64]={0};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x07;

	memcpy(spi_tx+4, bankVal, 64);

	swap_data(spi_tx, 68);
	crc = CRC16(spi_tx, 68);

	spi_tx[68] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[69] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 70);
}

bool cmd_write_register_8(struct spi_ctx *ctx, uint8_t chip_id)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;
	uint8_t bankVal[64]={0};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x08;

	memcpy(spi_tx+4, bankVal, 64);

	swap_data(spi_tx, 68);
	crc = CRC16(spi_tx, 68);

	spi_tx[68] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[69] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 70);
}

/*
regName  len(bytes, len of valid value)
reg0		8
reg1		16
reg2		16
reg3		8
reg4		8
reg5		8
reg6		8
reg7		64
reg8		64
*/
bool cmd_read_register(struct spi_ctx *ctx, uint8_t chip_id, uint8_t regAddr, uint8_t regDataLen)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};

	spi_tx[0] = chip_id;
	spi_tx[1] = CMD_READ_REG;
	spi_tx[2] = regAddr;
	spi_tx[3] = 0x00;

	printf("read register %d %s chip_%d:\n",regAddr, basename(ctx->devname),chip_id);
	
	if(spi_send_data(ctx, spi_tx, 4))
	{

	}
	else
	{
		applog(LOG_ERR, "cmd_read_register_bank_2 send command fail !");
		return false;
	}

	memset(spi_rx, 0, sizeof(spi_rx));
	if(!spi_poll_result(ctx, CMD_READ_REG, chip_id, spi_rx, regDataLen+4))
	{
		return false;
	}

	#if 1
	uint8_t i;
	for (i=0; i<(regDataLen+6); i++)
	{
		if(i%4 == 0)
			printf("\n");

		printf("0x%02x ", spi_rx[i]);
	}
	printf("\n");
	#endif

	uint16_t crc;
	crc = CRC16(spi_rx+4, regDataLen);

	uint16_t rx_crc;
	rx_crc = spi_rx[(regDataLen+6)-1] << 8 | spi_rx[(regDataLen+6)-2];


	if(crc != rx_crc)
		return false;

	return true;
}

/*****************************************************/

int SPI_PIN_POWER_EN[4] = {
117,
118,
123,
124,
};

int SPI_PIN_RESET[4] = {
1,
26,
85,
19,
};

void asic_gpio_init(int gpio, int direction)
{
	int fd;
	char fvalue[64];
	char fpath[64];

	fd = open(SYSFS_GPIO_EXPORT, O_WRONLY);
	if(fd == -1)
	{
		return;
	}
	memset(fvalue, 0, sizeof(fvalue));
	sprintf(fvalue, "%d", gpio);
	write(fd, fvalue, strlen(fvalue));
	close(fd);

	memset(fpath, 0, sizeof(fpath));
	sprintf(fpath, SYSFS_GPIO_DIR_STR, gpio);	
	fd = open(fpath, O_WRONLY);
	if(fd == -1)
	{
		return;
	}
	if(direction == 0)
	{
		write(fd, SYSFS_GPIO_DIR_OUT, sizeof(SYSFS_GPIO_DIR_OUT));
	}
	else
	{
		write(fd, SYSFS_GPIO_DIR_IN, sizeof(SYSFS_GPIO_DIR_IN));
	}	
	close(fd);
}

void asic_gpio_write(int gpio, int value)
{
	int fd;
	char fvalue[64];
	char fpath[64];

	memset(fpath, 0, sizeof(fpath));
	sprintf(fpath, SYSFS_GPIO_VAL_STR, gpio);
	fd = open(fpath, O_WRONLY);
	if(fd == -1)
	{
		return;
	}

	if(value == 0)
	{
		write(fd, SYSFS_GPIO_VAL_LOW, sizeof(SYSFS_GPIO_VAL_LOW));
	}
	else
	{
		write(fd, SYSFS_GPIO_VAL_HIGH, sizeof(SYSFS_GPIO_VAL_HIGH));
	}	
	close(fd);	
}

int asic_gpio_read(int gpio)
{
	int fd;
	char fvalue[64];
	char fpath[64];

	memset(fpath, 0, sizeof(fpath));
	sprintf(fpath, SYSFS_GPIO_VAL_STR, gpio);
	fd = open(fpath, O_RDONLY);
	if(fd == -1)
	{
		printf("=========%s, %s, %d ========\n ", __FILE__, __func__, __LINE__);
		return -1;
	}
	memset(fvalue, 0, sizeof(fvalue));
	read(fd, fvalue, 1);
	close(fd);
	if(fvalue[0] == '0')
	{
		return 0;
	}
	else
	{
		return 1;
	}	
} 

bool stu_gpio_init(int chain_id)
{
	asic_gpio_init(SPI_PIN_POWER_EN[chain_id], 0); //0 --> ouput
	asic_gpio_init(SPI_PIN_RESET[chain_id], 0);
	printf("~~~~~~~~~~~~~~~~~~~~~~\n");
}

bool stu_board_powerOnOf(int chain_id, uint8_t val)
{
	asic_gpio_write(SPI_PIN_POWER_EN[chain_id], val);
}

bool stu_board_hw_reset(int chain_id)
{
	asic_gpio_write(SPI_PIN_RESET[chain_id], 0);
	cgsleep_ms(200);
	asic_gpio_write(SPI_PIN_RESET[chain_id], 1);
	cgsleep_ms(200);
	asic_gpio_write(SPI_PIN_RESET[chain_id], 0);
}

void stu_board_set_reset_low(int chain_id)
{
	asic_gpio_write(SPI_PIN_RESET[chain_id], 1);
}

int Write_Sys_file(unsigned char num, char *path, unsigned char *value)
{
			int fd;
			char fpath[64];
			int ret, i, len;
	
	
			memset(fpath, 0, sizeof(fpath));
			sprintf(fpath, path, num);
			fd = open(fpath, O_WRONLY);
			if(fd == -1)
			{
				return -1;
			}
			len = strlen(value);

			for(i = 0; i < 5; i++)
			{
				ret = write(fd, value, len);
				if(ret >= 0)
				{
					printf("write sys file %d\n", ret);
					break;
				}
				printf("write sys file fail \n");
			}

			close(fd);
			return ret;
}

bool u8_dev_init(int chain_id)
{
	stu_gpio_init(chain_id);
	cgsleep_ms(50);
	stu_board_powerOnOf(chain_id, 1);
	cgsleep_ms(50);
	stu_board_hw_reset(chain_id);

	return true;
}

void Set_Led_OnOf(unsigned char num, unsigned char *val)
{
	Write_Sys_file(num+1, SYSFS_LED_BT, val);
}

void Set_Led_timer(unsigned char num)
{
	Write_Sys_file(num+1, SYSFS_LED_TR, SYSFS_LED_TIMER);
}