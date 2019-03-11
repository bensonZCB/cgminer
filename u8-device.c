#include "miner.h"

void u8_detect_chain(bool hotplug)
{

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
