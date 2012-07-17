TARGET ?= linux32

# Set up the compilers and whatnot
ifeq (${TARGET}, linux32)
	MKDIR = mkdir -p
	RM = rm -f
	TEST = test
	CP = cp -p
	CC = cc
	LD = cc
endif
ifeq (${TARGET}, win32)
	MKDIR = mkdir
	RM = del /Q
	CP = copy
	CC = mingw32-gcc
	LD = mingw32-gcc
endif

CFLAGS = -Wall 
CFLAGS += -std=c99
# On 32-bit systems wtih large files, the file offset is 64 bits. This causes
# errors if not adjusted, so fix it here as needed. This should not (in 
# principle) affect 64-bit systems adversely, though if we ever get to 
# 128-bit filesystems we will need to switch this to 128. 
CFLAGS += -D_FILE_OFFSET_BITS=64
CFLAGS += -DVERSION_MAJOR=1
CFLAGS += -DVERSION_MINOR=2
# For use while debugging.
#CFLAGS += -g -O0
CFLAGS += -O3

LDFLAGS = ${CFLAGS}
INSTALL_DIR = ${HOME}/bin/

TARGETS = 

UTIL = error.o types.o strings.o files.o options.o modes.o limits.o \
		t2.o t3.o 

PICOQUANT_OBJ = picoquant.o ${UTIL} \
		picoharp.o picoharp/ph_v20.o \
		hydraharp.o hydraharp/hh_v10.o \
		timeharp.o timeharp/th_v20.o timeharp/th_v30.o \
				timeharp/th_v50.o timeharp/th_v60.o 
PICOQUANT_LIB = 
TARGETS += picoquant

CORRELATE_OBJ = correlate.o ${UTIL} \
		combinations.o \
		correlate_t2.o t2.o \
		correlate_t3.o t3.o
CORRELATE_LIB = 
TARGETS += correlate

HISTOGRAM_OBJ = histogram.o ${UTIL} \
		combinations.o \
		histogram_gn.o \
		histogram_t2.o t2.o \
		histogram_t3.o t3.o 
HISTOGRAM_LIB = -lm
TARGETS += histogram

INTENSITY_OBJ = intensity.o ${UTIL}
INTENSITY_LIB = 
TARGETS += intensity

BIN_INTENSITY_OBJ = bin_intensity.o ${UTIL} \
		bin_intensity_t2.o t2.o \
		bin_intensity_t3.o t3.o \
		histogram_gn.o
BIN_INTENSITY_LIB = -lm
TARGETS += bin_intensity

all: ${TARGETS}

install: ${TARGETS}
	${TEST} -d ${INSTALL_DIR} || ${MKDIR} ${INSTALL_DIR}
	${CP} ${TARGETS} ${INSTALL_DIR}

uninstall: 
	${RM} ${addprefix ${INSTALL_DIR}, ${TARGETS}}

picoquant: ${PICOQUANT_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${PICOQUANT_LIB}

correlate: ${CORRELATE_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${CORRELATE_LIB}

histogram: ${HISTOGRAM_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${HISTOGRAM_LIB}

intensity: ${INTENSITY_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${INTENSITY_LIB}

bin_intensity: ${BIN_INTENSITY_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${BIN_INTENSITY_LIB}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $< 

clean:
	${RM} ${TARGETS} ${PICOQUANT_OBJ} ${CORRELATE_OBJ} ${HISTOGRAM_OBJ} \
			${INTENSITY_OBJ} ${BIN_INTENSITY_OBJ}
