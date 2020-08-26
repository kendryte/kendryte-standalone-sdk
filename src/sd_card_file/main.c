#include "bsp.h"
#include "dmac.h"
#include "ff.h"
#include "fpioa.h"
#include "plic.h"
#include "sdcard.h"
#include "sysctl.h"
#include "uarths.h"
#include <stdio.h>

void io_mux_init(void) {
    fpioa_set_function(29, FUNC_SPI0_SCLK);
    fpioa_set_function(30, FUNC_SPI0_D0);
    fpioa_set_function(31, FUNC_SPI0_D1);
    fpioa_set_function(32, FUNC_GPIOHS7);
    fpioa_set_function(24, FUNC_SPI0_SS3);
}

static int sdcard_init(void) {
    uint8_t status;

    printf("/******************sdcard test*****************/\n");
    status = sd_init();
    printf("sd init %d\n", status);
    if (status != 0) {
        return status;
    }

    printf("card info status %d\n", status);
    printf("CardCapacity:%ld\n", cardinfo.CardCapacity);
    printf("CardBlockSize:%d\n", cardinfo.CardBlockSize);
    return 0;
}

static int fs_init(void) {
    static FATFS sdcard_fs;
    FRESULT status;
    DIR dj;
    FILINFO fno;

    printf("/********************fs test*******************/\n");
    status = f_mount(&sdcard_fs, _T("0:"), 1);
    printf("mount sdcard:%d\n", status);
    if (status != FR_OK)
        return status;

    printf("printf filename\n");
    status = f_findfirst(&dj, &fno, _T("0:"), _T("*"));
    while (status == FR_OK && fno.fname[0]) {
        if (fno.fattrib & AM_DIR)
            printf("dir:%s\n", fno.fname);
        else
            printf("file:%s\n", fno.fname);
        status = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);
    return 0;
}

int main(void) {
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);

    io_mux_init();
    plic_init();
    sysctl_enable_irq();

    if (sdcard_init()) {
        printf("SD card err\n");
        return -1;
    }
    if (fs_init()) {
        printf("FAT32 err\n");
        return -1;
    }

    FIL file;
    FRESULT ret = FR_OK;
    // mkdir
    char *dir = "attendance";
    ret = f_mkdir(dir);
    if (ret == FR_OK) {
        printf("Mkdir %s ok\n", dir);
    } else {
        printf("Exist %s [%d]\n", dir, ret);
    }    

    // format should be attendance/datetime_attendance.csv
    printf("/*******************sd read write test*******************/\n");
    char *path = "attendance/20200826_attendance.csv";
    uint32_t v_ret_len = 0;
    FILINFO v_fileinfo;
    // get file status
    if ((ret = f_stat(path, &v_fileinfo)) == FR_OK) {
        // file exists
        printf("%s length is %lld\n", path, v_fileinfo.fsize);
    } else {
        // file is not exit
        printf("%s fstat err [%d]\n", path, ret);
        // open file with write model
        if ((ret = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
            printf("open file %s err[%d]\n", path, ret);
            return ret;
        } else {
            printf("Open %s ok\n", path);
            // text context
            char csv_header[42] = "timestamp, id, ambient_temperature, mask\n";
            ret = f_write(&file, csv_header, sizeof(csv_header), &v_ret_len);
            if (ret != FR_OK) {
                printf("Write %s err[%d]\n", path, ret);
            } else {
                printf("Write %d bytes to %s ok\n", v_ret_len, path);
            }
        }
        f_close(&file);
    }

    // open file with READ model
    if ((ret = f_open(&file, path, FA_READ)) == FR_OK) {
        char v_buf[80] = {0};
        // read file
        ret = f_read(&file, (void *)v_buf, 64, &v_ret_len);
        // read wrong
        if (ret != FR_OK) {
            printf("Read %s err[%d]\n", path, ret);
        } else {
            // read success
            printf("Read :> %s %d bytes lenth\n", v_buf, v_ret_len);
        }
        f_close(&file);
    }



    while (1)
        ;

    return 0;
}
