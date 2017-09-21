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
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <errno.h>

#if 0
#define PX4_I2C_OBDEV_LED	0x55
#define SUB_ADDR_PWM0		0x81	/**< blue     (without auto-increment) */
#define SUB_ADDR_PWM1		0x82	/**< green    (without auto-increment) */
#define SUB_ADDR_PWM2		0x83	/**< red      (without auto-increment) */
#define SUB_ADDR_SETTINGS	0x84	/**< settings (without auto-increment)*/
#define SETTING_NOT_POWERSAVE	0x01	/**< power-save mode not off */
#define SETTING_ENABLE   	0x02	/**< on */

int fd = 0;

int _writeReg(uint8_t address, uint8_t *in_buffer, int length)
{

	if (fd == 0)
    {
		printf("error: i2c bus is not yet opened\n");
		return -1;
	}

	uint8_t write_buffer[length + 1];

	if (in_buffer) 
    {
		memcpy(&write_buffer[1], in_buffer, length);
	}

	/* Save the address of the register to read from in the write buffer for the combined write. */
	write_buffer[0] = address;

    int bytes_written = write(fd, (char *) write_buffer, length + 1);

    if (bytes_written != length + 1) 
    {
        printf("Error: i2c write failed. Reported %d bytes written errno=%d\n", bytes_written, errno);

    } 
    else 
    {
        return 0;
    }


	return -1;

}

int main(int argc, char **argv)
{
    uint8_t val = 0xff;
    int ret;
	uint8_t settings_byte = 0;

    settings_byte |= SETTING_ENABLE;
	settings_byte |= SETTING_NOT_POWERSAVE;

    fd = open("/dev/i2c-1", O_RDWR);
    if(fd < 0)
    {
        printf("open iic failed\n");
        return -1;
    }

	ret = ioctl(fd, I2C_SLAVE, PX4_I2C_OBDEV_LED);
    if(ret < 0)
    {
        printf("set iic device address failed %d\n", ret);
        return -1;
    }

    while (1)
    {
        _writeReg(SUB_ADDR_PWM0, &val, sizeof(val));
        _writeReg(SUB_ADDR_SETTINGS, &settings_byte, sizeof(settings_byte));
        usleep(100000); 
    }

    close(fd);

	return 0;
}
#else

#include <inttypes.h>

#define I2C_FLOW_ADDRESS 		0x42	///< 7-bit address. 8-bit address is 0x84, range 0x42 - 0x49
#define PX4FLOW_REG			0x16	///< Measure Register 22

typedef  struct i2c_frame {
	uint16_t frame_count;
	int16_t pixel_flow_x_sum;
	int16_t pixel_flow_y_sum;
	int16_t flow_comp_m_x;
	int16_t flow_comp_m_y;
	int16_t qual;
	int16_t gyro_x_rate;
	int16_t gyro_y_rate;
	int16_t gyro_z_rate;
	uint8_t gyro_range;
	uint8_t sonar_timestamp;
	int16_t ground_distance;
} i2c_frame;

#define I2C_FRAME_SIZE (sizeof(i2c_frame))


typedef struct i2c_integral_frame {
	uint16_t frame_count_since_last_readout;
	int16_t pixel_flow_x_integral;
	int16_t pixel_flow_y_integral;
	int16_t gyro_x_rate_integral;
	int16_t gyro_y_rate_integral;
	int16_t gyro_z_rate_integral;
	uint32_t integration_timespan;
	uint32_t sonar_timestamp;
	uint16_t ground_distance;
	int16_t gyro_temperature;
	uint8_t qual;
} i2c_integral_frame;

#define I2C_INTEGRAL_FRAME_SIZE (sizeof(i2c_integral_frame))


int fd = 0;

int32_t iic_transfer(const uint8_t *send, unsigned send_len, uint8_t *recv, unsigned recv_len)
{
    //send
    if(send != NULL)
    {
        int bytes_written = write(fd, (char *) send, send_len);

        if (bytes_written != send_len) 
        {
            printf("Error: i2c write failed. Reported %d bytes written errno=%d\n", bytes_written, errno);
            return -1;
        } 
    }

    //recv
    if(recv != NULL)
    {
    	int bytes_read = 0;

        bytes_read = read(fd, recv, recv_len);

        if (bytes_read != recv_len)
        {
            printf("error: read register reports a read of %d bytes, but attempted to set %d bytes",
                   bytes_read, recv_len);
            return -1;
        }
    }


	return 0;


}

int measure()
{
	int ret;

	/*
	 * Send the command to begin a measurement.
	 */
	uint8_t cmd = PX4FLOW_REG;
	ret = iic_transfer(&cmd, 1, NULL, 0);

	if (0 != ret) {
		return ret;
	}

	ret = 0;

	return ret;
}

int collect()
{
	int ret = -EIO;

	/* read from the sensor */
	uint8_t val[I2C_FRAME_SIZE + I2C_INTEGRAL_FRAME_SIZE] = { 0 };

	if (PX4FLOW_REG == 0x00) {
		ret = iic_transfer(NULL, 0, &val[0], I2C_FRAME_SIZE + I2C_INTEGRAL_FRAME_SIZE);
	}

	if (PX4FLOW_REG == 0x16) {
		ret = iic_transfer(NULL, 0, &val[0], I2C_INTEGRAL_FRAME_SIZE);
	}

    return ret;

}

int main(int argc, char **argv)
{
    int ret ;
    fd = open("/dev/i2c-0", O_RDWR);
    if(fd < 0)
    {
        printf("open iic failed\n");
        return -1;
    }

	ret = ioctl(fd, I2C_SLAVE, I2C_FLOW_ADDRESS);
    if(ret < 0)
    {
        printf("set iic device address failed %d\n", ret);
        return -1;
    }

    while(1)
    {
        if (0 != measure()) {
            continue;
        }

        /* perform collection */
        if (0 != collect()) {
            continue;
        }


        usleep(100000);
    }

    return 0;
}


#endif
