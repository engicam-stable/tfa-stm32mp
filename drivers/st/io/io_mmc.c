/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <io_driver.h>
#include <io_mmc.h>
#include <io_storage.h>
#include <mmc.h>
#include <stm32_sdmmc2.h>
#include <string.h>

/* SDMMC device functions */
static int mmc_dev_open(const uintptr_t init_params, io_dev_info_t **dev_info);
static int mmc_block_open(io_dev_info_t *dev_info, const uintptr_t spec,
			  io_entity_t *entity);
static int mmc_dev_init(io_dev_info_t *dev_info, const uintptr_t init_params);
static int mmc_block_seek(io_entity_t *entity, int mode,
			  signed long long offset);
static int mmc_block_read(io_entity_t *entity, uintptr_t buffer, size_t length,
			  size_t *length_read);
static int mmc_block_close(io_entity_t *entity);
static int mmc_dev_close(io_dev_info_t *dev_info);
static io_type_t device_type_mmc(void);

static int64_t seek_offset;

static const io_dev_connector_t mmc_dev_connector = {
	.dev_open = mmc_dev_open
};

static const io_dev_funcs_t mmc_dev_funcs = {
	.type = device_type_mmc,
	.open = mmc_block_open,
	.seek = mmc_block_seek,
	.size = NULL,
	.read = mmc_block_read,
	.write = NULL,
	.close = mmc_block_close,
	.dev_init = mmc_dev_init,
	.dev_close = mmc_dev_close,
};

static const io_dev_info_t mmc_dev_info = {
	.funcs = &mmc_dev_funcs,
	.info = 0,
};

/* Identify the device type as mmc device */
static io_type_t device_type_mmc(void)
{
	return IO_TYPE_MMC;
}

/* Open a connection to the mmc device */
static int mmc_dev_open(const uintptr_t init_params, io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);
	*dev_info = (io_dev_info_t *)&mmc_dev_info;

	return 0;
}

static int mmc_dev_init(io_dev_info_t *dev_info, const uintptr_t init_params)
{
	return 0;
}

/* Close a connection to the mmc device */
static int mmc_dev_close(io_dev_info_t *dev_info)
{
	return 0;
}

/* Open a file on the mmc device */
static int mmc_block_open(io_dev_info_t *dev_info, const  uintptr_t spec,
			  io_entity_t *entity)
{
	seek_offset = 0;
	return 0;
}

/* Seek to a particular file offset on the mmc device */
static int mmc_block_seek(io_entity_t *entity, int mode,
			  signed long long offset)
{
	seek_offset = offset;
	return 0;
}

/* Read data from a file on the mmc device */
static int mmc_block_read(io_entity_t *entity, uintptr_t buffer,
			  size_t length, size_t *length_read)
{
	uint8_t retries = 3U;

	do {
		retries--;
		if (retries == 0U) {
			return -EIO;
		}

		*length_read = mmc_read_blocks(seek_offset / MMC_BLOCK_SIZE,
					       buffer, length);

	} while (*length_read != length);

	return 0;
}

/* Close a file on the mmc device */
static int mmc_block_close(io_entity_t *entity)
{
	return 0;
}

/* Register the mmc driver with the IO abstraction */
int register_io_dev_mmc(const io_dev_connector_t **dev_con)
{
	int result;

	assert(dev_con != NULL);

	result = io_register_device(&mmc_dev_info);
	if (result == 0) {
		*dev_con = &mmc_dev_connector;
	}

	return result;
}