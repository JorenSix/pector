/**
 * @file pector_stream_processor.h
 *
 * @brief Makes it more easy to process an audio stream.
 * 
 * The stream processor 
 *
 */

#ifndef PECTOR_STREAM_PROCESSOR_H
#define PECTOR_STREAM_PROCESSOR_H

    #include "pector_config.h"

    /**
     * Function pointer which is called when onset occurs.
     */
    typedef void (*Pector_Onset_Handler)(float, float, float,float,float,float);
    
    /**
     * @struct Pector_Stream_Processor
     * @brief An opaque struct with state information related to the stream processor.
     * 
     */
    typedef struct Pector_Stream_Processor Pector_Stream_Processor;

    /**
     * @brief      Initialize a new stream processor.
     *
     * @param[in]  raw_path   The path to the transcoded raw audio samples file 
     *
     * @return     Newly created state information related to the processor.
     */
    Pector_Stream_Processor * pector_stream_processor_new(Pector_Config * config,const char* raw_path,Pector_Onset_Handler onset_handler);



    void pector_stream_process_audio_block(Pector_Stream_Processor * processor,float * audio_data);

    /**
     * @brief      Process a file from the first to last audio sample.
     *
     * @param      pector_stream_processor  The olaf stream processor.
     */
    void pector_stream_processor_process(Pector_Stream_Processor * pector_stream_processor);

    /**
     * @brief      Prints onset info to stdout.
     *
     * @param[in]  onset_time_0  The onset time in seconds
     * @param[in]  onset_time_1  The onset time (in s), 1 onset ago.
     * @param[in]  onset_time_2  The onset time (in s), 2 onsets ago.
     * @param[in]  onset_time_3  The onset time (in s), 3 onsets ago.
     * @param[in]  loudness     The loudness in dbspl, of the current onset
     * @param[in]  onset_value  The onset value in complex onset value, of the current onsetÒ
     * 
     */
    void pector_print_onset_handler(float onset_time_0,float onset_time_1,float onset_time_2,float onset_time_3, float loudness, float onset_value);

    /**
     * @brief      Free up memory and release resources.
     *
     * @param      pector_stream_processor  The olaf stream processor.
     */
    void pector_stream_processor_destroy(Pector_Stream_Processor * pector_stream_processor);

#endif // PECTOR_STREAM_PROCESSOR_H
