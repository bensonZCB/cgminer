#include "u8-device.h"
#include "miner.h"

struct spi_config cfg[ASIC_CHAIN_NUM];
struct spi_ctx *spi[ASIC_CHAIN_NUM];
struct u8_chain *chain[ASIC_CHAIN_NUM];

extern char g_miner_version[256];
extern char g_miner_compiletime[256];
extern char g_miner_type[256];

static bool wq_enqueue(struct work_queue *wq, struct work *work)
{
	if (work == NULL)
		return false;
	struct work_ent *we = malloc(sizeof(*we));
	assert(we != NULL);

	we->work = work;
	INIT_LIST_HEAD(&we->head);
	list_add_tail(&we->head, &wq->head);
	wq->num_elems++;
	return true;
}

static struct work *wq_dequeue(struct work_queue *wq)
{
	if (wq == NULL)
		return NULL;
	if (wq->num_elems == 0)
		return NULL;
	struct work_ent *we;
	we = list_entry(wq->head.next, struct work_ent, head);
	struct work *work = we->work;

	list_del(&we->head);
	free(we);
	wq->num_elems--;
	return work;
}

struct u8_chain *init_u8_chain(struct spi_ctx *ctx, int chain_id)
{
	uint8_t devname[64]={0};
	char *p;
	int n;
	int loop;
	bool ret = false;
	struct u8_chain *achain =  calloc(1, sizeof(*achain));

	sprintf(devname, SPI_DEVICE_TEMPLATE, ctx->config.bus, ctx->config.cs_line);
	achain->chain_id = ctx->config.bus;
	achain->spi_ctx = ctx;
	achain->devname = strdup(devname);
	achain->spi_ctx->devname = strdup(devname);
	achain->num_chips = ASIC_CHIP_NUM;
	achain->num_active_chips = ASIC_CHIP_NUM;
	achain->set_job_cnt = 0;
	achain->chips = calloc(achain->num_active_chips, sizeof(struct u8_chip));
	achain->current_HTarget6 = 0xffffffff;
	//achain->need_wtlevel = true;
	//achain->read_chipid = 1;

	for(loop = 0; loop < 3; loop++)
	{
		applog(LOG_ERR, "start cmd_auto_address... loop: %d", loop);

		ret = cmd_auto_address(ctx, ADDR_BROADCAST);

		if(true == ret)
		{
			break;
		}
		else
		{
			goto FAIL;
		}
	}

#if 0
	if (!ret)
	{
		if ( !cmd_detect_chip2(ctx))
		{
			applog(LOG_ERR, "%s cmd_auto_address fail ! loop: %d", devname, loop);
			goto FAIL;
		}
	}
	applog(LOG_ERR, "cmd_auto_address finished... loop%d", loop);
#endif	
		
	mutex_init(&achain->lock);
	INIT_LIST_HEAD(&achain->active_wq.head);

	return achain;

FAIL:	
	free(achain->chips);
	free(achain->devname);
	free(achain);
	return NULL;

}
	
void u8_detect_chain(bool hotplug)
{
	int i;

	for (i=0; i<ASIC_CHAIN_NUM; i++)
	{
		cfg[i].bus     = i;
		cfg[i].cs_line = 0;
		cfg[i].mode    = SPI_MODE_0;
		cfg[i].speed   = DEFAULT_SPI_SPEED;
		cfg[i].bits    = 16;
		cfg[i].delay   = DEFAULT_SPI_DELAY_USECS;

		spi[i] = spi_init(&cfg[i]);
		if(spi[i] == NULL)
		{
			applog(LOG_ERR, "spi init fail");
			continue;
		}

		applog(LOG_WARNING, "spi_init, default SPI[%d] speed: %d", i , spi[i]->config.speed);
		
	}

	for (i=1; i<ASIC_CHAIN_NUM; i++)
	{
		if(spi[i] == NULL)
			continue;
		u8_dev_init(spi[i]->config.bus);
		cgsleep_ms(2);
		chain[i] = init_u8_chain(spi[i], i);

		if (!chain[i])
		{
			applog(LOG_ERR, "init u8 chain fail");
			Set_Led_OnOf(spi[i]->config.bus, LED_DISABLE);
			continue;
		}
	printf("----------------------------------------------\n");
		config_hash_board(chain[i]);
	printf("----------------------------------------------\n");


		Set_Led_OnOf(spi[i]->config.bus, LED_ENABLE);
		struct cgpu_info *cgpu = malloc(sizeof(*cgpu));

		memset(cgpu, 0, sizeof(*cgpu));
		cgpu->drv = &u8_drv;
		cgpu->name = "u8.SingleChain";
		cgpu->threads = 1;
		cgpu->device_id =  chain[i]->chain_id;

		cgpu->device_data = chain[i];
		chain[i]->cgpu = cgpu;
		add_cgpu(cgpu);
	}

	sprintf(g_miner_version, "%s", "18.7.22.08");
	sprintf(g_miner_compiletime, "%s", "Wed May 3 19:18:52 CST 2017");
	sprintf(g_miner_type, "%s", "StuMiner U8"); 
}

static uint8_t  * create_job( struct work *work, uint8_t chip_id)
{
	uint16_t crc;
	static uint8_t jobinfo[JOB_LENGTH] = {0};
	uint32_t *pnonce = (uint32_t*)(jobinfo+80);

	#define BOOST_EN   0x01
	#define START_NONCE	0x00
	

 #if 0 //for debug
	//#define START_NONCE	0x032ac98f
	//NONCE = 0x032ac98f
	uint32_t dat[20]={0x00000020, 0xac8a4806, 0x039f47f6, 0xf08c14f1, 0x46bfeba5, 
				       0x1e5331fa, 0x39e30200, 0x00000000, 0x00000000, 0xda8511c3, 
				       0xbe7d0189, 0x1005ef49, 0xa26f0f6d, 0x8aa8b1e8, 0xdc9acac6, 
				       0x88cea418, 0xf3caf1f0, 0x34f3925c, 0x17612e17, 0x00000000};

	uint32_t mid[8] = {0x33fb8f54, 0xac8f0266, 0x9909ba17, 0x48d36be5,
						0x85180446, 0x24d4b2f7, 0x46e91226, 0x0a732c14};

	uint32_t target[8] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
						0xfffff800, 0xc34f3caf, 0x00000000};

	memcpy(work->target, target, 8*4);
	memcpy(work->data, dat, 20*4);
	memcpy(work->midstate, mid, 8*4);
	memcpy(work->midstate1, mid, 8*4);

	//make diff
	work->midstate1[0] = 0x44;

	work->save_ntime = *((uint32_t *)(work->data+68));

		free(work->ntime);
		char str_ntime[12]={0};
		snprintf(str_ntime, 9, "%08x", swab32(work->save_ntime));

		work->ntime = strdup(str_ntime);
#endif
	
	

	jobinfo[0] = CMD_WRITE_JOB;
	jobinfo[1] = chip_id;

	//speed info
	jobinfo[2] = BOOST_EN;
	jobinfo[3] = 0x01;

	uint32_t xmid1[8],xmid[8],wdata[3], *pmid;

	pmid = work->midstate;
	for (int i=0; i<8; i++)
	{
		xmid[i] = swab32(pmid[7-i]);
	}

	pmid = work->midstate1;
	for (int i=0; i<8; i++)
	{
		xmid1[i] = swab32(pmid[7-i]);
	}

	uint32_t *pdata = (uint32_t*)(work->data+64);

	
	wdata[0] = swab32(pdata[2]);
	wdata[1] = swab32(pdata[1]);
	wdata[2] = swab32(pdata[0]);

	//midstate
	memcpy(jobinfo+4, xmid1, 32);
	//midstate1
	memcpy(jobinfo+36, xmid, 32);
	//wdata
	memcpy(jobinfo+68, wdata, 12);

	//init nonce
	//*pnonce = swap
	jobinfo[80] = (START_NONCE>>24)&0xff;
	jobinfo[81] = (START_NONCE>>16)&0xff;
	jobinfo[82] = (START_NONCE>>8)&0xff;
	jobinfo[83] = (START_NONCE>>0)&0xff;

	#if 1
		applog(LOG_ERR, "job data:");
		for (int i=0; i<JOB_LENGTH-2; i++)
		{
			printf("%02x ", jobinfo[i]);
			if (((i+1)%16) == 0)printf("\n");
		}
	#endif
	
	swap_data(jobinfo, JOB_LENGTH);
	crc = CRC16(jobinfo, JOB_LENGTH-2);
	printf("%04x\n", crc);

	jobinfo[JOB_LENGTH-2] = (uint8_t)((crc >> 0) & 0xff);
	jobinfo[JOB_LENGTH-1] = (uint8_t)((crc >> 8) & 0xff);
	

	return jobinfo;
}

uint32_t cal_zerobit(uint32_t diff)
{
	uint32_t zerobit=0;
	uint32_t shiftbit=0x80000000;
	
	for (int i=1; i<32; i++)
	{
		if (diff&shiftbit)
			break;
		else
			zerobit++;

		shiftbit = (shiftbit>>1);
	}

	return zerobit;
}

static inline change_target(struct u8_chain *achain, uint32_t new_diff)
{
	if (achain->current_HTarget6 != new_diff)
	{
		uint32_t setTarget=0xffffffff;
		uint32_t zerobit = cal_zerobit(new_diff);
		
		applog(LOG_ERR, "new diff =%08x, %d bit zero, target6=%08x", new_diff, zerobit, (setTarget>>zerobit));
		achain->current_HTarget6 = new_diff;

		setTarget = setTarget>>zerobit;
		
		cmd_write_register_4(achain->spi_ctx, ADDR_BROADCAST, setTarget);
	}
}

static bool set_work(struct u8_chain *achain,uint8_t chip_id, struct work *work)
{
	bool ret=false;
	struct spi_ctx *ctx = achain->spi_ctx;
	struct u8_chip *chip = &achain->chips[chip_id -1];

	if (1 == chip_id)
	{
		uint32_t diff6 = *((uint32_t *)(work->target+24));
		change_target(achain, diff6);
	}

	if (chip->work[0] != NULL)
	{
		if (chip->work[1] != NULL)
		{
			work_completed(achain->cgpu, chip->work[1]);
		}
		chip->work[1] = chip->work[0];
		chip->work[0] = NULL;
	}
	uint8_t *jobdata = create_job(work, chip_id);
	if (!cmd_write_job(achain, chip_id, jobdata))
	{
		work_completed(achain->cgpu, work);
		chip->work[0] = NULL;
		ret = false;
	}
	else
	{
		chip->work[0] = work;
		ret = true;
	}

	//applog(LOG_ERR, "1--chip_%d, chipAddr=%08x, work0-1:%08x %08x", 
	//			chip_id, chip, chip->work[0], chip->work[1]);

	return ret;
}

static bool get_nonce(struct u8_chain *achain, uint8_t *nonce, uint8_t *chip_id ,uint8_t *midstat_id, uint8_t *ntime)
{
	uint8_t buffer[24]={0};
	if (cmd_read_result(achain, ADDR_BROADCAST,buffer))
	{
		uint16_t rx_crc = CRC16(buffer, 8);
		uint16_t *pcrc = &buffer[8];
		
		if (rx_crc != *pcrc)
		{
			applog(LOG_ERR, "read nonce crc error!");
			return false;
		}
		
		*chip_id = buffer[0];
		*ntime = buffer[2];
		*midstat_id = buffer[3];

		nonce[0] = buffer[6];
		nonce[1] = buffer[7];
		nonce[2] = buffer[4];
		nonce[3] = buffer[5];
		
		applog(LOG_ERR, "Got nonce for chip_%d  mid_id=%d, NONCE=0x%08x, ntime=%02x",
			   *chip_id, *midstat_id, (*(uint32_t *)nonce), *ntime);
		
		return true;
	}
	return false;
}


static int64_t u8_scanwork(struct thr_info *thr)
{
#if 1
	struct cgpu_info *cgpu = thr->cgpu;
	struct u8_chain *achain = cgpu->device_data;
	bool work_updated = false;
	struct timeval t_now;
	uint64_t accept = 0;
	uint32_t nonce;
	char str_ntime[12]={0};
	uint8_t chip_ntime;
	uint32_t new_ntime;
	uint8_t chip_id;
	uint8_t midstat_id;
	
	int i, j;

	mutex_lock(&achain->lock);

	for (i=0; i<2; i++)
	{
		if (!get_nonce(achain, (uint8_t*)&nonce, &chip_id, &midstat_id, &chip_ntime))
			break;

		//applog(LOG_ERR, "%s, %s, %d", __FILE__, __func__, __LINE__);
		work_updated = true;
		if (chip_id<1 || chip_id>achain->num_active_chips)
		{
			//applog(LOG_WARNING, "%s, wrong chip_id %d", basename(achain->devname), chip_id);
			continue;
		}
		struct u8_chip *chip = &achain->chips[chip_id -1];
		struct work *work  = chip->work[0];

		//applog(LOG_ERR, "2--chip_%d, chipAddr=%08x, work0-1:%08x %08x", 
		//		chip_id, chip, chip->work[0], chip->work[1]);
		
		if (work == NULL) 
		{
			applog(LOG_ERR, "a work == NULL");
			/* already been flushed => stale */
			applog(LOG_ERR, "%s, %s, %d", __FILE__, __func__, __LINE__);
			applog(LOG_WARNING, "%s: chip %d: stale nonce 0x%08x\n",
			       basename(achain->devname), chip_id, nonce);
			chip->stales++;
			
			continue;
		}

		uint32_t *pversion = (uint32_t *)work->data;

		uint32_t *pnt = (uint32_t *)(work->data+68);

		#if 0
			uint32_t *pp = work->data;

			applog(LOG_ERR, "data before:");
			for (int i=0; i<20; i++)
			{
				printf("%08x ", pp[i]);
				if ((i+1)%8 == 0)printf("\n");
			}
			printf("\n");
		#endif

		//applog(LOG_ERR, "save ntime = 0x%08x,pntime=%08x, str=%s", 
		//			work->save_ntime, *pnt,  work->ntime);

		#if 1
		free(work->ntime);
		new_ntime = swab32(swab32(work->save_ntime)+chip_ntime);
		snprintf(str_ntime, 9, "%08x", swab32(new_ntime));

		//applog(LOG_ERR, "o_time=%08x, chip_ntime=%02x", swab32(work->save_ntime), chip_ntime);
		//applog(LOG_ERR, "o_time");

		work->ntime = strdup(str_ntime);
		*pnt = new_ntime;
		#endif

		//applog(LOG_ERR, "save ntime = 0x%08x,pntime=%08x, str=%s", 
		//			work->save_ntime, *pnt,  work->ntime);

		#if 1
		if (midstat_id)
			*pversion = work->version1;
		else
			*pversion = work->version;
		#endif

	applog(LOG_ERR, "v1=%08x v2=%08x",work->version, work->version1);
	applog(LOG_ERR, "use version=%08x",*pversion);

		#if 0
			 pp = work->data;
			applog(LOG_ERR, "data after:");
			for (int i=0; i<20; i++)
			{
				printf("%08x ", pp[i]);
				if ((i+1)%8 == 0)printf("\n");
			}
			printf("\n");
		#endif

		accept += (int64_t)work->sdiff;

		if (!submit_nonce(thr, work, nonce)) 
		{
			cgpu->last_nonce = 0;
			chip->hw_errors++;
#if 0
			//recheck
		struct u8_chip *chip = &achain->chips[chip_id -1];
		struct work *work  = chip->work[1];

			if (work == NULL) 
			{
				applog(LOG_ERR, "b work == NULL");
				/* already been flushed => stale */
				applog(LOG_WARNING, "%s: chip %d: stale nonce 0x%08x",
			       			basename(achain->devname), chip_id, nonce);
				chip->stales++;
				continue;
			}

			uint32_t *pversion = (uint32_t *)work->data;

			uint32_t *pnt = (uint32_t *)(work->data+68);

			applog(LOG_ERR, "old ntime = 0x%08x, str=%s", *pnt, work->ntime);

			#if 0
			free(work->ntime);
			snprintf(str_ntime, 9, "%08x", new_ntime);

			work->ntime = strdup(str_ntime);
			*pnt = new_ntime;
			#endif

			applog(LOG_ERR, "new ntime = 0x%08x, str=%s", *pnt, work->ntime);

			#if 0
			if (midstat_id)
				*pversion = work->version1;
			else
				*pversion = work->version;
			#endif

			applog(LOG_ERR, "version=%08x",*pversion);


			if (!submit_nonce(thr, work, nonce)) 
			{
				chip->hw_errors++;
				continue;
			}

			applog(LOG_DEBUG, "YEAH: %s: chip %d / midstat_id %d: nonce 0x%08x",
		       			basename(achain->devname), chip_id, midstat_id, nonce);
			chip->chip_nonces_found++;
			accept += (int64_t)work->sdiff;
#endif			
		}		

	}
	cgtime(&t_now);
	uint32_t t_ms = ms_tdiff(&t_now, &achain->last_set_work_t);

	if (t_ms>30*1000 || achain->need_flush_job)
	{
		#if 0
			for (int k=5; k>=0; k--)
			{
				printf("k:%d\n", k);
				sleep(1);
			}
		#endif

		
		struct work *work;
		work_updated = true;
		cgtime(&achain->last_set_work_t);
		for (i=0; i<achain->num_active_chips; i++)
		{
			work = wq_dequeue(&achain->active_wq);
			
			if (work != NULL) 
			{
		    		achain->need_flush_job = false;

				applog(LOG_ERR, "%s, chip_%d, setwork, t=%ds", basename(achain->devname), i+1, t_ms/1000);
				if (set_work(achain, i+1, work)) 
				{
				}
			}
		}
	}

	#if 0
		applog(LOG_ERR, "stop...");
		sleep(10000);
	#endif
	
	mutex_unlock(&achain->lock);

	return ((int64_t) accept<< 32);
#endif
}

static bool u8_queue_full(struct cgpu_info *cgpu)
{
	struct u8_chain *achain = cgpu->device_data;
	int queue_full = false;

	mutex_lock(&achain->lock);

	if (achain->active_wq.num_elems >=  achain->num_active_chips)
		queue_full = true;
	else
		wq_enqueue(&achain->active_wq, get_queued(cgpu));

	mutex_unlock(&achain->lock);

	return queue_full;
}

static void u8_flush_work(struct cgpu_info *cgpu)
{
	struct u8_chain *achain = cgpu->device_data;

	mutex_lock(&achain->lock);

	while (achain->active_wq.num_elems > 0) 
	{
		struct work *work = wq_dequeue(&achain->active_wq);
		assert(work != NULL);
		work_completed(cgpu, work);
	}

	mutex_unlock(&achain->lock);

	achain->need_flush_job = true;
}

struct device_drv u8_drv = {	
	.drv_id = DRIVER_u8,
	.dname = "u8",
	.name = "u8",
	.drv_detect = u8_detect_chain,
	.hash_work = hash_queued_work, 
	.scanwork = u8_scanwork,
	.queue_full = u8_queue_full,
	.update_work = u8_flush_work,
};
