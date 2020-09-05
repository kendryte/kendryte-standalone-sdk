#include "dvp.h"
#include "ff.h"
#include "kpu.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "htpa.h"
#include "htpa_32x32d.h"
#include "image_process.h"
#include "lcd.h"
#include "ov5640.h"
#include "rtc.h"
#include "sipeed_i2c.h"
#include "sysctl.h"
#include "uarths.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

htpa_t htpa;
uint8_t g_ai_buf_in[32*32*1] __attribute__((aligned(128)));

volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t display_image;

kpu_model_context_t mask_detect_task;

static void ai_done(void *ctx) { g_ai_done_flag = 1; }

void get_date_time(char *datetime) {
    int year, month, day, hour, minute, second;
    rtc_timer_get(&year, &month, &day, &hour, &minute, &second);
    sprintf(datetime, "%4d-%02d-%02d %02d:%02d:%02d", year, month, day, hour,
            minute, second);
    printf("%s\n", datetime);
}

int add_attendance(char *phone, char *id, float temper, char *mask) {
    FIL file;
    FRESULT ret = FR_OK;
    FILINFO v_fileinfo;
    uint32_t v_ret_len = 0;
    char *path = "attendance.csv";
    char added_attend[128];

    if ((ret = f_stat(path, &v_fileinfo)) == FR_OK) {
        // file exists
        printf("%s length is %lld\n", path, v_fileinfo.fsize);
        ret = f_open(&file, path, FA_OPEN_EXISTING | FA_WRITE);
    } else {
        if ((ret = f_open(&file, path, FA_CREATE_NEW | FA_WRITE)) == FR_OK) {
            printf("Create %s ok\n", path);
            // init csv header
            char csv_header[49] =
                "timestamp, phone, id, ambient_temperature, mask\n";
            ret = f_write(&file, csv_header, sizeof(csv_header), &v_ret_len);
            if (ret != FR_OK) {
                printf("Write %s err[%d]\n", path, ret);
            } else {
                printf("Write %d bytes to %s ok\n", v_ret_len, path);
            }
        }
    }
    char datetime[19];
    get_date_time(datetime);
    sprintf(added_attend, "%s, %s, %s, %f, %s\n", datetime, phone, id, temper,
            mask);
    ret = f_lseek(&file, f_size(&file));
    ret = f_write(&file, added_attend, sizeof(added_attend), &v_ret_len);
    f_close(&file);
    printf("file status: %d\n", ret);
    return ret;
    // file test success
}

void io_mux_init_dvp_spi(void) {
    // Init DVP IO map and function settings
    fpioa_set_function(11, FUNC_CMOS_RST);
    fpioa_set_function(13, FUNC_CMOS_PWDN);
    fpioa_set_function(14, FUNC_CMOS_XCLK);
    fpioa_set_function(12, FUNC_CMOS_VSYNC);
    fpioa_set_function(17, FUNC_CMOS_HREF);
    fpioa_set_function(15, FUNC_CMOS_PCLK);
    fpioa_set_function(10, FUNC_SCCB_SCLK);
    fpioa_set_function(9, FUNC_SCCB_SDA);
    // Init SPI IO map and function settings
    fpioa_set_function(8, FUNC_GPIOHS0 + 2);
    fpioa_set_function(6, FUNC_SPI0_SS3);
    fpioa_set_function(7, FUNC_SPI0_SCLK);
    // magic here, not sure
    sysctl_set_spi0_dvp_data(1);
    fpioa_set_function(26, FUNC_GPIOHS0 + 8);
    gpiohs_set_drive_mode(8, GPIO_DM_INPUT);
}

static void io_set_power(void) {
    // Set dvp and spi pin to 1.8V
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V18);
}

static int dvp_irq(void *ctx) {
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH)) {
        dvp_config_interrupt(
            DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    } else {
        dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    return 0;
}

int main(void) {
    char datetime[19];
    // Set CPU and dvp clk
    sysctl_pll_set_freq(SYSCTL_PLL0, PLL0_OUTPUT_FREQ);
    sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    uarths_init();
    io_mux_init_dvp_spi();
    io_set_power();
    plic_init();
    // rtc init
    rtc_init();
    // LCD init
    printf("LCD init\r\n");
    lcd_init();
    lcd_set_direction(DIR_YX_RLDU);
    lcd_clear(BLACK);

    // // flash init
    // printf("flash init\n");
    // w25qxx_init(3, 0);
    // w25qxx_enable_quad_mode();
    // add_attendance("010-6518-2866", "user1", 36.5, "y");

    int16_t pixels[1024];
    int32_t min_max[4];
    uint8_t htpa_img[1024];

    int htpa_stat = htpa_init(&htpa, I2C_DEVICE_0, 18, 19, 1000000);
    printf("htpa init status: %d\n", htpa_stat);
    if (htpa_stat == 0) {
        int htpa_temp = htpa_temperature(&htpa, pixels);
        printf("htpa_temperature: %d\n====================\n", htpa_temp);
        for (int i = 0; i < 1024; i++) {
            printf("%d,", pixels[i]);
        }
        printf("\n====================\n");
    }

    htpa_get_min_max(&htpa, min_max);
    printf("Max: %d, Min: %d\n====================\n", min_max[1], min_max[0]);
    htpa_get_to_image(&htpa, min_max[0], min_max[1], htpa_img);
    for (int i = 0; i < 1024; i++) {
        printf("%d,", htpa_img[i]);
    }
    printf("\n====================\n");

    // for display
    display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);

    // // each channel addr of image
    // dvp_set_ai_addr((uint32_t)g_ai_buf_in);
    // dvp_set_display_addr((uint32_t)display_image.addr);
    // dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE,
    //                      0);
    // dvp_disable_auto();

    // DVP interrupt config
    printf("DVP interrupt config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, dvp_irq, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    while (1) {
        sleep(1);
        // g_dvp_finish_flag = 0;
        // dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        // dvp_config_interrupt(
        //     DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        // while (g_dvp_finish_flag == 0)
        //     ;
        // // run mask detect
        // g_ai_done_flag = 0;
        // kpu_run_kmodel(&mask_detect_task, htpa_img, DMAC_CHANNEL5,
        //                ai_done, NULL);
        // while (!g_ai_done_flag)
        //     ;
    }
    return 0;
}
