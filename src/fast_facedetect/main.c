#include "board_config.h"
#include "fpioa.h"
#include "kpu.h"
#include "lcd.h"
#include "nt35310.h"
#include "image_process.h"
#include "plic.h"
#include "ov5640.h"
#include "sysctl.h"
#include "uarths.h"
#include "ultra_face.h"
#include "utils.h"
#include "dvp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

// #define RFB 1
// #define TEST 0

// extern const unsigned char gImage_image[] __attribute__((aligned(128)));

// #if TEST
// INCBIN(out, "face.bin");
// #else
// #if RFB
// INCBIN(model, "RFB-320.kmodel");
// #else
INCBIN(model, "slim-320.kmodel");
// #endif
// #endif
kpu_model_context_t task;

static image_t kpu_image, display_image;
volatile uint8_t g_dvp_finish_flag;
volatile uint8_t g_ai_done_flag;
static uint16_t lcd_gram[320 * 240] __attribute__((aligned(32)));

static int ai_done(void *ctx) {
    g_ai_done_flag = 1;
    return 0;
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

// #if BOARD_LICHEEDAN
// static void io_mux_init(void)
// {
//     /* Init DVP IO map and function settings */
//     fpioa_set_function(42, FUNC_CMOS_RST);
//     fpioa_set_function(44, FUNC_CMOS_PWDN);
//     fpioa_set_function(46, FUNC_CMOS_XCLK);
//     fpioa_set_function(43, FUNC_CMOS_VSYNC);
//     fpioa_set_function(45, FUNC_CMOS_HREF);
//     fpioa_set_function(47, FUNC_CMOS_PCLK);
//     fpioa_set_function(41, FUNC_SCCB_SCLK);
//     fpioa_set_function(40, FUNC_SCCB_SDA);

//     /* Init SPI IO map and function settings */
//     fpioa_set_function(38, FUNC_GPIOHS0 + DCX_GPIONUM);
//     fpioa_set_function(36, FUNC_SPI0_SS3);
//     fpioa_set_function(39, FUNC_SPI0_SCLK);
//     fpioa_set_function(37, FUNC_GPIOHS0 + RST_GPIONUM);

//     sysctl_set_spi0_dvp_data(1);
// }

// static void io_set_power(void)
// {
//     /* Set dvp and spi pin to 1.8V */
//     sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
//     sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
// }

// #else
static void io_mux_init(void) {
    /* Init DVP IO map and function settings */
    fpioa_set_function(11, FUNC_CMOS_RST);
    fpioa_set_function(13, FUNC_CMOS_PWDN);
    fpioa_set_function(14, FUNC_CMOS_XCLK);
    fpioa_set_function(12, FUNC_CMOS_VSYNC);
    fpioa_set_function(17, FUNC_CMOS_HREF);
    fpioa_set_function(15, FUNC_CMOS_PCLK);
    fpioa_set_function(10, FUNC_SCCB_SCLK);
    fpioa_set_function(9, FUNC_SCCB_SDA);

    /* Init SPI IO map and function settings */
    fpioa_set_function(8, FUNC_GPIOHS0 + DCX_GPIONUM);
    fpioa_set_function(6, FUNC_SPI0_SS3);
    fpioa_set_function(7, FUNC_SPI0_SCLK);

    sysctl_set_spi0_dvp_data(1);
}

static void io_set_power(void) {
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V18);
}
// #endif

static void drawboxes(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
                      uint32_t class, float prob) {
    if (x1 >= 320)
        x1 = 319;
    if (x2 >= 320)
        x2 = 319;
    if (y1 >= 240)
        y1 = 239;
    if (y2 >= 240)
        y2 = 239;

    lcd_draw_rectangle(x1, y1, x2, y2, 2, RED);
}

// void rgb888_to_lcd(uint8_t *src, uint16_t *dest, size_t width, size_t height) {
//     size_t i, chn_size = width * height;
//     for (size_t i = 0; i < width * height; i++) {
//         uint8_t r = src[i];
//         uint8_t g = src[chn_size + i];
//         uint8_t b = src[chn_size * 2 + i];

//         uint16_t rgb =
//             ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
//         size_t d_i = i % 2 ? (i - 1) : (i + 1);
//         dest[d_i] = rgb;
//     }
// }

int main(void) {
    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, PLL0_OUTPUT_FREQ);
    sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    uarths_init();

    io_mux_init();
    io_set_power();
    plic_init();

    /* LCD init */
    printf("LCD init\n");
    lcd_init();
    lcd_set_direction(DIR_YX_RLUD);

    // init ov5640
    dvp_init(16);
    dvp_set_xclk_rate(12000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(320, 240);
    ov5640_init();    
    // #endif
    /* enable global interrupt */
    sysctl_enable_irq();
    /* system start */
    printf("system start\n");
    kpu_image.pixel = 3;
    kpu_image.width = 320;
    kpu_image.height = 240;
    image_init(&kpu_image);
    display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);
    dvp_set_ai_addr((uint32_t)kpu_image.addr,
                    (uint32_t)(kpu_image.addr + 320 * 240),
                    (uint32_t)(kpu_image.addr + 320 * 240 * 2));
    dvp_set_display_addr((uint32_t)display_image.addr);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE,
                         0);
    dvp_disable_auto();
    /* DVP interrupt config */
    printf("DVP interrupt config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, dvp_irq, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);
// #if !TEST
    /* init kpu */
    if (kpu_load_kmodel(&task, model_data) != 0) {
        printf("\nmodel init error\n");
        while (1)
            ;
    }
// #endif

    ultra_face_init(320, 240, 0.3, 0.05, -1);

    while (1) {
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(
            DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while (g_dvp_finish_flag == 0)
            ;
        /* run face detect */
        g_ai_done_flag = 0;
// #if !TEST
        /* start to calculate */
        kpu_run_kmodel(&task, kpu_image.addr, DMAC_CHANNEL5, ai_done, NULL);
        while (!g_ai_done_flag)
            ;

        float *boxes;
        float *scores;
        size_t output_size;
        // #if RFB
        //         kpu_get_output(&task, 0, &boxes, &output_size);
        //         kpu_get_output(&task, 1, &scores, &output_size);
        // #else
        kpu_get_output(&task, 1, &boxes, &output_size);
        kpu_get_output(&task, 0, &scores, &output_size);
// #endif
// #else
//         float *boxes = out_data;
//         float *scores = out_data + 4420 * 4 * sizeof(float);
// #endif

        /* display pic*/
        // rgb888_to_lcd(gImage_image, lcd_gram, 320, 240);
        lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_image.addr);

        /* draw boxs */
        ultra_face_detect(scores, boxes, drawboxes);
    }
}
