#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#include "pector_config.h"

Pector_Config* pector_config_default(void){
	Pector_Config *config = (Pector_Config *) malloc(sizeof(Pector_Config));

	//audio info
	config->audio_block_size = 512;
	config->audio_step_size = 512;
	config->audio_sample_rate = 44100;

	config->bytes_per_audio_sample = 4; //32 bits float

	config->minimum_inter_onset_time = 0.1;//seconds
	config->silence_threshold = -15;

	config->onset_threshold = 90.0;

	//matcher configuration

	return config;
}

Pector_Config* pector_config_web(void){
	Pector_Config *config = (Pector_Config *) malloc(sizeof(Pector_Config));

	//audio info
	config->audio_block_size = 128;
	config->audio_step_size = 128;
	config->audio_sample_rate = 44100;

	config->bytes_per_audio_sample = 4; //32 bits float

	config->minimum_inter_onset_time = 0.1;//seconds
	
	config->silence_threshold = -10;

	config->onset_threshold = 90.0;

	//matcher configuration

	return config;
}

Pector_Config* pector_config_ESP32(void){
	Pector_Config *config = (Pector_Config *) malloc(sizeof(Pector_Config));

	//audio info
	config->audio_block_size = 512;
	config->audio_step_size = 512;
	config->audio_sample_rate = 16000;

	config->bytes_per_audio_sample = 4; //32 bits float

	config->minimum_inter_onset_time = 0.1;//seconds
	
	config->silence_threshold = -10;

	config->onset_threshold = 900.0;

	//matcher configuration

	return config;
}

void pector_config_destroy(Pector_Config * config){
	free(config);
}
