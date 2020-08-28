#include <stdlib.h>
#include "wav_decode.h"

/* Audio Parsing Constants */
#define  RIFF_ID		0x52494646  /* correspond to the letters 'RIFF' */
#define  WAVE_ID		0x57415645  /* correspond to the letters 'WAVE' */
#define  FMT_ID			0x666D7420  /* correspond to the letters 'fmt ' */
#define  LIST_ID		0x4C495354  /* correspond to the letters 'LIST' */
#define  DATA_ID		0x64617461  /* correspond to the letters 'data' */

#define BG_READ_WORD(x)	((((uint32_t)wav_head_buff[x + 0]) << 24) | (((uint32_t)wav_head_buff[x + 1]) << 16) |\
			(((uint32_t)wav_head_buff[x + 2]) << 8) | (((uint32_t)wav_head_buff[x + 3]) << 0))
#define LG_READ_WORD(x)	((((uint32_t)wav_head_buff[x + 3]) << 24) | (((uint32_t)wav_head_buff[x + 2]) << 16) |\
			(((uint32_t)wav_head_buff[x + 1]) << 8) | (((uint32_t)wav_head_buff[x + 0]) << 0))
#define LG_READ_HALF(x)	((((uint16_t)wav_head_buff[x + 1]) << 8) | (((uint16_t)wav_head_buff[x + 0]) << 0))

enum errorcode_e wav_init(struct wav_file_t *wav_file)
{
	uint8_t wav_head_buff[512];
	uint32_t index;

	if (FR_OK != f_read(wav_file->fp, wav_head_buff, 512, &index))
		return FILE_FAIL;

	index = 0;
	if (BG_READ_WORD(index) != RIFF_ID)
		return UNVALID_RIFF_ID;
	index += 4;
	if ((LG_READ_WORD(index) + 8) != f_size(wav_file->fp))
		return UNVALID_RIFF_SIZE;
	index += 4;
	if (BG_READ_WORD(index) != WAVE_ID)
		return UNVALID_WAVE_ID;
	index += 4;
	if (BG_READ_WORD(index) != FMT_ID)
		return UNVALID_FMT_ID;
	index += 4;
	if (LG_READ_WORD(index) != 0x10)
		return UNVALID_FMT_SIZE;
	index += 4;
	if (LG_READ_HALF(index) != 0x01)
		return UNSUPPORETD_FORMATTAG;
	index += 2;
	wav_file->numchannels = LG_READ_HALF(index);
	if (wav_file->numchannels != 1 && wav_file->numchannels != 2)
		return UNSUPPORETD_NUMBER_OF_CHANNEL;
	index += 2;
	wav_file->samplerate = LG_READ_WORD(index);
	if (wav_file->samplerate != 11025 && wav_file->samplerate != 22050 && wav_file->samplerate != 44100)
		return UNSUPPORETD_SAMPLE_RATE;
	index += 4;
	wav_file->byterate = LG_READ_WORD(index);
	index += 4;
	wav_file->blockalign = LG_READ_HALF(index);
	index += 2;
	wav_file->bitspersample = LG_READ_HALF(index);
	if (wav_file->bitspersample != 8 && wav_file->bitspersample != 16 && wav_file->bitspersample != 24)
		return UNSUPPORETD_BITS_PER_SAMPLE;
	index += 2;
	if (BG_READ_WORD(index) == LIST_ID) {
		index += 4;
		index += LG_READ_WORD(index);
		index += 4;
		if (index >= 500)
			return UNVALID_LIST_SIZE;
	}
	if (BG_READ_WORD(index) != DATA_ID)
		return UNVALID_DATA_ID;
	index += 4;
	wav_file->datasize = LG_READ_WORD(index);
	index += 4;
	if (FR_OK != f_lseek(wav_file->fp, index))
		return FILE_FAIL;
	return OK;
}

enum errorcode_e wav_decode_init(struct wav_file_t *wav_file)
{
	wav_file->buff_end = 0;
	wav_file->buff0_len = 256 * 1024;
	wav_file->buff1_len = 256 * 1024;
	wav_file->buff0  = (uint8_t *)malloc(wav_file->buff0_len);
	wav_file->buff1  = (uint8_t *)malloc(wav_file->buff1_len);
	if (FR_OK != f_read(wav_file->fp, wav_file->buff0, wav_file->buff0_len, &(wav_file->buff0_read_len)))
		return FILE_FAIL;
	if (wav_file->buff0_len > wav_file->buff0_read_len)
		wav_file->buff_end = 1;
	wav_file->buff_current  = wav_file->buff0;
	wav_file->buff_current_len  = wav_file->buff0_len;
	wav_file->buff0_used = 1;
	wav_file->buff1_used = 0;
	wav_file->buff_index = 0;
	return OK;
}

enum errorcode_e wav_decode(struct wav_file_t *wav_file)
{
	if (wav_file->buff0_used == 0) {
		if (FR_OK != f_read(wav_file->fp, wav_file->buff0, wav_file->buff0_len, &(wav_file->buff0_read_len)))
			return FILE_FAIL;
		wav_file->buff0_used = 1;
		if (wav_file->buff0_len > wav_file->buff0_read_len) {
			if (f_tell(wav_file->fp) == f_size(wav_file->fp))
				return FILE_END;
			return FILE_FAIL;
		}
	} else if (wav_file->buff1_used == 0) {
		if (FR_OK != f_read(wav_file->fp, wav_file->buff1, wav_file->buff1_len, &(wav_file->buff1_read_len)))
			return FILE_FAIL;
		wav_file->buff1_used = 1;
		if (wav_file->buff1_len > wav_file->buff1_read_len) {
			if (f_tell(wav_file->fp) == f_size(wav_file->fp))
				return FILE_END;
			return FILE_FAIL;
		}
	}
	return OK;
}

enum errorcode_e wav_decode_finish(struct wav_file_t *wav_file)
{
	free(wav_file->buff0);
	free(wav_file->buff1);
	return OK;
}
