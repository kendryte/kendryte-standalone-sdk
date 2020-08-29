#include "rtc.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "htpa.h"
#include "sipeed_i2c.h"
#include "htpa_32x32d.h"
#include "fpioa.h"

htpa_t htpa;

void get_date_time(bool alarm) {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    rtc_timer_get(&year, &month, &day, &hour, &minute, &second);
    printf("%4d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, minute, second);
}

int main(void) {
    rtc_init();
    get_date_time(false);
    rtc_timer_set(2020, 8, 29, 14, 0, 0);
    // rtc_alarm_set(2018, 9, 12, 23, 31, 00);
    get_date_time(false);
    printf("start loop\n");

    int32_t pixels[1024];
    int32_t min_max[4];
    uint8_t img[1024];
    uint16_t height;

    i2c_device_number_t i2c_num = I2C_DEVICE_0;
    // // i2c_init(i2c_num);
    int res = htpa_init(&htpa, i2c_num, 18, 19, 1000000);
    printf("htpa init status: %d\n", res);
    // htpa_height(htpa, );
    if (res == 0) {
        int re = htpa_temperature(&htpa, pixels);
        printf("htpa_temperature: %d\n", re);
        for (int i=0; i<1024; i++) {
            printf("%d,", pixels[i]);
        }
        printf("\n");
    }else {
        printf("==== init status: %d\n", res);
    }

    // htpa_height(&htpa, &height);
    // printf("height: %d\n", height);
    htpa_get_min_max(&htpa, min_max);
    htpa_get_to_image(&htpa, min_max[0], min_max[1], img);

    // fpioa_set_function(18, FUNC_I2C0_SCLK);
    // fpioa_set_function(19, FUNC_I2C0_SDA);
    // maix_i2c_init(i2c_num, 7, HTPA_EEPROM_I2C_FREQ);
    // uint8_t temp[2] = {HTPA_CMD_ADDR_CONF, 0x01}; // status register
    // int res = maix_i2c_send_data(i2c_num, HTPA_ADDR, temp, 1, 100);
    // printf("htpa init status: %d\n", res);
    // htpa_temperature(&htpa, &pixels);
    // for (int i=0; i<1024; i++) {
    //     printf("%d,", pixels[i]);
    // }

    while (1) {
        sleep(1);
        get_date_time(false);
    }
    return 0;
}
