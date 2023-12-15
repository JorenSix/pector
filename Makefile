compile:
	gcc -c src/pector.c						-W -Wall -std=c11 -pedantic -O2
	gcc -c src/pector_reader.c				-W -Wall -std=c11 -pedantic -O2
	gcc -c src/pector_config.c				-W -Wall -std=c11 -pedantic -O2
	gcc -c src/pector_stream_processor.c	-W -Wall -std=c11 -pedantic -O2
	gcc -c src/pector_util.c				-W -Wall -std=c11 -pedantic -O2
	gcc -c src/pffft.c						-W -Wall -std=c11 -pedantic -O2
	mkdir -p bin
	gcc -o bin/pector_c *.o 				-lc -lm -ffast-math

web:
	emcc -o html/js/pector.js \
		-s ASSERTIONS=1 \
		-s ENVIRONMENT=shell \
		-s MODULARIZE=1  \
		-s SINGLE_FILE=1 \
		--bind \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s EXPORTED_FUNCTIONS="['_malloc','_free']" \
		-s EXPORTED_RUNTIME_METHODS='["cwrap"]' \
		src/pector_wasm.c \
		src/pffft.c \
		src/pector_config.c \
		src/pector_stream_processor.c \
		src/pector_reader.c \
		src/pector_util.c \
		-O3 -Wall -lm -lc -W -I. -ffast-math

clean:
	-rm -f *.o
	-rm -f bin/*


