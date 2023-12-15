/** @file pector.c
 * @brief pector
 *
 * @mainpage Percussive signals detECTOR: pector
 *
 * @section intro_sec Introduction
 * Pector is a C application / library for detecting percussive sounds in a signal * 
 * 
 * @section configuration Pector's configuration
 * 
 * The configuration of pector is set at compile time.   
 * 
 * @secreflist
 * 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "pector_stream_processor.h"


void pector_print_help(const char* message){
    fprintf(stderr,"%s",message);
    fprintf(stderr,"\tpector_c [audio.raw |on clap] \n");
    f
    fprintf(stderr,"\t\tIf a name of a raw file is given, it is analyzed. A raw audio file should contain mono f32 samples\n");
    fprintf(stderr,"\t\tIf nothing is given, samples are read from STDIN \n");
    printf(stderr,"\t\tIf on clap is given, the percussion detector waits for a double clap and then exits.\n");
    exit(-10);
}

void pector_exit_after_double_clap_handler(float onset_time_0,float onset_time_1,float onset_time_2,float onset_time_3, float loudness, float onset_value){
    //calculate if it a double triple or quad onset
    float max_time_between_onsets = 0.45;//seconds
    float delta_0_1 = onset_time_0 - onset_time_1;
   
    bool double_onset = delta_0_1 < max_time_between_onsets;
    //printf("%.3f\n",delta_0_1);
    (void) onset_time_3;
    (void) onset_time_2;
    (void) loudness;
    (void) onset_value;

    if(double_onset)
        exit(0);
}

int main(int argc, const char* argv[]){
    
    Pector_Config* config = pector_config_default();
    Pector_Stream_Processor* processor;
    if(argc == 1){
        //fprintf(stdout,"Running pector, reading samples from STDIN \n");
        processor = pector_stream_processor_new(config,NULL,NULL);
        pector_stream_processor_process(processor);
        pector_stream_processor_destroy(processor);
    }else if (argc == 2){
        //fprintf(stdout,"Running pector, reading samples from %s \n",argv[1]);
        processor = pector_stream_processor_new(config,argv[1],NULL);
        pector_stream_processor_process(processor);
        pector_stream_processor_destroy(processor);
    }else if (argc == 3){
        processor = pector_stream_processor_new(config,NULL,pector_exit_after_double_clap_handler);
        pector_stream_processor_process(processor);
        pector_stream_processor_destroy(processor);
    }else{
        pector_print_help("Invalid number of command line arguments.");
    }

    return 0;
}
