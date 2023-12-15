#include <stdbool.h>

#include "pector_config.h"

/**
 * @file pector_reader.h
 *
 * @brief Reads audio samples from a file in blocks of samples with a certain overlap.
 *
 * The configuration determines the block sizes and overlap between blocks.
 */

#ifndef PECTOR_READER_H
#define PECTOR_READER_H
    

    /**
     * @struct Pector_Reader
     * @brief The state information. 
     * 
     * A struct to keep the internal state of the matcher hidden and should 
     * not be used in other places
     */
    typedef struct Pector_Reader Pector_Reader;

    /**
     * @brief      Create a new reader
     *
     * @param      config  The configuration
     * @param[in]  source  The path of the audio file. The audio file should be a raw mono file of a certain format.
     *
     * @return     A struct with internal state.
     */
    Pector_Reader * pector_reader_new(Pector_Config * config, const char * source);

    /**
     * @brief      Read an audio block with overlap.
     *
     * @param      pector_reader  The olaf reader
     * @param      block        An array to store the audio samples.
     *  
     * @return     The number of samples read.
     */
    size_t pector_reader_read(Pector_Reader * pector_reader,float * block);

    /**
     * @brief      Returns the number of samples read.
     *
     * @param      pector_reader  The olaf reader state.
     *
     * @return     The number of samples read in total.
     */
    size_t pector_reader_total_samples_read(Pector_Reader * pector_reader);

    /**
     * @brief      Returns true if the samples are read from STDIN.
     *
     * @param      pector_reader  The olaf reader state.
     *
     * @return     True  if the samples are read from STDIN.
     */
    bool pector_reader_from_stdin(Pector_Reader * pector_reader);

    /**
     * @brief      Free resources related to the reader.
     *
     * @param      pector_reader  The olaf reader state.
     */
    void pector_reader_destroy(Pector_Reader * pector_reader);


#endif // APS_DETECTOR_READER_H
