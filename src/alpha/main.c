#include "rtc.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>
// #include <sleep.h>

#define DATA_ADDRESS 0x000000
uint8_t* settings_buf;
uint8_t* test_settings_buf;

void get_date_time(char *datetime) {
    int year, month, day, hour, minute, second;
    rtc_timer_get(&year, &month, &day, &hour, &minute, &second);
    sprintf(datetime, "%4d-%02d-%02d %02d:%02d:%02d", year, month, day, hour,
            minute, second);
    printf("%s\n", datetime);
}

int main(void) {
    char datetime[19];

    rtc_init();
    printf("%s\n", "v1.0");

    // try to write data
    test_settings_buf = "fd=1\nmd=1";
    w25qxx_write_data(DATA_ADDRESS, test_settings_buf, 10);

    // get init timestamp from uart
    // read settings from uart
    // uart_receive_data(UART_NUM, &recv, 1);

    // set init timestamp
    rtc_timer_set(2020, 8, 28, 12, 30, 0);
    get_date_time(datetime);

    printf("start loop\n");
    while (1) {
        get_date_time(datetime);
        sleep(1);
    }
    return 0;
}
