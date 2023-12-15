#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten.h>
#include <math.h>

#include "pffft.h"

#include "pector_util.h"
#include "pector_config.h"
#include "pector_stream_processor.h"
#include "pector_reader.h"

struct Pector_State{
	float * audio;
	
	size_t audio_block_index;

	size_t audio_sample_index;

	Pector_Config *config;

	Pector_Stream_Processor* processor;
};


void pector_onset_callback_js(float onset_time_0,float onset_time_1,float onset_time_2,float onset_time_3, float loudness, float onset_value) {
	printf("%.3f\n",onset_time_0);
	EM_ASM_ARGS({ pector_onset_callback($0, $1, $2, $3, $4, $5, $6); },onset_time_0,onset_time_1,onset_time_2,onset_time_3, loudness, onset_value); 
}

//the static keyword ensures that
//state is maintained between subsequent
//calls
static struct Pector_State state;

/**
 * @brief      Detect onset in an audio buffer
 *
 * @param      audio_buffer       The audio buffer
 * @param[in]  audio_buffer_size  The audio buffer size
 * @param[in]  samplerate         The audio samplerate
 * @param[in]  onset_threshold    The onset threshold
 * @param[in]  silence_threshold  The silence threshold
 *
 * @return     the audio block, main return is via a callback.
 */
int EMSCRIPTEN_KEEPALIVE pector_onset_detection(float * audio_buffer, size_t audio_buffer_size,float samplerate,float onset_threshold, float silence_threshold){

	if(state.config == NULL){
		//Get the default configuration
		state.config = pector_config_web();
		state.config->audio_block_size = audio_buffer_size;
		state.config->audio_step_size = audio_buffer_size;
		state.config->audio_sample_rate = (uint32_t) samplerate;

		state.audio_block_index = 0;

		state.audio = (float *) calloc(audio_buffer_size, sizeof(float));

		state.processor = pector_stream_processor_new(state.config,NULL,&pector_onset_callback_js);
	}

	//printf("Called onset detection %d \n",state.audio_block_index);
	//printf("wa buffer  %.8f %.8f \n",audio_buffer[0],audio_buffer[10]);

	//potentially changing parameters
	state.config->silence_threshold = silence_threshold;
	state.config->onset_threshold = onset_threshold;
	
	//add the new samples
	for(size_t i = 0 ; i < audio_buffer_size;i++){
		state.audio[state.audio_sample_index] = audio_buffer[i];
		state.audio_sample_index++;

		if(state.audio_sample_index == (size_t) state.config->audio_block_size){
			
			//process full audio block
			pector_stream_process_audio_block(state.processor,state.audio);

			state.audio_sample_index = 0;

			state.audio_block_index++;
		}
	}

	return state.audio_block_index;
}