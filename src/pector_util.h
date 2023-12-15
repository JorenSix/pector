#include <stdint.h>
#include "pector_config.h"
/**
 * @file pector_power.h
 *
 * @briefCalculates the power of a block of audio samples (in dB)
 *
 * The configuration determines the block sizes.
 */

#ifndef PECTOR_UTIL_H
#define PECTOR_UTIL_H
    /**
     * @brief      Calculate the power in an audio block.
     *
     * @param      audio_block       The block of audio to handle.
     * @param      audio_block_size  The size of the block to handle.
     *
     * @return     The power in db SPL
     */
    double pector_power_dbspl(float * audio_block, size_t audio_block_size);

    /**
     * @brief      Converts the audio block index time to seconds
     *
     * @param      config             The configuration
     * @param[in]  audio_block_index  The audio block index
     *
     * @return     The time in seconds.
     */
    double pector_index_to_seconds(Pector_Config * config, size_t audio_block_index);
    

    /**
     * @brief      Calculates a hamming window of the requested size.
     *
     * @param      window       The window
     * @param[in]  window_size  The window size
     */
    void pector_hamming_window(float* window, size_t window_size);


    /**
     * @brief      Calculates the power of a single frequency in a signal.
     *             See "Goertzel algorithm generalized to non-integer multiples of fundamental frequency" by Petr Sysel and Pavel Rajmic
     *
     * @param[in]  frequency_to_detect         The frequency to detect in Hz
     * @param[in]  audio_sample_rate           The audio sample rate in Hz
     * @param      audio_block                 The audio block with audio samples in 32bit floats
     * @param      window                      The window a window to apply to the audio samples
     * @param[in]  audio_block_size            The audio block size 
     *
     * @return     The power of the requested frequency
     */
    float pector_detect_frequency(float frequency_to_detect,float audio_sample_rate, float * audio_block, float * window, size_t audio_block_size);
#endif
