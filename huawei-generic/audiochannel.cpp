/**
 * Copyright (C) 2010 Eduardo José Tagle <ejtagle@tutopia.com>
 *
 * Since deeply inspired from portaudio dev port:
 * Copyright (C) 2009-2010 r3gis (http://www.r3gis.fr)
 * Copyright (C) 2008-2009 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include "audiochannel.h"

#define LOG_NDEBUG 0
#define LOG_TAG "RILAudioCh"
#include <utils/Log.h>

#include <system/audio.h>
#include <media/AudioRecord.h>
#include <media/AudioSystem.h>
#include <media/AudioTrack.h>

// ---- Android sound streaming ----

/* modemAudioOut: 
	Output an audio frame (160 samples) to the 3G audio port of the cell modem 
	
		data = Pointer to audio data to write
	
*/
static int modemAudioOut(struct GsmAudioTunnel* ctx,const void* data)
{
	if (ctx->fd == -1) 
		return 0;
		
	// Write audio to the 3G modem audio port in 320 bytes chunks... This is
	//	required by huawei modems...
	
	// Write audio chunk
	int res = write(ctx->fd, data, ctx->frame_size * (ctx->bits_per_sample/8));
	if (res < 0)
		return -1;

	// Write a 0 length frame to post data
	res = write(ctx->fd, data, 0);
	if (res < 0)
		return -1;
			
	return 0;
}

/* modemAudioIn:
	Input an audio frame (160 samples) from the 3G audio port of the cell modem 

		data = Pointer to buffer where data must be stored
*/
static int modemAudioIn(struct GsmAudioTunnel* ctx, void* data)
{
	int res = 0;
	if (ctx->fd == -1) 
		return 0;
	
		
	while (1) {
        res = read(ctx->fd, data, ctx->frame_size * (ctx->bits_per_sample/8));
        if (res == -1) {
		
            if (errno != EAGAIN && errno != EINTR) {
                // A real error, not something that trying again will fix
				break;
            }
        } else {
			break;
		}
    }

	/* Failure means 0 bytes */
	if (res < 0)
		res = 0;
		
	/* If some samples missing, complete them with silence */
	if (res < (int) (ctx->frame_size * (ctx->bits_per_sample/8) )) {
	
		/* Output silence */
		memset( (char*)data + res, 0, (ctx->frame_size * (ctx->bits_per_sample/8) - res));
	}

    return 0;
}

static void AndroidRecorderCallback(int event, void* userData, void* info)
{
	struct GsmAudioTunnel *ctx = (struct GsmAudioTunnel*) userData;
	android::AudioRecord::Buffer* uinfo = (android::AudioRecord::Buffer*) info;
	unsigned nsamples;
	void *input;

	if(!ctx || !uinfo)
		return;
	
	if (ctx->quit_flag)
		goto on_break;

	input = (void *) uinfo->raw;

	// Calculate number of total samples we've got
	nsamples = uinfo->frameCount + ctx->rec_buf_count;

	if (nsamples >= ctx->frame_size) {
	
		/* If buffer is not empty, combine the buffer with the just incoming
		 * samples, then call put_frame.
		 */
		if (ctx->rec_buf_count) {
		
			unsigned chunk_count = ctx->frame_size - ctx->rec_buf_count;
			memcpy( (char*)ctx->rec_buf + ctx->rec_buf_count * (ctx->bits_per_sample/8), input, chunk_count * (ctx->bits_per_sample/8));

			/* Send the audio to the modem */
			modemAudioOut(ctx, ctx->rec_buf);

			input = (char*) input + chunk_count * (ctx->bits_per_sample/8);
			nsamples -= ctx->frame_size;
			ctx->rec_buf_count = 0;
		}

		// Give all frames we have
		while (nsamples >= ctx->frame_size) {
		
			/* Send the audio to the modem */
			modemAudioOut(ctx, input);

			input = (char*) input + ctx->frame_size * (ctx->bits_per_sample/8);
			nsamples -= ctx->frame_size;
		}

		// Store the remaining samples into the buffer
		if (nsamples) {
			ctx->rec_buf_count = nsamples;
			memcpy(ctx->rec_buf, input, nsamples * (ctx->bits_per_sample/8));
		}

	} else {
		// Not enough samples, let's just store them in the buffer
		memcpy((char*)ctx->rec_buf + ctx->rec_buf_count * (ctx->bits_per_sample/8), input, uinfo->frameCount * (ctx->bits_per_sample/8));
		ctx->rec_buf_count += uinfo->frameCount;
	}
	
	return;

on_break:
	LOGD("Record thread stopped");
	ctx->rec_thread_exited = 1;
	return;
}

static void AndroidPlayerCallback( int event, void* userData, void* info)
{

	unsigned nsamples_req;
	void *output;	
	struct GsmAudioTunnel *ctx = (struct GsmAudioTunnel*) userData;
	android::AudioTrack::Buffer* uinfo = (android::AudioTrack::Buffer*) info;
	
	if (!ctx || !uinfo)
		return;

	if (ctx->quit_flag)
		goto on_break;
	
	nsamples_req = uinfo->frameCount;
	output = (void*) uinfo->raw;

	// Check if any buffered samples
	if (ctx->play_buf_count) {
	
		// samples buffered >= requested by sound device
		if (ctx->play_buf_count >= nsamples_req) {
		
			memcpy(output, ctx->play_buf, nsamples_req * (ctx->bits_per_sample/8));
			ctx->play_buf_count -= nsamples_req;
			
			memmove(ctx->play_buf, 
				(char*)ctx->play_buf + nsamples_req * (ctx->bits_per_sample/8), ctx->play_buf_count * (ctx->bits_per_sample/8));
			nsamples_req = 0;
			return;
		}

		// samples buffered < requested by sound device
		memcpy(output, ctx->play_buf,
						ctx->play_buf_count * (ctx->bits_per_sample/8));
		nsamples_req -= ctx->play_buf_count;
		output = (char*)output + ctx->play_buf_count * (ctx->bits_per_sample/8);
		ctx->play_buf_count = 0;
	}

	// Fill output buffer as requested in chunks
	while (nsamples_req) {
		if (nsamples_req >= ctx->frame_size) {

			/* get a frame from the modem */
			modemAudioIn(ctx, output);
				
			nsamples_req -= ctx->frame_size;
			output = (char*)output + ctx->frame_size * (ctx->bits_per_sample/8);
				
		} else {

			/* get a frame from the modem */
			modemAudioIn(ctx, ctx->play_buf);
				
			memcpy(output, ctx->play_buf,
							nsamples_req * (ctx->bits_per_sample/8));
			ctx->play_buf_count = ctx->frame_size - nsamples_req;
			memmove(ctx->play_buf,
							(char*)ctx->play_buf + nsamples_req * (ctx->bits_per_sample/8),
							ctx->play_buf_count * (ctx->bits_per_sample/8));
			nsamples_req = 0;
		}
	}

	return;

on_break:
	LOGD("Play thread stopped");
	ctx->play_thread_exited = 1;
	return;
}

 //AT^DDSETEX=2
 
int gsm_audio_tunnel_start(struct GsmAudioTunnel *ctx,const char* gsmvoicechannel,unsigned int sampling_rate,unsigned int frame_size,unsigned int bits_per_sample)
{
	struct termios newtio;
	int format = (bits_per_sample > 8) 
		? AUDIO_FORMAT_PCM_16_BIT 
		: AUDIO_FORMAT_PCM_8_BIT;
	
	/* If already running, dont do it again */
	if (ctx->running)
		return 0;

	memset(ctx,0,sizeof(struct GsmAudioTunnel));
	
	ctx->sampling_rate = sampling_rate;
	ctx->frame_size = frame_size;
	ctx->bits_per_sample = bits_per_sample;
	
	LOGD("Opening GSM voice channel '%s', sampling_rate:%u hz, frame_size:%u, bits_per_sample:%u  ...",
		gsmvoicechannel,sampling_rate,frame_size,bits_per_sample);
		
	// Open the device(com port) to be non-blocking (read will return immediately)
	ctx->fd = open(gsmvoicechannel, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	if (ctx->fd < 0) {
		LOGE("Could not open '%s'",gsmvoicechannel);
		return -1;
	}
	
	// Configure it to get data as raw as possible
	tcgetattr(ctx->fd, &newtio );
	newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR | IGNBRK | IGNCR | IXOFF;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;     
	newtio.c_cc[VMIN]=0;
	newtio.c_cc[VTIME]=5;	// You may want to tweak this; 5 = 1/2 second, 10 = 1 second
	tcsetattr(ctx->fd,TCSANOW, &newtio);

	LOGD("Creating streams....");
	ctx->rec_buf = malloc(ctx->frame_size * (ctx->bits_per_sample/8));
	if (!ctx->rec_buf) {
		close(ctx->fd);
		return -1;
	}

	ctx->play_buf = malloc(ctx->frame_size * (ctx->bits_per_sample/8));
	if (!ctx->play_buf) {
		free(ctx->rec_buf);
		close(ctx->fd);
		return -1;
	}

	// Compute buffer size
	size_t inputBuffSize = 0;
	android::AudioSystem::getInputBufferSize(
					ctx->sampling_rate, // Samples per second
					format,
					AUDIO_CHANNEL_IN_MONO, 
					&inputBuffSize);
					
	// We use 2* size of input buffer for ping pong use of record buffer.
	inputBuffSize = 2 * inputBuffSize;

	// Create audio record channel
	ctx->rec_strm = new android::AudioRecord();
	if(!ctx->rec_strm) {
		LOGE("fail to create audio record");
		free(ctx->play_buf);
		free(ctx->rec_buf);
		close(ctx->fd);
		return -1;
	}

	// Unmute microphone
	// android::AudioSystem::muteMicrophone(false);
	int create_result = ((android::AudioRecord*)ctx->rec_strm)->set(
					AUDIO_SOURCE_MIC,
					ctx->sampling_rate,
					format,
					AUDIO_CHANNEL_IN_MONO,
					inputBuffSize,
					0, 					//flags
					&AndroidRecorderCallback,
					(void *) ctx,
					(inputBuffSize / 2), // Notification frames
					false,
					0);

	if(create_result != android::NO_ERROR){
		LOGE("fail to check audio record : error code %d", create_result);
		delete ((android::AudioRecord*)ctx->rec_strm);
		free(ctx->play_buf);
		free(ctx->rec_buf);
		close(ctx->fd);
		return -1;
	}

	if(((android::AudioRecord*)ctx->rec_strm)->initCheck() != android::NO_ERROR) {
		LOGE("fail to check audio record : buffer size is : %d, error code : %d", inputBuffSize, ((android::AudioRecord*)ctx->rec_strm)->initCheck() );
		delete ((android::AudioRecord*)ctx->rec_strm);
		free(ctx->play_buf);
		free(ctx->rec_buf);
		close(ctx->fd);
		return -1;
	}

	// Create audio playback channel
	ctx->play_strm = new android::AudioTrack();
	if(!ctx->play_strm) {
		LOGE("Failed to create AudioTrack");
		delete ((android::AudioRecord*)ctx->rec_strm);
		free(ctx->play_buf);
		free(ctx->rec_buf);
		close(ctx->fd);
		return -1;
	}

	// android::AudioSystem::setMasterMute(false);
	create_result = ((android::AudioTrack*)ctx->play_strm)->set(
					AUDIO_STREAM_VOICE_CALL,
					ctx->sampling_rate, //this is sample rate in Hz (16000 Hz for example)
					format,
					AUDIO_CHANNEL_OUT_MONO, //For now this is mono (we expect 1)
					inputBuffSize,
					0, //flags
					&AndroidPlayerCallback,
					(void *) ctx,
					(inputBuffSize / 2),
					0,
					false,
					0);
					
	if(create_result != android::NO_ERROR){
		LOGE("fail to check audio record : error code %d", create_result);
		delete ((android::AudioTrack*)ctx->play_strm);
		delete ((android::AudioRecord*)ctx->rec_strm);
		free(ctx->play_buf);
		free(ctx->rec_buf);
		close(ctx->fd);
		return -1;
	}

	if(((android::AudioTrack*)ctx->play_strm)->initCheck() != android::NO_ERROR) {
		LOGE("fail to check audio playback : buffer size is : %d, error code : %d", inputBuffSize, ((android::AudioTrack*)ctx->play_strm)->initCheck() );
		delete ((android::AudioTrack*)ctx->play_strm);
		delete ((android::AudioRecord*)ctx->rec_strm);
		free(ctx->play_buf);
		free(ctx->rec_buf);
		close(ctx->fd);
		return -1;
	}
	
	/* Save the current audio routing setting, then switch it to earpiece. */
	// android::AudioSystem::getMode(&ctx->saved_audio_mode);
	// android::AudioSystem::getRouting(ctx->saved_audio_mode, &ctx->saved_audio_routing);
	// android::AudioSystem::setRouting(ctx->saved_audio_mode,
	//                      android::AudioSystem::ROUTE_EARPIECE,
	//                      android::AudioSystem::ROUTE_ALL);

	LOGD("Starting streaming...");
	
	if (ctx->play_strm) {
		((android::AudioTrack*)ctx->play_strm)->start();
	}

	if (ctx->rec_strm) {
		((android::AudioRecord*)ctx->rec_strm)->start();
	}

	LOGD("Done");
	
	ctx->running = 1;
	
	// OK, done
	return 0;
}

/* API: destroy ctx. */
int gsm_audio_tunnel_stop(struct GsmAudioTunnel *ctx)
{
	int i = 0;

	/* If not running, dont do it again */
	if (!ctx->running)
		return 0;

	
	LOGD("Will Stop ctx, wait for all audio callback clean");
	ctx->quit_flag = 1;
	for (i=0; !ctx->rec_thread_exited && i<100; ++i){
		usleep(100000);
	}
	for (i=0; !ctx->play_thread_exited && i<100; ++i){
		usleep(100000);
	}

	// After all sleep for 0.1 seconds since android device can be slow
	usleep(100000);

	LOGD("Stopping ctx..");
	if (ctx->rec_strm) {
		((android::AudioRecord*)ctx->rec_strm)->stop();
	}

	if (ctx->play_strm) {
		((android::AudioTrack*)ctx->play_strm)->stop();
	}

	// Restore the audio routing setting
	//      android::AudioSystem::setRouting(ctx->saved_audio_mode,
	//                      ctx->saved_audio_routing,
	//                      android::AudioSystem::ROUTE_ALL);


	LOGD("Closing streaming");

	if (ctx->play_strm) {
		delete ((android::AudioTrack*)ctx->play_strm);
		ctx->play_strm = NULL;
	}

	if (ctx->rec_strm) {
		delete ((android::AudioRecord*)ctx->rec_strm);
		ctx->rec_strm = NULL;
	}

	free(ctx->play_buf);
	free(ctx->rec_buf);
	
	close(ctx->fd);
	memset(ctx,0,sizeof(struct GsmAudioTunnel));

	LOGD("Done");
	return 0;
}

