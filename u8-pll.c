#include "u8-device.h"
#include "spi-context.h"

bool cmd_set_pll3(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pll)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;

	uint8_t RefDiv = 4; //oscilator=20M -> RefDiv=4; oscilator=25M -> RefDiv=5;	
	uint32_t FbDiv = (2*pll)/5;

	uint8_t regpll[4]={0x02, 0x08, 0x22, 0x1f};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0a;

	/************step1 set pll bit************/
	//pll3
	uint8_t Fb_h8bit = (FbDiv>>4)&0xff;
	uint8_t Fb_l4bit = FbDiv &0x0f;
	regpll[1] = Fb_h8bit;
	regpll[2] = (regpll[2]&0x0f)|(Fb_l4bit<<4);
	regpll[0] = (regpll[0]&0xc0)|(RefDiv&0x3f);

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll bit:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	flush_spi(ctx);
	
	/***********step 2 set power down***********/
	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0a;
	
	regpll[3] &= 0xf5;

	memcpy(spi_tx+4, regpll, 4);
	
	applog(LOG_ERR, " set power down:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	flush_spi(ctx);
	
	/***********step 3 set pll gate bit*************/
	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0a;
	
	regpll[0] |= 0x80;

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll gate:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	
	if (spi_tx[0] != 0x00)
	{
		spi_poll_result(ctx, spi_tx[1], chip_id, spi_rx, 2);
		applog(LOG_ERR, "chip_%d, bank2 return:%02x %02x %02x %02x",
					chip_id, spi_rx[0],spi_rx[1],spi_rx[2],spi_rx[3] );
	}
	flush_spi(ctx);
}

bool cmd_set_pll2(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pll)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;

	uint8_t RefDiv = 4; //oscilator=20M -> RefDiv=4; oscilator=25M -> RefDiv=5;	
	uint32_t FbDiv = (2*pll)/5;

	uint8_t regpll[4]={0x02, 0x08, 0x22, 0x1f};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0b;

	/************step1 set pll bit************/
	uint8_t Fb_h8bit = (FbDiv>>4)&0xff;
	uint8_t Fb_l4bit = FbDiv &0x0f;
	regpll[1] = Fb_h8bit;
	regpll[2] = (regpll[2]&0x0f)|(Fb_l4bit<<4);
	regpll[0] = (regpll[0]&0xc0)|(RefDiv&0x3f);

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll bit:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	flush_spi(ctx);
	
	/***********step 2 set power down***********/
	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0b;
	
	regpll[3] &= 0xf5;

	memcpy(spi_tx+4, regpll, 4);
	
	applog(LOG_ERR, " set power down:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	flush_spi(ctx);
	
	/***********step 3 set pll gate bit*************/
	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0b;
	
	regpll[0] |= 0x80;

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll gate:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	
	if (spi_tx[0] != 0x00)
	{
		spi_poll_result(ctx, spi_tx[1], chip_id, spi_rx, 2);
		applog(LOG_ERR, "chip_%d, bank2 return:%02x %02x %02x %02x",
					chip_id, spi_rx[0],spi_rx[1],spi_rx[2],spi_rx[3] );
	}
	flush_spi(ctx);
}

bool cmd_set_pll1(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pll)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;

	uint8_t RefDiv = 4; //oscilator=20M -> RefDiv=4; oscilator=25M -> RefDiv=5;	
	uint32_t FbDiv = (2*pll)/5;

	uint8_t regpll[4]={0x02, 0x08, 0x22, 0x1f};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0c;

	/************step1 set pll bit************/
	uint8_t Fb_h8bit = (FbDiv>>4)&0xff;
	uint8_t Fb_l4bit = FbDiv &0x0f;
	regpll[1] = Fb_h8bit;
	regpll[2] = (regpll[2]&0x0f)|(Fb_l4bit<<4);
	regpll[0] = (regpll[0]&0xc0)|(RefDiv&0x3f);

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll bit:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	flush_spi(ctx);
	
	/***********step 2 set power down***********/
	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0c;
	
	regpll[3] &= 0xf5;

	memcpy(spi_tx+4, regpll, 4);
	
	applog(LOG_ERR, " set power down:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	flush_spi(ctx);
	
	/***********step 3 set pll gate bit*************/
	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0c;
	
	regpll[0] |= 0x80;

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll gate:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	
	if (spi_tx[0] != 0x00)
	{
		spi_poll_result(ctx, spi_tx[1], chip_id, spi_rx, 2);
		applog(LOG_ERR, "chip_%d, bank2 return:%02x %02x %02x %02x",
					chip_id, spi_rx[0],spi_rx[1],spi_rx[2],spi_rx[3] );
	}
	flush_spi(ctx);
}

bool cmd_set_pll0(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pll)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;

	uint8_t RefDiv = 4; //oscilator=20M -> RefDiv=4; oscilator=25M -> RefDiv=5;	
	uint32_t FbDiv = (2*pll)/5;

	uint8_t regpll[4]={0x02, 0x08, 0x22, 0x1f};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0d;

	/************step1 set pll bit************/
	uint8_t Fb_h8bit = (FbDiv>>4)&0xff;
	uint8_t Fb_l4bit = FbDiv &0x0f;
	regpll[1] = Fb_h8bit;
	regpll[2] = (regpll[2]&0x0f)|(Fb_l4bit<<4);
	regpll[0] = (regpll[0]&0xc0)|(RefDiv&0x3f);

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll bit:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	flush_spi(ctx);
	
	/***********step 2 set power down***********/
	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0d;
	
	regpll[3] &= 0xf5;

	memcpy(spi_tx+4, regpll, 4);
	
	applog(LOG_ERR, " set power down:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	flush_spi(ctx);
	
	/***********step 3 set pll gate bit*************/
	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0d;
	
	regpll[0] |= 0x80;

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll gate:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	
	if (spi_tx[0] != 0x00)
	{
		spi_poll_result(ctx, spi_tx[1], chip_id, spi_rx, 2);
		applog(LOG_ERR, "chip_%d, bank2 return:%02x %02x %02x %02x",
					chip_id, spi_rx[0],spi_rx[1],spi_rx[2],spi_rx[3] );
	}
	flush_spi(ctx);
}

bool cmd_update_pll3(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pll)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;

	uint8_t RefDiv = 4; //oscilator=20M -> RefDiv=4; oscilator=25M -> RefDiv=5;	
	uint32_t FbDiv = (2*pll)/5;

	uint8_t regpll[4]={0x02, 0x08, 0x22, 0x1f};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0a;

	/************step1 set pll bit************/
	uint8_t Fb_h8bit = (FbDiv>>4)&0xff;
	uint8_t Fb_l4bit = FbDiv &0x0f;
	regpll[1] = Fb_h8bit;
	regpll[2] = (regpll[2]&0x0f)|(Fb_l4bit<<4);
	regpll[0] = (regpll[0]&0xc0)|(RefDiv&0x3f);
	
	/***********step 2 set power down***********/
	
	regpll[3] &= 0xf5;

	/***********step 3 set pll gate bit*************/	
	regpll[0] |= 0x80;

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll gate:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	
	if (spi_tx[0] != 0x00)
	{
		spi_poll_result(ctx, spi_tx[1], chip_id, spi_rx, 2);
		applog(LOG_ERR, "chip_%d, bank2 return:%02x %02x %02x %02x",
					chip_id, spi_rx[0],spi_rx[1],spi_rx[2],spi_rx[3] );
	}
	flush_spi(ctx);
}

bool cmd_update_pll2(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pll)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;

	uint8_t RefDiv = 4; //oscilator=20M -> RefDiv=4; oscilator=25M -> RefDiv=5;	
	uint32_t FbDiv = (2*pll)/5;

	uint8_t regpll[4]={0x02, 0x08, 0x22, 0x1f};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0b;

	/************step1 set pll bit************/
	uint8_t Fb_h8bit = (FbDiv>>4)&0xff;
	uint8_t Fb_l4bit = FbDiv &0x0f;
	regpll[1] = Fb_h8bit;
	regpll[2] = (regpll[2]&0x0f)|(Fb_l4bit<<4);
	regpll[0] = (regpll[0]&0xc0)|(RefDiv&0x3f);
	
	/***********step 2 set power down***********/
	
	regpll[3] &= 0xf5;

	/***********step 3 set pll gate bit*************/	
	regpll[0] |= 0x80;

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll gate:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	
	if (spi_tx[0] != 0x00)
	{
		spi_poll_result(ctx, spi_tx[1], chip_id, spi_rx, 2);
		applog(LOG_ERR, "chip_%d, bank2 return:%02x %02x %02x %02x",
					chip_id, spi_rx[0],spi_rx[1],spi_rx[2],spi_rx[3] );
	}
	flush_spi(ctx);
}

bool cmd_update_pll1(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pll)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;

	uint8_t RefDiv = 4; //oscilator=20M -> RefDiv=4; oscilator=25M -> RefDiv=5;	
	uint32_t FbDiv = (2*pll)/5;

	uint8_t regpll[4]={0x02, 0x08, 0x22, 0x1f};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0c;

	/************step1 set pll bit************/
	uint8_t Fb_h8bit = (FbDiv>>4)&0xff;
	uint8_t Fb_l4bit = FbDiv &0x0f;
	regpll[1] = Fb_h8bit;
	regpll[2] = (regpll[2]&0x0f)|(Fb_l4bit<<4);
	regpll[0] = (regpll[0]&0xc0)|(RefDiv&0x3f);
	
	/***********step 2 set power down***********/
	
	regpll[3] &= 0xf5;

	/***********step 3 set pll gate bit*************/	
	regpll[0] |= 0x80;

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll gate:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	
	if (spi_tx[0] != 0x00)
	{
		spi_poll_result(ctx, spi_tx[1], chip_id, spi_rx, 2);
		applog(LOG_ERR, "chip_%d, bank2 return:%02x %02x %02x %02x",
					chip_id, spi_rx[0],spi_rx[1],spi_rx[2],spi_rx[3] );
	}
	flush_spi(ctx);
}

bool cmd_update_pll0(struct spi_ctx *ctx, uint8_t chip_id, uint32_t pll)
{
	uint8_t spi_tx[MAX_CMD_LENGTH]={0};
	uint8_t spi_rx[MAX_CMD_LENGTH]={0};
	uint16_t crc;

	uint8_t RefDiv = 4; //oscilator=20M -> RefDiv=4; oscilator=25M -> RefDiv=5;	
	uint32_t FbDiv = (2*pll)/5;

	uint8_t regpll[4]={0x02, 0x08, 0x22, 0x1f};

	spi_tx[0] = CMD_WRITE_REG;
	spi_tx[1] = chip_id;
	spi_tx[2] = 0x00;
	spi_tx[3] = 0x0d;

	/************step1 set pll bit************/
	uint8_t Fb_h8bit = (FbDiv>>4)&0xff;
	uint8_t Fb_l4bit = FbDiv &0x0f;
	regpll[1] = Fb_h8bit;
	regpll[2] = (regpll[2]&0x0f)|(Fb_l4bit<<4);
	regpll[0] = (regpll[0]&0xc0)|(RefDiv&0x3f);
	
	/***********step 2 set power down***********/
	
	regpll[3] &= 0xf5;

	/***********step 3 set pll gate bit*************/	
	regpll[0] |= 0x80;

	memcpy(spi_tx+4, regpll, 4);

	applog(LOG_ERR, " set pll gate:");
	for (int i=0; i<8; i++)
		printf("%02x ", spi_tx[i]);
	printf("\n");

	swap_data(spi_tx, 8);
	crc = CRC16(spi_tx, 8);

	spi_tx[8] = (uint8_t)((crc >> 0) & 0xff);
	spi_tx[9] = (uint8_t)((crc >> 8) & 0xff);

	spi_send_data(ctx, spi_tx, 10);
	
	if (spi_tx[0] != 0x00)
	{
		spi_poll_result(ctx, spi_tx[1], chip_id, spi_rx, 2);
		applog(LOG_ERR, "chip_%d, bank2 return:%02x %02x %02x %02x",
					chip_id, spi_rx[0],spi_rx[1],spi_rx[2],spi_rx[3] );
	}
	flush_spi(ctx);
}