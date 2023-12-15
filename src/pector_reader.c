
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "pector_config.h"
#include "pector_reader.h"

//have we reached the end of the file?
volatile bool end_of_file_reached;

struct Pector_Reader{
	//the olaf configuration
	Pector_Config* config;

	//The file currently being read
	FILE* audio_file;
    
    bool from_stdin;

	size_t total_samples_read;
};

Pector_Reader * pector_reader_new(Pector_Config * config,const char * source){

	Pector_Reader *reader = (Pector_Reader *) malloc(sizeof(Pector_Reader));
	reader->config = config;
	reader->total_samples_read = 0;

	FILE* file = NULL;
	if(source == NULL){
		file = freopen(NULL, "rb", stdin);
        reader->from_stdin = true;
	}else{
		file = fopen(source,"rb");  // r for read, b for binary
		if (file==NULL) {
			fprintf(stderr,"Audio file %s not found or unreadable.\n",source);
			exit(1);
		}
        reader->from_stdin = false;
	}
	
	reader->audio_file = file;

	return reader;
}

size_t pector_reader_read(Pector_Reader *reader ,float * audio_block){
	
	size_t number_of_samples_read;

	size_t step_size = reader->config->audio_step_size;
	size_t block_size = reader->config->audio_block_size;
	size_t overlap_size = block_size - step_size;

	//make room for the new samples: shift the oldest to the beginning
	for(size_t i = 0 ; i < overlap_size;i++){
		audio_block[i]=audio_block[i+step_size];
	}

	//start from the middle of the array
	float* startAddress = &audio_block[overlap_size];

	// copy the file into the audioData:
	number_of_samples_read = fread(startAddress,reader->config->bytes_per_audio_sample,step_size,reader->audio_file);

	//When reading the last buffer, make sure that the block is zero filled
	for(size_t i = number_of_samples_read ; i < step_size ;i++){
		audio_block[i] = 0;
	}
	
	if(feof(reader->audio_file)) {
		end_of_file_reached = true;
    }
	reader->total_samples_read+=number_of_samples_read;

	return number_of_samples_read;
}

size_t pector_reader_total_samples_read(Pector_Reader * reader){
	return reader->total_samples_read;
}

bool pector_reader_from_stdin(Pector_Reader * pector_reader){
    return pector_reader->from_stdin;
}

void pector_reader_destroy(Pector_Reader *  reader){

	if(!end_of_file_reached){
		fprintf(stderr, "Warning: did not reach end of file\n");
	}

	// After reading, close the file
	fclose(reader->audio_file);

	free(reader);
}
