/* Put this declaration somewhere in file */
#include "mbed.h"

DigitalInOut sda(P0_10);
DigitalInOut scl(P0_8);

#define I2C_QUARTERPERIOD_DELAY() do {wait_us(5); } while(0)
#define softi2c_sda_down() do {sda.output(); sda = 0; } while (0)
#define softi2c_sda_up() do {sda.input(); } while (0)
#define softi2c_scl_down() do {scl.output(); scl = 0; } while (0)
#define softi2c_scl_up() do {scl.input(); } while (0)
#define softi2c_get_sda() (sda.read())

// Assert START condition on I2C bus
void softi2c_start()
{
        softi2c_sda_up();
        I2C_QUARTERPERIOD_DELAY();
        softi2c_scl_up();
        I2C_QUARTERPERIOD_DELAY();
        softi2c_sda_down();
        I2C_QUARTERPERIOD_DELAY();
}

// Assert STOP condition on I2C bus
void softi2c_stop()
{
        softi2c_sda_down();
        I2C_QUARTERPERIOD_DELAY();
        softi2c_scl_up();
        I2C_QUARTERPERIOD_DELAY();
        softi2c_sda_up();
        I2C_QUARTERPERIOD_DELAY();
}

// Provide a clock pulse for the ACK bit, and return its value
uint8_t softi2c_check_ack()
{
        uint8_t ack = 1;
        I2C_QUARTERPERIOD_DELAY();
        softi2c_scl_up();
        if (softi2c_get_sda())
                ack = 0;
        I2C_QUARTERPERIOD_DELAY();
        softi2c_scl_down();
        I2C_QUARTERPERIOD_DELAY();
        return ack;
}

// Write 1 byte, and return the slave ACK bit.
uint8_t softi2c_put(uint8_t byte)
{
        uint8_t count = 8;
        while (count--)
        {
                softi2c_scl_down();
                I2C_QUARTERPERIOD_DELAY();
                if (byte & 0x80)
                        softi2c_sda_up();
                else
                        softi2c_sda_down();
                I2C_QUARTERPERIOD_DELAY();
                softi2c_scl_up();
                I2C_QUARTERPERIOD_DELAY();
                byte <<= 1;
        }
        softi2c_scl_down();
        I2C_QUARTERPERIOD_DELAY();
        // Release SDA to allow ACK
        softi2c_sda_up();

        return softi2c_check_ack();
}

// Read 1 byte, and return the slave ACK bit.
uint8_t softi2c_get(uint8_t *ack_ptr = NULL)
{
        uint8_t count = 8;
        uint8_t byte = 0;
        while (count--)
        {
                softi2c_scl_down();
                softi2c_sda_up();
                I2C_QUARTERPERIOD_DELAY();
                softi2c_scl_up();
                byte <<= 1;
                if (softi2c_get_sda())
                        byte |= 1;
                I2C_QUARTERPERIOD_DELAY();
        }
        uint8_t ack = softi2c_check_ack();
        if (ack_ptr)
                *ack_ptr = ack;
        return byte;
}

// Start writing to the address addr (MSB aligned, i.e. addr & 0x1 == 0)
uint8_t softi2c_startrw(uint8_t addr, uint8_t r)
{
        softi2c_start();
        softi2c_put(addr | !!r);
        return softi2c_check_ack();
}
