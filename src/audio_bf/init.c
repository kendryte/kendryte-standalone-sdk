#include "init.h"
#include <stddef.h>
#include <stdio.h>
#include <printf.h>
#include "audio_bf.h"


uint64_t dir_logic_count;
uint64_t voc_logic_count;

#if AUDIO_BF_FFT_ENABLE
uint32_t AUDIO_BF_DIR_FFT_BUFFER[AUDIO_BF_DIR_CHANNEL_MAX]
				[AUDIO_BF_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
uint32_t AUDIO_BF_VOC_FFT_BUFFER[AUDIO_BF_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#else
int16_t AUDIO_BF_DIR_BUFFER[AUDIO_BF_DIR_CHANNEL_MAX][AUDIO_BF_DIR_CHANNEL_SIZE]
	__attribute__((aligned(128)));
int16_t AUDIO_BF_VOC_BUFFER[AUDIO_BF_VOC_CHANNEL_SIZE]
	__attribute__((aligned(128)));
#endif


int int_audio_bf(void *ctx)
{
	struct audio_bf_int_stat_t rdy_reg = audio_bf->bf_int_stat_reg;

	if (rdy_reg.dir_search_data_rdy) {
		audio_bf_dir_clear_int_state();

#if AUDIO_BF_FFT_ENABLE
		static int ch;

		ch = (ch + 1) % 16;
		for (uint32_t i = 0; i < 512; i++) { //
			uint32_t data = audio_bf->sobuf_dma_rdata;

			AUDIO_BF_DIR_FFT_BUFFER[ch][i] = data;
		}
		if (ch == 0) { //
			dir_logic_count++;
		}
#else
		for (uint32_t ch = 0; ch < AUDIO_BF_DIR_CHANNEL_MAX; ch++) {
			for (uint32_t i = 0; i < 256; i++) { //
				uint32_t data = audio_bf->sobuf_dma_rdata;

				AUDIO_BF_DIR_BUFFER[ch][i * 2 + 0] =
					data & 0xffff;
				AUDIO_BF_DIR_BUFFER[ch][i * 2 + 1] =
					(data >> 16) & 0xffff;
			}
		}
		dir_logic_count++;
#endif

	} else if (rdy_reg.voc_buf_data_rdy) {
		audio_bf_voc_clear_int_state();

#if AUDIO_BF_FFT_ENABLE
		for (uint32_t i = 0; i < 512; i++) { //
			uint32_t data = audio_bf->vobuf_dma_rdata;

			AUDIO_BF_VOC_FFT_BUFFER[i] = data;
		}
#else
		for (uint32_t i = 0; i < 256; i++) { //
			uint32_t data = audio_bf->vobuf_dma_rdata;

			AUDIO_BF_VOC_BUFFER[i * 2 + 0] = data & 0xffff;
			AUDIO_BF_VOC_BUFFER[i * 2 + 1] = (data >> 16) & 0xffff;
		}
#endif

		voc_logic_count++;
	} else { //
		printk("[waring]: unknown %s interrupt cause.\n", __func__);
	}
	return 0;
}

#if AUDIO_BF_DMA_ENABLE
int int_audio_bf_dir_dma(void *ctx)
{
	uint64_t chx_intstatus =
		dmac->channel[AUDIO_BF_DIR_DMA_CHANNEL].intstatus;
	if (chx_intstatus & 0x02) {
		dmac_chanel_interrupt_clear(AUDIO_BF_DIR_DMA_CHANNEL);

#if AUDIO_BF_FFT_ENABLE
		static int ch;

		ch = (ch + 1) % 16;
		dmac->channel[AUDIO_BF_DIR_DMA_CHANNEL].dar =
			(uint64_t)AUDIO_BF_DIR_FFT_BUFFER[ch];
#else
		dmac->channel[AUDIO_BF_DIR_DMA_CHANNEL].dar =
			(uint64_t)AUDIO_BF_DIR_BUFFER;
#endif

		dmac->chen = 0x0101 << AUDIO_BF_DIR_DMA_CHANNEL;

#if AUDIO_BF_FFT_ENABLE
		if (ch == 0) { //
			dir_logic_count++;
		}
#else
		dir_logic_count++;
#endif

	} else {
		printk("[warning] unknown dma interrupt. %lx %lx\n",
		       dmac->intstatus, dmac->com_intstatus);
		printk("dir intstatus: %lx\n", chx_intstatus);

		dmac_chanel_interrupt_clear(AUDIO_BF_DIR_DMA_CHANNEL);
	}
	return 0;
}


int int_audio_bf_voc_dma(void *ctx)
{
	uint64_t chx_intstatus =
		dmac->channel[AUDIO_BF_VOC_DMA_CHANNEL].intstatus;

	if (chx_intstatus & 0x02) {
		dmac_chanel_interrupt_clear(AUDIO_BF_VOC_DMA_CHANNEL);

#if AUDIO_BF_FFT_ENABLE
		dmac->channel[AUDIO_BF_VOC_DMA_CHANNEL].dar =
			(uint64_t)AUDIO_BF_VOC_FFT_BUFFER;
#else
		dmac->channel[AUDIO_BF_VOC_DMA_CHANNEL].dar =
			(uint64_t)AUDIO_BF_VOC_BUFFER;
#endif

		dmac->chen = 0x0101 << AUDIO_BF_VOC_DMA_CHANNEL;


		voc_logic_count++;

	} else {
		printk("[warning] unknown dma interrupt. %lx %lx\n",
		       dmac->intstatus, dmac->com_intstatus);
		printk("voc intstatus: %lx\n", chx_intstatus);

		dmac_chanel_interrupt_clear(AUDIO_BF_VOC_DMA_CHANNEL);
	}
	return 0;
}
#endif

void init_fpioa(void)
{
	printk("init fpioa.\n");
	fpioa_init();
	fpioa_set_function(47, FUNC_GPIOHS4);
	fpioa_set_function(42, FUNC_I2S0_IN_D0);
	fpioa_set_function(43, FUNC_I2S0_IN_D1);
	fpioa_set_function(44, FUNC_I2S0_IN_D2);
	fpioa_set_function(45, FUNC_I2S0_IN_D3);
	fpioa_set_function(46, FUNC_I2S0_WS);
	fpioa_set_function(39, FUNC_I2S0_SCLK);
}


void init_i2s(void)
{
	printk("init i2s.\n");

	/* I2s init */
    i2s_init(I2S_DEVICE_0, I2S_RECEIVER, 0x3);

    i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_0,
            RESOLUTION_16_BIT, SCLK_CYCLES_32,
            TRIGGER_LEVEL_4, STANDARD_MODE);
    i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_1,
            RESOLUTION_16_BIT, SCLK_CYCLES_32,
            TRIGGER_LEVEL_4, STANDARD_MODE);
    i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_2,
            RESOLUTION_16_BIT, SCLK_CYCLES_32,
            TRIGGER_LEVEL_4, STANDARD_MODE);
    i2s_rx_channel_config(I2S_DEVICE_0, I2S_CHANNEL_3,
            RESOLUTION_16_BIT, SCLK_CYCLES_32,
            TRIGGER_LEVEL_4, STANDARD_MODE);

    i2s_set_sample_rate(I2S_DEVICE_0, 44100);

}

void init_bf(void)
{
	printk("init bf.\n");
	uint16_t fir_prev_t[] = {
		0x020b, 0x0401, 0xff60, 0xfae2, 0xf860, 0x0022,
		0x10e6, 0x22f1, 0x2a98, 0x22f1, 0x10e6, 0x0022,
		0xf860, 0xfae2, 0xff60, 0x0401, 0x020b,
	};
	uint16_t fir_post_t[] = {
		0xf649, 0xe59e, 0xd156, 0xc615, 0xd12c, 0xf732,
		0x2daf, 0x5e03, 0x7151, 0x5e03, 0x2daf, 0xf732,
		0xd12c, 0xc615, 0xd156, 0xe59e, 0xf649,
	};

	uint16_t fir_neg_one[] = {
		0x8000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	uint16_t fir_common[] = {
		0x03c3, 0x03c3, 0x03c3, 0x03c3, 0x03c3, 0x03c3,
		0x03c3, 0x03c3, 0x03c3, 0x03c3, 0x03c3, 0x03c3,
		0x03c3, 0x03c3, 0x03c3, 0x03c3, 0x03c3,
	};
//3cm	
	// uint8_t offsets[16][8] = {
	// 	{0, 1, 5, 7, 7, 5, 1, 4, },
	// 	{0, 0, 3, 6, 7, 6, 3, 4, },
	// 	{1, 0, 2, 5, 8, 7, 4, 4, },
	// 	{2, 0, 1, 4, 7, 8, 6, 4, },
	// 	{4, 1, 0, 2, 6, 8, 7, 4, },
	// 	{5, 2, 0, 1, 4, 7, 8, 4, },
	// 	{6, 3, 0, 0, 2, 6, 7, 4, },
	// 	{7, 5, 1, 0, 1, 5, 7, 4, },
	// 	{7, 6, 3, 0, 0, 3, 6, 4, },
	// 	{8, 7, 4, 1, 0, 2, 5, 4, },
	// 	{7, 8, 6, 2, 0, 1, 4, 4, },
	// 	{6, 8, 7, 4, 1, 0, 2, 4, },
	// 	{4, 7, 8, 5, 2, 0, 1, 4, },
	// 	{3, 6, 7, 6, 3, 0, 0, 4, },
	// 	{1, 5, 7, 7, 5, 1, 0, 4, },
	// 	{0, 3, 6, 7, 6, 2, 0, 4, },
	// };

	audio_bf_dir_set_prev_fir(fir_neg_one);
	audio_bf_dir_set_post_fir(fir_neg_one);
	audio_bf_voc_set_prev_fir(fir_neg_one);
	audio_bf_voc_set_post_fir(fir_neg_one);

	audio_bf_set_delay(3, 7, 1);
	audio_bf_set_smpl_shift(AUDIO_BF_SMPL_SHIFT);
	audio_bf_voc_set_saturation_limit(AUDIO_BF_SATURATION_VPOS_DEBUG,
					  AUDIO_BF_SATURATION_VNEG_DEBUG);
	audio_bf_set_audio_gain(AUDIO_BF_AUDIO_GAIN_TEST);
	audio_bf_voc_set_direction(0);
	audio_bf_set_channel_enabled(0x3f);
	audio_bf_set_down_size(0, 0);

#if AUDIO_BF_FFT_ENABLE
	audio_bf_set_fft_shift_factor(1, 0xaa);
#else
	audio_bf_set_fft_shift_factor(0, 0);
#endif

	audio_bf_set_interrupt_mask(AUDIO_BF_DMA_ENABLE, AUDIO_BF_DMA_ENABLE);
#if AUDIO_BF_DIR_ENABLE
	audio_bf_dir_enable();
#endif
#if AUDIO_BF_VOC_ENABLE
	audio_bf_voc_enable(1);
#else
	audio_bf_voc_enable(0);
#endif
}

#if AUDIO_BF_DMA_ENABLE
void init_dma(void)
{
	printk("%s\n", __func__);
	// dmac enable dmac and interrupt
	union dmac_cfg_u dmac_cfg;

	dmac_cfg.data = readq(&dmac->cfg);
	dmac_cfg.cfg.dmac_en = 1;
	dmac_cfg.cfg.int_en = 1;
	writeq(dmac_cfg.data, &dmac->cfg);

	sysctl_dma_select(SYSCTL_DMA_CHANNEL_0 + AUDIO_BF_DIR_DMA_CHANNEL,
			  SYSCTL_DMA_SELECT_I2S0_BF_DIR_REQ);
	sysctl_dma_select(SYSCTL_DMA_CHANNEL_0 + AUDIO_BF_VOC_DMA_CHANNEL,
			  SYSCTL_DMA_SELECT_I2S0_BF_VOICE_REQ);
}
#endif

void init_dma_ch(int ch, volatile uint32_t *src_reg, void *buffer,
		 size_t size_of_byte)
{
	printk("%s %d\n", __func__, ch);

	dmac->channel[ch].sar = (uint64_t)src_reg;
	dmac->channel[ch].dar = (uint64_t)buffer;
	dmac->channel[ch].block_ts = (size_of_byte / 4) - 1;
	dmac->channel[ch].ctl =
		(((uint64_t)1 << 47) | ((uint64_t)15 << 48)
		 | ((uint64_t)1 << 38) | ((uint64_t)15 << 39)
		 | ((uint64_t)3 << 18) | ((uint64_t)3 << 14)
		 | ((uint64_t)2 << 11) | ((uint64_t)2 << 8) | ((uint64_t)0 << 6)
		 | ((uint64_t)1 << 4) | ((uint64_t)1 << 2) | ((uint64_t)1));
	/*
	 * dmac->channel[ch].ctl = ((  wburst_len_en  ) |
	 *                        (    wburst_len   ) |
	 *                        (  rburst_len_en  ) |
	 *                        (    rburst_len   ) |
	 *                        (one transaction:d) |
	 *                        (one transaction:s) |
	 *                        (    dst width    ) |
	 *                        (    src width   ) |
	 *                        (    dinc,0 inc  )|
	 *                        (  sinc:1,no inc ));
	 */

	dmac->channel[ch].cfg = (((uint64_t)1 << 49) | ((uint64_t)ch << 44)
				 | ((uint64_t)ch << 39) | ((uint64_t)2 << 32));
	/*
	 * dmac->channel[ch].cfg = ((     prior       ) |
	 *                         (      dst_per    ) |
	 *                         (     src_per     )  |
	 *           (    peri to mem  ));
	 *  01: Reload
	 */

	dmac->channel[ch].intstatus_en = 0x2; // 0xFFFFFFFF;
	dmac->channel[ch].intclear = 0xFFFFFFFF;

	dmac->chen = 0x0101 << ch;
}


void init_interrupt(void)
{
	plic_init();
	// bf
	plic_set_priority(IRQN_I2S0_INTERRUPT, 4);
	plic_irq_enable(IRQN_I2S0_INTERRUPT);
	plic_irq_register(IRQN_I2S0_INTERRUPT, int_audio_bf, NULL);

#if AUDIO_BF_DMA_ENABLE
	// dma
	plic_set_priority(IRQN_DMA0_INTERRUPT + AUDIO_BF_DIR_DMA_CHANNEL, 4);
	plic_irq_register(IRQN_DMA0_INTERRUPT + AUDIO_BF_DIR_DMA_CHANNEL,
			  int_audio_bf_dir_dma, NULL);
	plic_irq_enable(IRQN_DMA0_INTERRUPT + AUDIO_BF_DIR_DMA_CHANNEL);
	// dma
	plic_set_priority(IRQN_DMA0_INTERRUPT + AUDIO_BF_VOC_DMA_CHANNEL, 4);
	plic_irq_register(IRQN_DMA0_INTERRUPT + AUDIO_BF_VOC_DMA_CHANNEL,
			  int_audio_bf_voc_dma, NULL);
	plic_irq_enable(IRQN_DMA0_INTERRUPT + AUDIO_BF_VOC_DMA_CHANNEL);
#endif
}

void init_ws2812b(void)
{
	gpiohs->output_en.bits.b4 = 1;
	gpiohs->output_val.bits.b4 = 0;
}

void init_all(void)
{
	init_fpioa();
//	init_pll();
	init_interrupt();
	init_i2s();
	init_bf();

	if (AUDIO_BF_DMA_ENABLE) {
		#if AUDIO_BF_DMA_ENABLE
		init_dma();
		#endif
#if AUDIO_BF_FFT_ENABLE
		init_dma_ch(AUDIO_BF_DIR_DMA_CHANNEL,
			    &audio_bf->sobuf_dma_rdata,
			    AUDIO_BF_DIR_FFT_BUFFER[0], 512 * 4);
		init_dma_ch(AUDIO_BF_VOC_DMA_CHANNEL,
			    &audio_bf->vobuf_dma_rdata, AUDIO_BF_VOC_FFT_BUFFER,
			    512 * 4);
#else
		init_dma_ch(AUDIO_BF_DIR_DMA_CHANNEL,
			    &audio_bf->sobuf_dma_rdata, AUDIO_BF_DIR_BUFFER,
			    512 * 16 * 2);
		init_dma_ch(AUDIO_BF_VOC_DMA_CHANNEL,
			    &audio_bf->vobuf_dma_rdata, AUDIO_BF_VOC_BUFFER,
			    512 * 2);
#endif
	}
	init_ws2812b();
	// audio_bf_print_setting();
}
