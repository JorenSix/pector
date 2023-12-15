#include <stdbool.h> //bool 
#include <stddef.h>

/**
 * @file pector_config.h
 *
 * @brief Pector configuration pramameters.
 *
 * Holds the compile time configuration parameters.
 *
 * To create a new config, use @ref pector_config_default.  To destroy a config, use
 * @ref pector_config_destroy.
 *
 */

#ifndef PECTOR_CONFIG_H
#define PECTOR_CONFIG_H
	
	typedef struct Pector_Config Pector_Config;

	/**
	 * @struct Pector_Config
	 * @brief Configuration parameters defining the behaviour of Pector.
	 * 
	 * Pector_Config configuration parameters define the behaviour of Olaf.
	 * The configuration settings are set at compile time and should not change
	 * while running the program. 
	 */
	struct Pector_Config{

		//------ Audio input configuration

		/** The size of a single audio block: e.g. 1024 samples*/
		size_t audio_block_size;
		
		/**The sample rate of the incoming audio: e.g. 16000Hz.*/
		size_t audio_sample_rate;

		/**The size of a step from one block of samples to the next: e.g. 128 samples.*/
		size_t audio_step_size;

		/**The size of a single audio sample. For example a 32 bit float sample consists of 4 bytes */
		size_t bytes_per_audio_sample;
		
	
		/** Print debug info. If true a lot of messages are printed to the console. */ 
		bool verbose;

		/**
		 * Minimum time between two accepted onsets, in seconds.
		 */
		float minimum_inter_onset_time;

		/**
		 * Minimum loudness for which an onset is accepted.
		 */
		float silence_threshold;

		/**
		 * Onset is accepted when this is exceeded.
		 */
		float onset_threshold;
	};

	/**
	 * The default configuration to use on traditional computers.
	 *  @return   A new configuration struct, or NULL if it was not possible to allocate the memory.
	 */
	Pector_Config* pector_config_default(void);

	Pector_Config* pector_config_web(void);

	Pector_Config* pector_config_ESP32(void);
	
	/**
	 * Free the memory used by the configuration struct
	 * @param config      The configuration struct to destroy.
	 */
	void pector_config_destroy(Pector_Config *config);

#endif // PECTOR_CONFIG_H

