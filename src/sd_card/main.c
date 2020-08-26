#include <stdio.h>
#include "fpioa.h"
#include "sysctl.h"
#include "dmac.h"
#include "fpioa.h"
#include "sdcard.h"
#include "ff.h"
#include "i2s.h"
#include "plic.h"
#include "wav_decode.h"
#include "uarths.h"
#include "bsp.h"

static int sdcard_test(void);
static int fs_test(void);
static int wav_test(TCHAR *path);
FRESULT sd_write_test(TCHAR *path);

void io_mux_init(void)
{
    fpioa_set_function(29, FUNC_SPI0_SCLK);
    fpioa_set_function(30, FUNC_SPI0_D0);
    fpioa_set_function(31, FUNC_SPI0_D1);
	fpioa_set_function(32, FUNC_GPIOHS7);

    fpioa_set_function(24, FUNC_SPI0_SS3);

    fpioa_set_function(33, FUNC_I2S0_OUT_D0);
    fpioa_set_function(35, FUNC_I2S0_SCLK);
    fpioa_set_function(34, FUNC_I2S0_WS);

}

int main(void)
{
    sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);

    io_mux_init();
    dmac_init();
    plic_init();
    sysctl_enable_irq();

    if(sdcard_test())
    {
        printf("SD card err\n");
        return -1;
    }
    if(fs_test())
    {
        printf("FAT32 err\n");
        return -1;
    }
    if(sd_write_test(_T("0:test.txt")))
    {
        printf("SD write err\n");
        return -1;
    }
    while (1) {
        if(wav_test(_T("0:music1.wav")))
        {
            printf("Play music err\n");
            return -1;
        }
    }

    return 0;
}

static int sdcard_test(void)
{
    uint8_t status;

    printf("/******************sdcard test*****************/\n");
    status = sd_init();
    printf("sd init %d\n", status);
    if (status != 0)
    {
        return status;
    }

    printf("card info status %d\n", status);
    printf("CardCapacity:%ld\n", cardinfo.CardCapacity);
    printf("CardBlockSize:%d\n", cardinfo.CardBlockSize);
    return 0;
}

static int fs_test(void)
{
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

struct wav_file_t wav_file;

static int on_irq_dma3(void *ctx)
{
    if (wav_file.buff_end) {
        wav_file.buff_end = 2;
        return 0;
    }
    if (wav_file.buff_index == 0) {
        if (wav_file.buff1_used == 0) {
            printf("error\n");
            return 0;
        }
        wav_file.buff0_used = 0;
        wav_file.buff_index = 1;
        wav_file.buff_current = wav_file.buff1;
        wav_file.buff_current_len = wav_file.buff1_read_len;
        if (wav_file.buff1_len > wav_file.buff1_read_len)
            wav_file.buff_end = 1;
    } else if (wav_file.buff_index == 1) {
        if (wav_file.buff0_used == 0) {
            printf("error\n");
            return 0;
        }
        wav_file.buff1_used = 0;
        wav_file.buff_index = 0;
        wav_file.buff_current = wav_file.buff0;
        wav_file.buff_current_len = wav_file.buff0_read_len;
        if (wav_file.buff0_len > wav_file.buff0_read_len)
            wav_file.buff_end = 1;
    }

    i2s_play(I2S_DEVICE_0,
            DMAC_CHANNEL3, (void *)wav_file.buff_current, wav_file.buff_current_len, wav_file.buff_current_len, 16, 2);

    return 0;
}

FRESULT sd_write_test(TCHAR *path)
{
    FIL file;
    FRESULT ret = FR_OK;
    printf("/*******************sd write test*******************/\n");
    uint32_t v_ret_len = 0;

    FILINFO v_fileinfo;
    if((ret = f_stat(path, &v_fileinfo)) == FR_OK)
    {
        printf("%s length is %lld\n", path, v_fileinfo.fsize);
    }
    else
    {
        printf("%s fstat err [%d]\n", path, ret);
    }

    if((ret = f_open(&file, path, FA_READ)) == FR_OK)
    {
        char v_buf[64] = {0};
        ret = f_read(&file, (void *)v_buf, 64, &v_ret_len);
        if(ret != FR_OK)
        {
            printf("Read %s err[%d]\n", path, ret);
        }
        else
        {
            printf("Read :> %s %d bytes lenth\n", v_buf, v_ret_len);
        }
        f_close(&file);
    }

    if ((ret = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
        printf("open file %s err[%d]\n", path, ret);
        return ret;
    }
    else
    {
        printf("Open %s ok\n", path);
    }
    uint8_t hello[1024];
    uint32_t i;
    for(i = 0; i < 1024; i++)
    {
        hello[i] = 'A';
    }
    ret = f_write(&file, hello, sizeof(hello), &v_ret_len);
    if(ret != FR_OK)
    {
        printf("Write %s err[%d]\n", path, ret);
    }
    else
    {
        printf("Write %d bytes to %s ok\n", v_ret_len, path);
    }
    f_close(&file);
    return ret;
}

static int wav_test(TCHAR *path)
{
    enum errorcode_e status;
    FIL file;

    printf("/*******************wav test*******************/\n");
    if (FR_OK != f_open(&file, path, FA_READ)) {
        printf("open file fail\n");
        return -1;
    }

    wav_file.fp = &file;
    status = wav_init(&wav_file);
    printf("result:%d\n", status);
    printf("point:0x%08x\n", (uint32_t)f_tell(&file));
    printf("numchannels:%d\n", wav_file.numchannels);
    printf("samplerate:%d\n", wav_file.samplerate);
    printf("byterate:%d\n", wav_file.byterate);
    printf("blockalign:%d\n", wav_file.blockalign);
    printf("bitspersample:%d\n", wav_file.bitspersample);
    printf("datasize:%d\n", wav_file.datasize);

    dmac_set_irq(DMAC_CHANNEL3, on_irq_dma3, NULL, 1);

    i2s_init(I2S_DEVICE_0, I2S_TRANSMITTER, 0x03);

    i2s_tx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_0,
        RESOLUTION_16_BIT, SCLK_CYCLES_32,
        /*TRIGGER_LEVEL_1*/TRIGGER_LEVEL_4,
        RIGHT_JUSTIFYING_MODE
        );


    printf("start decode\n");
    status = wav_decode_init(&wav_file);
    if (OK != status) {
        f_close(&file);
        printf("decode init fail\n");
        return -1;
    }

    i2s_play(I2S_DEVICE_0,
            DMAC_CHANNEL3, (void *)wav_file.buff_current, wav_file.buff_current_len, wav_file.buff_current_len, 16, 2);

    while (1) {
        status = wav_decode(&wav_file);
        if (FILE_END == status) {
            while (wav_file.buff_end != 2)
                ;
            printf("decode finish\n");
            break;
        } else if (FILE_FAIL == status) {
            printf("decode init fail\n");
            break;
        }
    }
    f_close(&file);
    wav_decode_finish(&wav_file);
    return 0;
}
