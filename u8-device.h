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

struct work_ent {
	struct work *work;
	struct list_head head;
};

struct work_queue {
	int num_elems;
	struct list_head head;
};


#endif
