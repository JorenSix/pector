#include <math.h>
#include <stddef.h>
#include <complex.h>

#include "pector_util.h"

double pector_power_dbspl(float * audio_block, size_t audio_block_size){
	double rms = 0.0;
	double db_spl;
	
	for (size_t i = 0; i < audio_block_size; i++) {
		rms += audio_block[i] * audio_block[i];
	}

	rms = rms / ((double) audio_block_size);
	rms = sqrt(rms);

	db_spl = 20.0 * log10(rms);
	
	return db_spl;
}


void pector_hamming_window(float* window, size_t window_size) {
    size_t i;

    for (i = 0; i < window_size; ++i) {
        window[i] = 0.54 - 0.46 * cos(2 * M_PI * i / (window_size - 1));
    }
}

