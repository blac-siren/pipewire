/* Simple Plugin API
 *
 * Copyright © 2018 Wim Taymans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __SPA_AUDIO_TYPES_H__
#define __SPA_AUDIO_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <spa/param/audio/raw.h>

#define SPA_TYPE__AudioFormat		SPA_TYPE_ENUM_BASE "AudioFormat"
#define SPA_TYPE_AUDIO_FORMAT_BASE	SPA_TYPE__AudioFormat ":"

static const struct spa_type_info spa_type_audio_format[] = {
	{ SPA_AUDIO_FORMAT_UNKNOWN, SPA_TYPE_AUDIO_FORMAT_BASE "UNKNOWN", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_ENCODED, SPA_TYPE_AUDIO_FORMAT_BASE "ENCODED", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S8, SPA_TYPE_AUDIO_FORMAT_BASE "S8", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U8, SPA_TYPE_AUDIO_FORMAT_BASE "U8", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S16_LE, SPA_TYPE_AUDIO_FORMAT_BASE "S16LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S16_BE, SPA_TYPE_AUDIO_FORMAT_BASE "S16BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U16_LE, SPA_TYPE_AUDIO_FORMAT_BASE "U16LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U16_BE, SPA_TYPE_AUDIO_FORMAT_BASE "U16BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S24_32_LE, SPA_TYPE_AUDIO_FORMAT_BASE "S24_32LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S24_32_BE, SPA_TYPE_AUDIO_FORMAT_BASE "S24_32BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U24_32_LE, SPA_TYPE_AUDIO_FORMAT_BASE "U24_32LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U24_32_BE, SPA_TYPE_AUDIO_FORMAT_BASE "U24_32BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S32_LE, SPA_TYPE_AUDIO_FORMAT_BASE "S32LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S32_BE, SPA_TYPE_AUDIO_FORMAT_BASE "S32BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U32_LE, SPA_TYPE_AUDIO_FORMAT_BASE "U32LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U32_BE, SPA_TYPE_AUDIO_FORMAT_BASE "U32BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S24_LE, SPA_TYPE_AUDIO_FORMAT_BASE "S24LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S24_BE, SPA_TYPE_AUDIO_FORMAT_BASE "S24BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U24_LE, SPA_TYPE_AUDIO_FORMAT_BASE "U24LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U24_BE, SPA_TYPE_AUDIO_FORMAT_BASE "U24BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S20_LE, SPA_TYPE_AUDIO_FORMAT_BASE "S20LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S20_BE, SPA_TYPE_AUDIO_FORMAT_BASE "S20BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U20_LE, SPA_TYPE_AUDIO_FORMAT_BASE "U20LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U20_BE, SPA_TYPE_AUDIO_FORMAT_BASE "U20BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S18_LE, SPA_TYPE_AUDIO_FORMAT_BASE "S18LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S18_BE, SPA_TYPE_AUDIO_FORMAT_BASE "S18BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U18_LE, SPA_TYPE_AUDIO_FORMAT_BASE "U18LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_U18_BE, SPA_TYPE_AUDIO_FORMAT_BASE "U18BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_F32_LE, SPA_TYPE_AUDIO_FORMAT_BASE "F32LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_F32_BE, SPA_TYPE_AUDIO_FORMAT_BASE "F32BE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_F64_LE, SPA_TYPE_AUDIO_FORMAT_BASE "F64LE", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_F64_BE, SPA_TYPE_AUDIO_FORMAT_BASE "F64BE", SPA_TYPE_Int, },

	{ SPA_AUDIO_FORMAT_U8P, SPA_TYPE_AUDIO_FORMAT_BASE "U8P", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S16P, SPA_TYPE_AUDIO_FORMAT_BASE "S16P", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S24_32P, SPA_TYPE_AUDIO_FORMAT_BASE "S24_32P", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S32P, SPA_TYPE_AUDIO_FORMAT_BASE "S32P", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_S24P, SPA_TYPE_AUDIO_FORMAT_BASE "S24P", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_F32P, SPA_TYPE_AUDIO_FORMAT_BASE "F32P", SPA_TYPE_Int, },
	{ SPA_AUDIO_FORMAT_F64P, SPA_TYPE_AUDIO_FORMAT_BASE "F64P", SPA_TYPE_Int, },

        { 0, NULL, },
};

#define SPA_TYPE__AudioFlags		SPA_TYPE_FLAGS_BASE "AudioFlags"
#define SPA_TYPE_AUDIO_FLAGS_BASE	SPA_TYPE__AudioFlags ":"

static const struct spa_type_info spa_type_audio_flags[] = {
	{ SPA_AUDIO_FLAG_NONE, SPA_TYPE_AUDIO_FLAGS_BASE "none", SPA_TYPE_Int, },
	{ SPA_AUDIO_FLAG_UNPOSITIONED, SPA_TYPE_AUDIO_FLAGS_BASE "unpositioned", SPA_TYPE_Int, },
        { 0, NULL, },
};

#define SPA_TYPE__AudioChannel		SPA_TYPE_ENUM_BASE "AudioChannel"
#define SPA_TYPE_AUDIO_CHANNEL_BASE	SPA_TYPE__AudioChannel ":"

static const struct spa_type_info spa_type_audio_channel[] = {
	{ SPA_AUDIO_CHANNEL_UNKNOWN, SPA_TYPE_AUDIO_CHANNEL_BASE "UNK", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_NA,	SPA_TYPE_AUDIO_CHANNEL_BASE "NA", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_MONO, SPA_TYPE_AUDIO_CHANNEL_BASE "MONO", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FL, SPA_TYPE_AUDIO_CHANNEL_BASE "FL", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FR, SPA_TYPE_AUDIO_CHANNEL_BASE "FR", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FC, SPA_TYPE_AUDIO_CHANNEL_BASE "FC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_LFE, SPA_TYPE_AUDIO_CHANNEL_BASE "LFE", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_SL, SPA_TYPE_AUDIO_CHANNEL_BASE "SL", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_SR, SPA_TYPE_AUDIO_CHANNEL_BASE "SR", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FLC, SPA_TYPE_AUDIO_CHANNEL_BASE "FLC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FRC, SPA_TYPE_AUDIO_CHANNEL_BASE "FRC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_RC, SPA_TYPE_AUDIO_CHANNEL_BASE "RC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_RL, SPA_TYPE_AUDIO_CHANNEL_BASE "RL", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_RR, SPA_TYPE_AUDIO_CHANNEL_BASE "RR", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TC, SPA_TYPE_AUDIO_CHANNEL_BASE "TC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TFL, SPA_TYPE_AUDIO_CHANNEL_BASE "TFL", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TFC, SPA_TYPE_AUDIO_CHANNEL_BASE "TFC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TFR, SPA_TYPE_AUDIO_CHANNEL_BASE "TFR", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TRL, SPA_TYPE_AUDIO_CHANNEL_BASE "TRL", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TRC, SPA_TYPE_AUDIO_CHANNEL_BASE "TRC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TRR, SPA_TYPE_AUDIO_CHANNEL_BASE "TRR", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_RLC, SPA_TYPE_AUDIO_CHANNEL_BASE "RLC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_RRC, SPA_TYPE_AUDIO_CHANNEL_BASE "RRC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FLW, SPA_TYPE_AUDIO_CHANNEL_BASE "FLW", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FRW, SPA_TYPE_AUDIO_CHANNEL_BASE "FRW", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_LFE2, SPA_TYPE_AUDIO_CHANNEL_BASE "LFE2", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FLH, SPA_TYPE_AUDIO_CHANNEL_BASE "FLH", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FCH, SPA_TYPE_AUDIO_CHANNEL_BASE "FCH", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_FRH, SPA_TYPE_AUDIO_CHANNEL_BASE "FRH", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TFLC, SPA_TYPE_AUDIO_CHANNEL_BASE "TFLC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TFRC, SPA_TYPE_AUDIO_CHANNEL_BASE "TFRC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TSL, SPA_TYPE_AUDIO_CHANNEL_BASE "TSL", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_TSR, SPA_TYPE_AUDIO_CHANNEL_BASE "TSR", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_LLFE, SPA_TYPE_AUDIO_CHANNEL_BASE "LLFR", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_RLFE, SPA_TYPE_AUDIO_CHANNEL_BASE "RLFE", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_BC, SPA_TYPE_AUDIO_CHANNEL_BASE "BC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_BLC, SPA_TYPE_AUDIO_CHANNEL_BASE "BLC", SPA_TYPE_Int, },
	{ SPA_AUDIO_CHANNEL_BRC, SPA_TYPE_AUDIO_CHANNEL_BASE "BRC", SPA_TYPE_Int, },
	{ 0, NULL, },
};

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __SPA_AUDIO_RAW_TYPES_H__ */