/*
 * Copyright (c) 2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <byteswap.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <string.h>

#define DIR_READ    0x80
#define DIR_WRITE   0x00

int fd = 0;

int _readReg(uint8_t address, uint8_t *val)
{
	uint8_t write_buffer[2] = {0}; // automatic write buffer
	uint8_t read_buffer[2] = {0}; // automatic read buffer

	write_buffer[0] = address | DIR_READ; // read mode
	write_buffer[1] = 0; // write data

	struct spi_ioc_transfer spi_transfer; // datastructures for linux spi interface
	memset(&spi_transfer, 0, sizeof(struct spi_ioc_transfer));

	spi_transfer.tx_buf = (unsigned long)write_buffer;
	spi_transfer.rx_buf = (unsigned long)read_buffer;
	spi_transfer.len = 2;
	// spi_transfer.speed_hz = SPI_FREQUENCY_1MHZ; // temporarily override spi speed
	spi_transfer.bits_per_word = 8;
	spi_transfer.delay_usecs = 0;

	int result = 0;
	result = ioctl(fd, SPI_IOC_MESSAGE(1), &spi_transfer);

	if (result != 2) {
		printf("error: SPI combined read write failed: %d", result);
		return -1;
	}

	*val = read_buffer[1];

    return 0;
}

int main(int argc, char **argv)
{
    uint8_t val = 0;

    fd = open("/dev/spidev0.0", 0);

    while(1)
    {
        _readReg(0x0f, &val);
        printf("val=%x\n", val);
        usleep(100000); 
    }

    close(fd);

	return 0;
}
