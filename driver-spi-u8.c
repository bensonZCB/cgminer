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

struct stu_chain *init_u1000_chain(struct spi_ctx *ctx, int chain_id)
{

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

	for (i=0; i<ASIC_CHAIN_NUM; i++)
	{
		if(spi[i] == NULL)
			continue;
		u8_dev_init(spi[i]->config.bus);
		cgsleep_ms(2);
		chain[i] = init_u1000_chain(spi[i], i);

		if (!chain[i])
		{
			applog(LOG_ERR, "init u1000 chain fail");
			Set_Led_OnOf(spi[i]->config.bus, LED_DISABLE);
			continue;
		}

		Set_Led_OnOf(spi[i]->config.bus, LED_ENABLE);
		struct cgpu_info *cgpu = malloc(sizeof(*cgpu));

		memset(cgpu, 0, sizeof(*cgpu));
		cgpu->drv = &u8_drv;
		cgpu->name = "u1000.SingleChain";
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

static int64_t u8_scanwork(struct thr_info *thr)
{
	return 0;
}

static bool u8_queue_full(struct cgpu_info *cgpu)
{
	return true;
}

static void u8_flush_work(struct cgpu_info *cgpu)
{

}

struct device_drv u8_drv = {	
	.drv_id = DRIVER_u8,
	.dname = "u8",
	.name = "u8",
	.drv_detect = u8_detect_chain,
	
	.hash_work = hash_queued_work, 
	.scanwork = u8_scanwork,
	.queue_full = u8_queue_full,
	//.flush_work = u8_flush_work,
	.update_work = u8_flush_work,
};
