/*
**
** Copyright (C) 2010 Eduardo José Tagle <ejtagle@tutopia.com>
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
**
** Author: Christian Bejram <christian.bejram@stericsson.com>
*/
#ifndef _AUDIOCHANNEL_H
#define _AUDIOCHANNEL_H 1

struct GsmAudioTunnel {
	int running;					// If running

	// 3G voice modem
	int fd;							// Voice data serial port handler

	// Common properties
	volatile int quit_flag;			// If threads should quit
	unsigned int frame_size;					// Frame size
	unsigned int sampling_rate;				// Sampling rate
	unsigned int bits_per_sample;			// Bits per sample. valid values = 16/8

	// Playback
	void* play_strm; // Playback stream
	volatile int play_thread_exited;// If play thread has exited
	void* play_buf;					// Pointer to the playback buffer
	unsigned int play_buf_count;				// Count of already stored samples in the playback buffer
	
	// Record
	void* rec_strm;	// Record stream
	volatile int rec_thread_exited;	// If record thread has exited
	void* rec_buf;					// Pointer to the recording buffer
	unsigned int rec_buf_count;				// Count of already stored samples in the recording buffer
};

#define GSM_AUDIO_CHANNEL_STATIC_INIT { 0, 0, 0,0,0,0, 0,0,0,0 ,0,0,0,0}

#ifdef __cplusplus
extern "C" {
#endif

int gsm_audio_tunnel_start(struct GsmAudioTunnel *stream,
	const char* gsmvoicechannel,
	unsigned int sampling_rate,
	unsigned int frame_size,
	unsigned int bits_per_sample);
	
int gsm_audio_tunnel_stop(struct GsmAudioTunnel *stream);

#ifdef __cplusplus
}
#endif

#endif
