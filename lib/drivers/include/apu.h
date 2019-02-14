#ifndef _apu_H_
#define _apu_H_

#if defined(__cplusplus)
extern “C” {
#endif

#define DIRECTION_RES 16
#define I2S_FS 44100
#define SOUND_SPEED 340

extern volatile struct apu_reg_t *const apu;

enum en_bf_dir
{
    APU_DIR0 = 0,
    APU_DIR1,
    APU_DIR2,
    APU_DIR3,
    APU_DIR4,
    APU_DIR5,
    APU_DIR6,
    APU_DIR7,
    APU_DIR8,
    APU_DIR9,
    APU_DIR10,
    APU_DIR11,
    APU_DIR12,
    APU_DIR13,
    APU_DIR14,
    APU_DIR15,
};

struct apu_ch_cfg_t
{
    /**
     * BF unit sound channel enable control bits.
     * Bit 'x' corresponds to enable bit for sound channel 'x' (x = 0, 1, 2,
     * . . ., 7). BF sound channels are related with I2S host RX channels.
     * BF sound channel 0/1 correspond to the left/right channel of I2S RX0;
     * BF channel 2/3 correspond to left/right channels of I2S RX1; and
     * things like that. 0x1: writing '1' to enable the corresponding BF
     * sound channel. 0x0: writing '0' to close the corresponding BF sound
     * channel.
     */
    uint32_t bf_sound_ch_en : 8;
    /**
     * Target direction select for valid voice output.
     * When the source voice direaction searching is done, software can use
     * this field to select one from 16 sound directions for the following
     * voice recognition. 0x0: select sound direction 0;   0x1: select sound
     * direction 1; . . . . . . 0xF: select sound direction 15.
     */
    uint32_t bf_target_dir : 4;
    /**
     * This is the audio sample gain factor. Using this gain factor to
     * enhance or reduce the stength of the sum of at most 8 source
     * sound channel outputs. This is a unsigned 11-bit fix-point number,
     * bit 10 is integer part and bit 9~0 are the fractional part.
     */
    uint32_t audio_gain : 11;
    uint32_t reserved1 : 1;
    /**
     * audio data source configure parameter. This parameter controls where
     * the audio data source comes from. 0x0: audio data directly sourcing
     * from apu internal buffer; 0x1: audio data sourcing from
     * FFT result buffer.
     */
    uint32_t data_src_mode : 1;
    uint32_t reserved2 : 3;
    /**
     * write enable for bf_sound_ch_en parameter.
     * 0x1: allowing updates made to 'bf_sound_ch_en'.
     * Access Mode: write only
     */
    uint32_t we_bf_sound_ch_en : 1;
    /**
     * write enable for bf_target_dir parameter.
     * 0x1: allowing updates made to 'bf_target_dir'.
     * Access Mode: write only
     */
    uint32_t we_bf_target_dir : 1;
    /**
     * write enable for audio_gain parameter.
     * 0x1: allowing updates made to 'audio_gain'.
     * Access Mode: write only
     */
    uint32_t we_audio_gain : 1;
    /**
     * write enable for data_out_mode parameter.
     * 0x1: allowing updates made to 'data_src_mode'.
     */
    uint32_t we_data_src_mode : 1;
} __attribute__((packed, aligned(4)));

struct apu_ctl_t
{
    /**
     * Sound direction searching enable bit.
     * Software writes '1' to start sound direction searching function.
     * When all the sound sample buffers are filled full, this bit is
     * cleared by hardware (this sample buffers are used for direction
     * detect only). 0x1: enable direction searching.
     */
    uint32_t bf_dir_search_en : 1;
    /*
     *use this parameter to reset all the control logic on direction search processing path.  This bit is self-clearing.
     *  0x1: apply reset to direction searching control logic;
     *  0x0: No operation.
     */
    uint32_t search_path_reset : 1;
    uint32_t reserved : 2;
    /**
     * Valid voice sample stream generation enable bit.
     * After sound direction searching is done, software can configure this
     * bit to generate a stream of voice samples for voice recognition. 0x1:
     * enable output of voice sample stream. 0x0: stop the voice samlpe
     * stream output.
     */
    uint32_t bf_stream_gen_en : 1;
    /*
     *use this parameter to reset all the control logic on voice stream generating path.  This bit is self-clearing.
     *  0x1: apply reset to voice stream generating control logic;
     *  0x0: No operation.
     */
    uint32_t voice_gen_path_reset : 1;
    /*
     *use this parameter to switch to a new voice source direction.  Software write '1' here and hardware will automatically clear it.
     *  0x1: write '1' here to request switching to new voice source direction.
     */
    uint32_t update_voice_dir : 1;

    uint32_t reserved1 : 1;
    //write enable for 'bf_dir_search_en' parameter.
    uint32_t we_bf_dir_search_en : 1;
    uint32_t we_search_path_rst : 1;
    uint32_t we_bf_stream_gen : 1;
    uint32_t we_voice_gen_path_rst : 1;
    uint32_t we_update_voice_dir : 1;
    uint32_t reserved2 : 19;

} __attribute__((packed, aligned(4)));

struct apu_dir_bidx_t
{
    uint32_t dir_rd_idx0 : 6;
    uint32_t reserved : 2;
    uint32_t dir_rd_idx1 : 6;
    uint32_t reserved1 : 2;
    uint32_t dir_rd_idx2 : 6;
    uint32_t reserved2 : 2;
    uint32_t dir_rd_idx3 : 6;
    uint32_t reserved3 : 2;
} __attribute__((packed, aligned(4)));

struct apu_fir_coef_t
{
    uint32_t fir_tap0 : 16;
    uint32_t fir_tap1 : 16;
} __attribute__((packed, aligned(4)));

struct apu_dwsz_cfg_t
{
    /**
     * TThe down-sizing ratio used for direction searching.
     * 0x0: no down-sizing;
     * 0x1: 1/2 down sizing;
     * 0x2: 1/3 down sizing;
     * . . . . . .
     * 0xF: 1/16 down sizing.
     */
    uint32_t dir_dwn_siz_rate : 4;
    /**
     * The down-sizing ratio used for voice stream generation.
     * 0x0: no down-sizing;
     * 0x1: 1/2 down sizing;
     * 0x2: 1/3 down sizing;
     * . . . . . .
     * 0xF: 1/16 down sizing.
     */
    uint32_t voc_dwn_siz_rate : 4;
    /**
     * This bit field is used to perform sample precision reduction when
     * the source sound sample (from I2S0 host receiving channels)
     * precision is 20/24/32 bits.
     * 0x0: take bits 15~0 from the source sound sample;
     * 0x1: take bits 16~1 from the source sound sample;
     * 0x2: take bits 17~2 from the source sound sample;
     * . . . . . .
     * 0x10: take bits 31~16 from the source sound sample;
     */
    uint32_t smpl_shift_bits : 5;
    uint32_t reserved : 19;
} __attribute__((packed, aligned(4)));

//0x31c
struct apu_fft_cfg_t
{
    uint32_t fft_shift_factor : 9;
    uint32_t reserved1 : 3;
    uint32_t fft_enable : 1;
    uint32_t reserved2 : 19;
} __attribute__((packed, aligned(4)));

//0x328
struct apu_int_stat_t
{
    /**
     * sound direction searching data ready interrupt event.
     * Writing '1' to clear this interrupt event.
     * 0x1: data is ready for sound direction detect;
     * 0x0: no event.
     */
    uint32_t dir_search_data_rdy : 1;
    /**
     * voice output stream buffer data ready interrupt event.
     * When a block of 512 voice samples are collected, this interrupt event
     * is asserted. Writing '1' to clear this interrupt event. 0x1: voice
     * output stream buffer data is ready; 0x0: no event.
     */
    uint32_t voc_buf_data_rdy : 1;
    uint32_t reserved : 30;
} __attribute__((packed, aligned(4)));
/*
 */
//0x32c
struct apu_int_mask_t
{
    /**
     * This is the interrupt mask to dir searching data ready interrupt.
     * 0x1: mask off this interrupt;
     * 0x0: enable this interrupt.
     */
    uint32_t dir_data_rdy_msk : 1;
    /**
     * This is the interrupt mask to voice output stream buffer ready
     * interrupt. 0x1: mask off this interrupt; 0x0: enable this interrupt.
     */
    uint32_t voc_buf_rdy_msk : 1;
    uint32_t reserved : 30;
} __attribute__((packed, aligned(4)));

struct apu_reg_t
{
    //0x200
    struct apu_ch_cfg_t     bf_ch_cfg_reg;
    //0x204
    struct apu_ctl_t            bf_ctl_reg;
    //0x208
    struct apu_dir_bidx_t       bf_dir_bidx[16][2];
    //0x288
    struct apu_fir_coef_t       bf_pre_fir0_coef[9];
    //0x2ac
    struct apu_fir_coef_t       bf_post_fir0_coef[9];
    //0x2d0
    struct apu_fir_coef_t       bf_pre_fir1_coef[9];
    //0x2f4
    struct apu_fir_coef_t       bf_post_fir1_coef[9];
    //0x318
    struct apu_dwsz_cfg_t       bf_dwsz_cfg_reg;
    //0x31c
    struct apu_fft_cfg_t        bf_fft_cfg_reg;
    // 0x320
    /**
     * This is the read register for system DMA to read data stored in
     * sample out buffers (the sample out buffers are used for sound
     * direction detect). Each data contains two sound samples.
     */
    volatile uint32_t           sobuf_dma_rdata;
    // 0x324
    /**
     * This is the read register for system DMA to read data stored in voice
     * out buffers (the voice out buffers are used for voice recognition).
     * Each data contains two sound samples.
     */
    volatile uint32_t           vobuf_dma_rdata;
    //0x328
    struct apu_int_stat_t       bf_int_stat_reg;
    //0x32c
    struct apu_int_mask_t       bf_int_mask_reg;
    //0x330
    uint32_t                saturation_counter;
    //0x334
    uint32_t                saturation_limits;
} __attribute__((packed, aligned(4)));

void apu_set_audio_gain(uint16_t gain);
void apu_set_smpl_shift(uint8_t smpl_shift);
uint8_t apu_get_smpl_shift(void);
void apu_set_channel_enabled(uint8_t channel_bit);
void apu_set_direction_delay(uint8_t dir_num, uint8_t *dir_bidx);
void apu_set_delay(float R, uint8_t mic_num_a_circle, uint8_t center);

void apu_set_fft_shift_factor(uint8_t enable_flag, uint16_t shift_factor);
void apu_set_down_size(uint8_t dir_dwn_siz, uint8_t voc_dwn_siz); //split to 2 functions
void apu_set_interrupt_mask(uint8_t dir_int_mask, uint8_t voc_int_mask); //split to 2 functions


void apu_dir_enable(void);
void apu_dir_reset(void);
void apu_dir_set_prev_fir(uint16_t *fir_coef);
void apu_dir_set_post_fir(uint16_t *fir_coef);
void apu_dir_set_down_size(uint8_t dir_dwn_size);
void apu_dir_set_interrupt_mask(uint8_t dir_int_mask);
void apu_dir_clear_int_state(void);

void apu_voc_enable(uint8_t enable_flag);
void apu_voc_reset(void);
void apu_voc_set_direction(enum en_bf_dir direction);
void apu_voc_set_prev_fir(uint16_t *fir_coef);
void apu_voc_set_post_fir(uint16_t *fir_coef);
void apu_voc_set_down_size(uint8_t voc_dwn_size);
void apu_voc_set_interrupt_mask(uint8_t voc_int_mask);
void apu_voc_clear_int_state(void);
void apu_voc_reset_saturation_counter(void);
uint32_t apu_voc_get_saturation_counter(void);
void apu_voc_set_saturation_limit(uint16_t upper, uint16_t bottom);
uint32_t apu_voc_get_saturation_limit(void);

void apu_print_setting(void);

#if defined(__cplusplus)
}
#endif
#endif
