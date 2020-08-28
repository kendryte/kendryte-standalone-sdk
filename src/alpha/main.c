#include "bsp.h"
#include "dmac.h"
#include "dvp.h"
#include "ff.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "image_process.h"
#include "incbin.h"
#include "iomem.h"
#include "kpu.h"
#include "lcd.h"
#include "nt35310.h"
#include "ov5640.h"
#include "plic.h"
#include "region_layer.h"
#include "rtc.h"
#include "sdcard.h"
#include "spi.h"
#include "sysctl.h"
#include "uart.h"
#include "uarths.h"
#include "utils.h"
#include "w25qxx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
// #include "ov2640.h"

#define SINGLE_FACE_DETECT 1

int result_face_detect = 0;
int result_mask_detect = 0;
int result_recognition = 0;
int result_temperature = 0;

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
// uart related
#define UART_NUM UART_DEVICE_3

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t kpu_image, display_image;

kpu_model_context_t face_detect_task;
static region_layer_t face_detect_rl;
static obj_info_t face_detect_info;
#define ANCHOR_NUM 5
static float anchor[ANCHOR_NUM * 2] = {1.889,    2.5245, 2.9465,   3.94056,
                                       3.99987,  5.3658, 5.155437, 6.92275,
                                       6.718375, 9.01025};
INCBIN(model, "detect.kmodel");

static void ai_done(void *ctx) { g_ai_done_flag = 1; }

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

void io_mux_init_uart_sd(void) {
    // uart
    fpioa_set_function(4, FUNC_UART1_RX + UART_NUM * 2);
    fpioa_set_function(5, FUNC_UART1_TX + UART_NUM * 2);
    fpioa_set_function(23, FUNC_GPIOHS3);
    // SD card
    fpioa_set_function(29, FUNC_SPI0_SCLK);
    fpioa_set_function(30, FUNC_SPI0_D0);
    fpioa_set_function(31, FUNC_SPI0_D1);
    fpioa_set_function(32, FUNC_GPIOHS7);
    fpioa_set_function(24, FUNC_SPI0_SS3);
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
    sysctl_set_spi0_dvp_data(1);
    fpioa_set_function(26, FUNC_GPIOHS0 + 8);
    gpiohs_set_drive_mode(8, GPIO_DM_INPUT);
}

static void io_set_power(void) {
    // Set dvp and spi pin to 1.8V
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V18);
}

void get_date_time(char *datetime) {
    int year, month, day, hour, minute, second;
    rtc_timer_get(&year, &month, &day, &hour, &minute, &second);
    sprintf(datetime, "%4d-%02d-%02d %02d:%02d:%02d", year, month, day, hour,
            minute, second);
    printf("%s\n", datetime);
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

static void draw_edge(uint32_t *gram, obj_info_t *obj_info, uint32_t index,
                      uint16_t color) {
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;

    x1 = obj_info->obj[index].x1;
    y1 = obj_info->obj[index].y1;
    x2 = obj_info->obj[index].x2;
    y2 = obj_info->obj[index].y2;

    if (x1 <= 0)
        x1 = 1;
    if (x2 >= 319)
        x2 = 318;
    if (y1 <= 0)
        y1 = 1;
    if (y2 >= 239)
        y2 = 238;

    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 8) / 2;
    addr3 = gram + (320 * (y2 - 1) + x1) / 2;
    addr4 = gram + (320 * (y2 - 1) + x2 - 8) / 2;
    for (uint32_t i = 0; i < 4; i++) {
        *addr1 = data;
        *(addr1 + 160) = data;
        *addr2 = data;
        *(addr2 + 160) = data;
        *addr3 = data;
        *(addr3 + 160) = data;
        *addr4 = data;
        *(addr4 + 160) = data;
        addr1++;
        addr2++;
        addr3++;
        addr4++;
    }
    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 2) / 2;
    addr3 = gram + (320 * (y2 - 8) + x1) / 2;
    addr4 = gram + (320 * (y2 - 8) + x2 - 2) / 2;
    for (uint32_t i = 0; i < 8; i++) {
        *addr1 = data;
        *addr2 = data;
        *addr3 = data;
        *addr4 = data;
        addr1 += 160;
        addr2 += 160;
        addr3 += 160;
        addr4 += 160;
    }
}

int read_img_from_sd() {
    // SD card related
    io_mux_init_uart_sd();

    if (sdcard_init()) {
        printf("SD card err\n");
        return -1;
    }
    if (fs_init()) {
        printf("FAT32 err\n");
        return -1;
    }

    // FIL file;
    FRESULT ret = FR_OK;
    FILINFO v_fileinfo;
    char *dir = "images";
    if ((ret = f_stat(dir, &v_fileinfo)) == FR_OK) {
        // file exists
        printf("=== DIR %s exists. ===\n", dir);
    }
    // // open file with READ model
    // if ((ret = f_open(&file, path, FA_READ)) == FR_OK) {
    //     char v_buf[80] = {0};
    //     // read file
    //     ret = f_read(&file, (void *)v_buf, 64, &v_ret_len);
    //     // read wrong
    //     if (ret != FR_OK) {
    //         printf("Read %s err[%d]\n", path, ret);
    //     } else {
    //         // read success
    //         printf("Read :> %s %d bytes lenth\n", v_buf, v_ret_len);
    //     }
    //     f_close(&file);
    // }

    return 0;
}

int read_data_from_uart() {
    // uart related
    gpiohs_set_drive_mode(3, GPIO_DM_OUTPUT);
    gpio_pin_value_t value = GPIO_PV_HIGH;
    gpiohs_set_pin(3, value);

    uart_init(UART_NUM);
    uart_configure(UART_NUM, 115200, 8, UART_STOP_1, UART_PARITY_NONE);
    // uart init finished

    // get init timestamp from uart
    // read settings from uart
    // char recv = 0;
    // while (uart_receive_data(UART_NUM, &recv, 1)){
    //     printf("%s", recv);
    // }
    return 0;
}

int init_dvp_lcd(){
    // warm up
    uarths_init();
    io_mux_init_dvp_spi();
    io_set_power();
    // lcd init
    printf("LCD init\n");
    lcd_init();
    // match with ov5640
    lcd_set_direction(DIR_YX_RLUD);
    lcd_clear(BLACK);
    // init ov5640
    dvp_init(16);
    dvp_set_xclk_rate(12000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(320, 240);
    ov5640_init();
}

int main(void) {
    char datetime[19];
    char *version = {"v1.8"};

    // Set CPU and dvp clk
    sysctl_pll_set_freq(SYSCTL_PLL0, PLL0_OUTPUT_FREQ);
    sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);

    plic_init();
    sysctl_enable_irq();

    // rtc init
    printf("RTC init ver. %s\n", version);
    rtc_init();
    read_img_from_sd();

    init_dvp_lcd();

    // // flash init
    // printf("flash init\n");
    // w25qxx_init(3, 0);
    // w25qxx_enable_quad_mode();

    uint8_t *model_data_align = model_data;

    // set init timestamp
    printf("RTC set time\n");
    rtc_timer_set(2020, 8, 28, 12, 30, 0);
    get_date_time(datetime);

    kpu_image.pixel = 3;
    kpu_image.width = 320;
    kpu_image.height = 240;
    image_init(&kpu_image);
    display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);
    dvp_set_ai_addr(
        (uint32_t)kpu_image.addr,
        (uint32_t)(kpu_image.addr + 320 * 240),
        (uint32_t)(kpu_image.addr + 320 * 240 * 2)
    );
    dvp_set_display_addr((uint32_t)display_image.addr);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE,
                         0);
    dvp_disable_auto();
    /* DVP interrupt config */
    printf("DVP interrupt config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, dvp_irq, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);
    /* init face detect model */
    if (kpu_load_kmodel(&face_detect_task, model_data_align) != 0) {
        printf("\nmodel init error\n");
        while (1)
            ;
    }
    face_detect_rl.anchor_number = ANCHOR_NUM;
    face_detect_rl.anchor = anchor;
    face_detect_rl.threshold = 0.7;
    face_detect_rl.nms_value = 0.3;
    region_layer_init(&face_detect_rl, 20, 15, 30, kpu_image.width, kpu_image.height);
    // enable global interrupt
    sysctl_enable_irq();
    // system start
    printf("System start\n");

    while (1) {
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while (g_dvp_finish_flag == 0)
            ;
        /* run face detect */
        g_ai_done_flag = 0;
        kpu_run_kmodel(&face_detect_task, kpu_image.addr, DMAC_CHANNEL5,
                       ai_done, NULL);
        while (!g_ai_done_flag)
            ;
        float *output;
        size_t output_size;
        kpu_get_output(&face_detect_task, 0, (uint8_t **)&output, &output_size);
        face_detect_rl.input = output;
        region_layer_run(&face_detect_rl, &face_detect_info);
        // run key point detect
        for (uint32_t face_cnt = 0; face_cnt < face_detect_info.obj_number; face_cnt++) {
            draw_edge((uint32_t *)display_image.addr, &face_detect_info, face_cnt, GREEN);
            #if SINGLE_FACE_DETECT
                break;
            #endif
        }
        // display result
        lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_image.addr);

    }
    // return 0;
}
