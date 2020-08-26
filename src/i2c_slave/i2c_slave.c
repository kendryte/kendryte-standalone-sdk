#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "fpioa.h"
#include "gpiohs.h"
#include "i2c.h"
#include "utils.h"
#include "sysctl.h"
#include "i2c_slave.h"

#define DELAY_TIME  10

slave_info_t slave_device;
i2c_slave_handler_t slave_handler;

void i2c_slave_receive(uint32_t data)
{
    if (slave_device.acces_reg == 0xFF)
    {
        if (data < SLAVE_MAX_ADDR)
            slave_device.acces_reg = data;
    } 
    else if (slave_device.acces_reg < SLAVE_MAX_ADDR)
    {
        slave_device.reg_data[slave_device.acces_reg] = data;
        slave_device.acces_reg++;
    }
}
uint32_t i2c_slave_transmit()
{
    uint32_t ret = 0;
    if (slave_device.acces_reg >= SLAVE_MAX_ADDR)
        slave_device.acces_reg = 0;
    ret = slave_device.reg_data[slave_device.acces_reg];
    slave_device.acces_reg++;
    return ret;
}
void i2c_slave_event(i2c_event_t event)
{
    if(I2C_EV_START == event)
    {
        if (slave_device.acces_reg >= SLAVE_MAX_ADDR)
            slave_device.acces_reg = 0xFF;
    }
    else if(I2C_EV_STOP == event)
    {
        slave_device.acces_reg = 0xFF;
    }
}

void i2c_slave_init(void)
{
    slave_handler.on_event = i2c_slave_event,
    slave_handler.on_receive = i2c_slave_receive,
    slave_handler.on_transmit = i2c_slave_transmit,

    i2c_init_as_slave(0, SLAVE_ADDRESS, 7, &slave_handler);
    slave_device.acces_reg = 0xFF;
}

void i2c_master_init(void)
{
    gpiohs_set_drive_mode(6, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(7, GPIO_DM_OUTPUT);
    gpiohs_set_pin(6, GPIO_PV_HIGH);
    gpiohs_set_pin(7, GPIO_PV_HIGH);
}

void i2c_start(void)
{
    gpiohs_set_drive_mode(7, GPIO_DM_OUTPUT);
    gpiohs_set_pin(7, GPIO_PV_HIGH);
    gpiohs_set_pin(6, GPIO_PV_HIGH);
    usleep(DELAY_TIME);
    gpiohs_set_pin(7, GPIO_PV_LOW);
    usleep(DELAY_TIME);
    gpiohs_set_pin(6, GPIO_PV_LOW);
}

void i2c_stop(void)
{
    gpiohs_set_drive_mode(7, GPIO_DM_OUTPUT);
    gpiohs_set_pin(7, GPIO_PV_LOW);
    gpiohs_set_pin(6, GPIO_PV_HIGH);
    usleep(DELAY_TIME);
    gpiohs_set_pin(7, GPIO_PV_HIGH);
}

uint8_t i2c_send_byte(uint8_t data)
{
    uint8_t index;

    gpiohs_set_drive_mode(7, GPIO_DM_OUTPUT);
    for (index = 0; index < 8; index++)
    {
        if (data & 0x80)
            gpiohs_set_pin(7, GPIO_PV_HIGH);
        else
            gpiohs_set_pin(7, GPIO_PV_LOW);
        usleep(DELAY_TIME);
        gpiohs_set_pin(6, GPIO_PV_HIGH);
        usleep(DELAY_TIME);
        gpiohs_set_pin(6, GPIO_PV_LOW);
        data <<= 1;
    }
    gpiohs_set_drive_mode(7, GPIO_DM_INPUT);
    usleep(DELAY_TIME);
    gpiohs_set_pin(6, GPIO_PV_HIGH);
    data = gpiohs_get_pin(7);
    usleep(DELAY_TIME);
    gpiohs_set_pin(6, GPIO_PV_LOW);

    return data;
}

uint8_t i2c_receive_byte(uint8_t ack)
{
    uint8_t index, data = 0;

    gpiohs_set_drive_mode(7, GPIO_DM_INPUT);
    for (index = 0; index < 8; index++)
    {
        usleep(DELAY_TIME);
        gpiohs_set_pin(6, GPIO_PV_HIGH);
        data <<= 1;
        if (gpiohs_get_pin(7))
            data++;
        usleep(DELAY_TIME);
        gpiohs_set_pin(6, GPIO_PV_LOW);
    }
    gpiohs_set_drive_mode(7, GPIO_DM_OUTPUT);
    if (ack)
        gpiohs_set_pin(7, GPIO_PV_HIGH);
    else
        gpiohs_set_pin(7, GPIO_PV_LOW);
    usleep(DELAY_TIME);
    gpiohs_set_pin(6, GPIO_PV_HIGH);
    usleep(DELAY_TIME);
    gpiohs_set_pin(6, GPIO_PV_LOW);

    return data;
}

uint8_t i2c_write_reg(uint8_t reg, uint8_t *data_buf, size_t length)
{
    i2c_start();
    i2c_send_byte(SLAVE_ADDRESS << 1);
    i2c_send_byte(reg);
    while (length--)
        i2c_send_byte(*data_buf++);
    i2c_stop();

    return 0;
}

uint8_t i2c_read_reg(uint8_t reg, uint8_t *data_buf, size_t length)
{
    i2c_start();
    i2c_send_byte(SLAVE_ADDRESS << 1);
    i2c_send_byte(reg);
    i2c_start();
    i2c_send_byte((SLAVE_ADDRESS << 1) + 1);
    while (length > 1L)
    {
        *data_buf++ = i2c_receive_byte(0);
        length--;
    }
    *data_buf++ = i2c_receive_byte(1);
    i2c_stop();

    return 0;
}

