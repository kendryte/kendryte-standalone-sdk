#include "rtc.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "htpa.h"
#include "sipeed_i2c.h"
#include "htpa_32x32d.h"
#include "fpioa.h"
#include "ff.h"

htpa_t htpa;

void get_date_time(char *datetime) {
    int year, month, day, hour, minute, second;
    rtc_timer_get(&year, &month, &day, &hour, &minute, &second);
    sprintf(datetime, "%4d-%02d-%02d %02d:%02d:%02d", year, month, day, hour,
            minute, second);
    printf("%s\n", datetime);
}

// int add_attendance(char* phone, char* id, float temper, char* mask) {
//     FIL file;
//     FRESULT ret = FR_OK;
//     FILINFO v_fileinfo;
//     uint32_t v_ret_len = 0;
//     char *path = "attendance.csv";
//     char *added_attend;

//     if ((ret = f_stat(path, &v_fileinfo)) == FR_OK) {
//         // file exists
//         printf("%s length is %lld\n", path, v_fileinfo.fsize);
//         ret = f_open(&file, path, FA_OPEN_EXISTING | FA_WRITE);
//     } else {
//         if ((ret = f_open(&file, path, FA_CREATE_NEW | FA_WRITE)) == FR_OK) {
//             printf("Create %s ok\n", path);
//             // init csv header
//             char csv_header[49] = "timestamp, phone, id, ambient_temperature, mask\n";
//             ret = f_write(&file, csv_header, sizeof(csv_header), &v_ret_len);
//             if (ret != FR_OK) {
//                 printf("Write %s err[%d]\n", path, ret);
//             } else {
//                 printf("Write %d bytes to %s ok\n", v_ret_len, path);
//             }
//         }
//     }
//     char datetime[19];
//     get_date_time(datetime);
//     sprintf(added_attend, "%s, %s, %s, %f, %s\n", datetime, phone, id, temper, mask);
//     ret = f_lseek(&file, f_size(&file));
//     ret = f_write(&file, added_attend, sizeof(added_attend), &v_ret_len);
//     f_close(&file);
//     printf("file status: %d\n", ret);
//     return ret;
//     // file test success
// }

int main(void) {
    char datetime[19];

    rtc_init();
    rtc_timer_set(2020, 9, 2, 9, 0, 0);
    get_date_time(datetime);

    // // flash init
    // printf("flash init\n");
    // w25qxx_init(3, 0);
    // w25qxx_enable_quad_mode();
    // add_attendance("010-6518-2866", "user1", 36.5, "y");

    int16_t pixels[1024];
    int16_t min_max[4];
    int16_t htpa_img[1024];

    int htpa_stat = htpa_init(&htpa, I2C_DEVICE_0, 18, 19, 1000000);
    printf("htpa init status: %d\n", htpa_stat);
    if (htpa_stat == 0) {
        int htpa_temp = htpa_temperature(&htpa, pixels);
        printf("htpa_temperature: %d\n====================\n", htpa_temp);
        for (int i=0; i<1024; i++) {
            printf("%d,", pixels[i]);
        }
        printf("\n====================\n");
    }
    // else {
    //     printf("==== init status: %d\n", res);
    // }

    htpa_get_min_max(&htpa, min_max);
    printf("Max: %d, Min: %d\n====================\n", min_max[1], min_max[0]);
    htpa_get_to_image(&htpa, min_max[0], min_max[1], htpa_img);
    for (int i=0; i<1024; i++) {
        printf("%d,", htpa_img[i]);
    }
    printf("\n====================\n");

    while (1) {
        sleep(1);
        // get_date_time(datetime);
    }
    return 0;
}
