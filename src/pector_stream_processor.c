#include <time.h>
#include <libgen.h>
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> 

#include "pffft.h"

#include "pector_stream_processor.h"
#include "pector_config.h"
#include "pector_reader.h"
#include "pector_util.h"

struct Pector_Stream_Processor{
	/**
	 * The configuration struct
	 */
	Pector_Config *config;

	/**
	 * The audio reader
	 */
	Pector_Reader *reader;

	/**
	 * Input audio samples
	 */
	float * audio_data;

	/**
	 * the (hamming) window to apply to the audio block
	 */
	float * window;

	/**
	 * The reusable fft struct
	 */
	PFFFT_Setup *fftSetup;
	/**
	 * The fft input data
	 */
	float *fft_in;
	/**
	 * The fft output, complex numbers
	 */
	float *fft_out;

	/**
	 * The audio block index
	 */
	size_t audio_block_index;

	/**
	 * Previous onset time, one onset ago.
	 */
	float onset_time_1;

	/**
	 * Onset time, two onsets ago.
	 */
	float onset_time_2;

	/**
	 * Onset time, three onets ago.
	 */
	float onset_time_3;

	/**
	 * The function pointer to the onset handler.
	 */
	Pector_Onset_Handler  onset_handler;

	/**
	 * Current magnitude
	 */
	float * magnitudes_0;

	/**
	 * Previous norm (power, magnitude) vector
	 */
	float * magnitudes_1;

	/**
	 * Current phases
	 */
	float * phases_0;

	/**
	 * Previous phase vector, one frame behind
	 */
	float * phases_1;

	/**
	 * Previous phase vector, two frames behind
	 */
	float * phases_2;
};


Pector_Stream_Processor * pector_stream_processor_new(Pector_Config * config, const char* raw_path,Pector_Onset_Handler  onset_handler ){
    
    Pector_Stream_Processor * processor = (Pector_Stream_Processor *) malloc(sizeof(Pector_Stream_Processor));
    
    if(onset_handler == NULL){
    	processor->onset_handler = &pector_print_onset_handler;
    }else{
    	processor->onset_handler = onset_handler;	
    }
    
	processor->config = config;
	processor->reader = pector_reader_new(processor->config,raw_path);
    processor->audio_data = (float *) calloc(processor->config->audio_block_size , sizeof(float)); //Input audio samples
    processor->window = (float *) calloc(processor->config->audio_block_size , sizeof(float)); //Input audio samples

    //cacluate an store the hamming window in window;
	pector_hamming_window(processor->window, config->audio_block_size);

	processor->fftSetup = pffft_new_setup(config->audio_block_size,PFFFT_REAL);
  	processor->fft_in = (float*) pffft_aligned_malloc(config->audio_block_size*4);//fft input
  	processor->fft_out= (float*) pffft_aligned_malloc(config->audio_block_size*4);//fft output

    processor->magnitudes_0 = (float *) calloc(processor->config->audio_block_size , sizeof(float));
    processor->magnitudes_1 = (float *) calloc(processor->config->audio_block_size , sizeof(float));
   
    processor->phases_0     = (float *) calloc(processor->config->audio_block_size , sizeof(float));
    processor->phases_1     = (float *) calloc(processor->config->audio_block_size , sizeof(float));
    processor->phases_2     = (float *) calloc(processor->config->audio_block_size , sizeof(float));
   
    processor->audio_block_index = 0;

    //onset times
    processor->onset_time_1 = 0;
    processor->onset_time_2 = 0;
    processor->onset_time_3 = 0;

	return processor;
}

void pector_stream_processor_destroy(Pector_Stream_Processor * processor){
	pector_reader_destroy(processor->reader);
    
	free(processor->magnitudes_0);
	free(processor->magnitudes_1);

	free(processor->phases_0);
	free(processor->phases_1);
    free(processor->phases_2);
    
	//cleanup fft structures
	pffft_aligned_free(processor->fft_in);
	pffft_aligned_free(processor->fft_out);
	pffft_destroy_setup(processor->fftSetup);

    free(processor->window);
	free(processor->audio_data);
    free(processor);
}

/**
 *
 * A complex Domain Method onset detection function
 * 
 * Christopher Duxbury, Mike E. Davies, and Mark B. Sandler. Complex domain
 * onset detection for musical signals. In Proceedings of the Digital Audio
 * Effects Conference, DAFx-03, pages 90-93, London, UK, 2003
 * 
 * The implementation is a translation of onset.c from Aubio, Copyright (C)
 * 2003-2009 Paul Brossier: piem@aubio.org
 * 
 * @author Joren Six
 * @author Paul Brossiers
 * 
 * @brief      process an audio block for percussive onsets
 *
 * @param      processor  The processor
 */
void pector_stream_process_audio_block(Pector_Stream_Processor * processor,float * audio_data){

	//printf("Called audio block process audio data %.8f %.8f \n",audio_data[0],audio_data[10]);


	//Keep the history of phase and magnitudes
	size_t binIndex = 0;
	for(binIndex = 0 ; binIndex < processor->config->audio_block_size / 2 ; binIndex++){
		processor->magnitudes_1[binIndex] = processor->magnitudes_0[binIndex];
		processor->phases_2[binIndex] = processor->phases_1[binIndex];
		processor->phases_1[binIndex] = processor->phases_0[binIndex];
	}

	// windowing + copy to fft input
	for(size_t j = 0 ; j <  processor->config->audio_block_size ; j++){
		processor->fft_in[j] = audio_data[j] * processor->window[j];
	}

	//do the transform and store the new phase and magnitude info
	pffft_transform_ordered(processor->fftSetup, processor->fft_in, processor->fft_out, 0, PFFFT_FORWARD);
	float * fft_out = processor-> fft_out;
	binIndex = 0;
	for(size_t j = 0 ; j < processor->config->audio_block_size ; j+=2){
		//complex numbers to magnitudes
		processor->magnitudes_0[binIndex] = sqrt(fft_out[j] * fft_out[j] + fft_out[j+1] * fft_out[j+1]);
		//complex numbers to phase info
		processor->phases_0[binIndex] = atan2(fft_out[j+1], fft_out[j]);
		binIndex++;
	}

	float onsetValue = 0;
	for(binIndex = 0 ; binIndex < processor->config->audio_block_size / 2 ; binIndex++){
		float dev1 = 2.0f * processor->phases_1[binIndex] - processor->phases_2[binIndex];

		onsetValue += sqrt(
			fabs(
				pow(processor->magnitudes_1[binIndex],2) + 
				pow(processor->magnitudes_0[binIndex],2) - 
				2.0 * 
				processor->magnitudes_1[binIndex] *
				processor->magnitudes_0[binIndex] * 
				cos(dev1 - processor->phases_0[binIndex])
			)
		);
	}

	if(onsetValue > processor->config->onset_threshold){
		float min_dbspl = processor->config->silence_threshold;
		float audio_block_duration_in_s = processor->config->audio_block_size / ((float) processor->config->audio_sample_rate);
		float onset_time_0 = processor->audio_block_index * audio_block_duration_in_s;
		float onset_delta = onset_time_0 - processor->onset_time_1;
		float dbspl = (float) pector_power_dbspl(audio_data,processor->config->audio_block_size);
		
		if( onset_delta > processor->config->minimum_inter_onset_time && dbspl > min_dbspl  ){
			processor->onset_handler(onset_time_0,processor->onset_time_1,processor->onset_time_2,processor->onset_time_3,dbspl,onsetValue);
			processor->onset_time_3=processor->onset_time_2;
			processor->onset_time_2=processor->onset_time_1;
			processor->onset_time_1=onset_time_0;
		}
	}
	
	//float dbspl = (float) pector_power_dbspl(audio_data,processor->config->audio_block_size);	
	//printf("onset value: %.3f, onset t: %.3f, silence t: %.3f, dbspl: %.4f\n",onsetValue,processor->config->onset_threshold,processor->config->silence_threshold,dbspl);
	//
	//increase the audio buffer counter
	processor->audio_block_index++;
}

void pector_print_onset_handler(float onset_time_0,float onset_time_1,float onset_time_2,float onset_time_3, float loudness, float onset_value){
	//calculate if it a double triple or quad onset
	float max_time_between_onsets = 0.45;//seconds
	float max_diff_between_onsets = 0.1;//seconds

	float delta_0_1 = onset_time_0 - onset_time_1;
	float delta_1_2 = onset_time_1 - onset_time_2;
	float delta_2_3 = onset_time_2 - onset_time_3;

	bool double_onset = delta_0_1 < max_time_between_onsets;
	printf("%.3f\n",delta_0_1);
	bool triple_onset = delta_0_1 < max_time_between_onsets && 
		delta_1_2 < max_time_between_onsets && 
		fabs(delta_0_1 - delta_1_2) < max_diff_between_onsets;
	
	bool quad_onset = delta_0_1 < max_time_between_onsets && 
		delta_1_2 < max_time_between_onsets && 
		delta_2_3 < max_time_between_onsets && 
		fabs(delta_0_1 - delta_1_2) < max_diff_between_onsets &&
		fabs(delta_0_1 - delta_2_3) < max_diff_between_onsets &&
		fabs(delta_1_2 - delta_2_3) < max_diff_between_onsets;
	
	printf("%.3f\t%.3f\t%.3f\t%.3f\t\t%.2f\t%.2f\t\t%d\t%d\t%d\n",onset_time_0,onset_time_1,onset_time_2,onset_time_3,loudness,onset_value,double_onset,triple_onset,quad_onset);
}

void pector_stream_processor_process(Pector_Stream_Processor * processor){

	size_t samples_read = pector_reader_read(processor->reader,processor->audio_data);
	size_t samples_expected = processor->config->audio_step_size;

	clock_t start, end;
    start = clock();
    
	while(samples_read==samples_expected){
		//read the next audio block
		samples_read = pector_reader_read(processor->reader,processor->audio_data);

		//process the full audio block
		pector_stream_process_audio_block(processor,processor->audio_data);

		//report some info for the streaming case
		if(processor->audio_block_index % 100 == 0 && pector_reader_from_stdin(processor->reader)){
			//double audio_duration = (double) pector_reader_total_samples_read(processor->reader) / (double) processor->config->audio_sample_rate;
			//fprintf(stderr,"Time: %.3fs \n",audio_duration);
		}
	}
	
	//handle the last samples? No :)
	double audio_duration = (double) pector_reader_total_samples_read(processor->reader) / (double) processor->config->audio_sample_rate;

	//for timing statistics
	end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    double ratio = audio_duration / cpu_time_used;
	
    fprintf(stderr,"Processed %.1fs in %.3fs (%.0f times realtime)\n", audio_duration,cpu_time_used,ratio);
}
