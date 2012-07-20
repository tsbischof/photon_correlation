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
CFLAGS += -DVERSION_MINOR=3
# For use while debugging.
#CFLAGS += -g -O0
CFLAGS += -O3 
CFLAGS += -fPIC

LDFLAGS = ${CFLAGS}
INSTALL_DIR = ${HOME}/bin/

TARGETS = 

UTIL = error.o types.o strings.o files.o options.o modes.o limits.o \
		t2.o t3.o 

PICOQUANT_OBJ = picoquant_main.o ${UTIL} \
		picoquant.o \
		picoharp.o picoharp/ph_v20.o \
		hydraharp.o hydraharp/hh_v10.o \
		timeharp.o timeharp/th_v20.o timeharp/th_v30.o \
				timeharp/th_v50.o timeharp/th_v60.o 
PICOQUANT_LIB = 
TARGETS += picoquant

CORRELATE_OBJ = correlate_main.o ${UTIL} \
		correlate.o \
		combinations.o \
		correlate_t2.o t2.o \
		correlate_t3.o t3.o
CORRELATE_LIB = 
TARGETS += correlate

HISTOGRAM_OBJ = histogram_main.o ${UTIL} \
		histogram.o \
		combinations.o \
		histogram_gn.o \
		histogram_t2.o \
		histogram_t3.o 
HISTOGRAM_LIB = -lm
TARGETS += histogram

INTENSITY_OBJ = intensity_main.o ${UTIL} \
		intensity.o \
		intensity_t2.o \
		intensity_t3.o
INTENSITY_LIB = 
TARGETS += intensity

BIN_INTENSITY_OBJ = bin_intensity_main.o ${UTIL} \
		bin_intensity.o \
		bin_intensity_t2.o t2.o \
		bin_intensity_t3.o t3.o \
		histogram_gn.o
BIN_INTENSITY_LIB = -lm
TARGETS += bin_intensity

LIB_DIR = ${HOME}/lib
INCLUDE_DIR = ${HOME}/include/picoquant
LIB_TARGETS = 
LIBPICOQUANT_OBJ = ${sort ${filter-out %main.o, \
		${PICOQUANT_OBJ} ${CORRELATE_OBJ} ${HISTOGRAM_OBJ} \
		${INTENSITY_OBJ} ${BIN_INTENSITY_OBJ}}}
INCLUDES = ${patsubst %.o, %.h, ${LIBPICOQUANT_OBJ}}
LIBPICOQUANT_FLAGS = ${LDFLAGS} -shared 
LIBPICOQUANT_LIB = ${sort \
		${PICQOUANT_LIB} ${CORRELATE_LIB} ${HISTOGRAM_LIB} \
		${INTENSITY_LIB} ${BIN_INTENSITY_LIB}}
LIB_TARGETS += libpicoquant.so

all: ${TARGETS} ${LIB_TARGETS}

install: ${TARGETS} ${LIB_TARGETS}
	${TEST} -d ${INSTALL_DIR} || ${MKDIR} ${INSTALL_DIR}
	${CP} ${TARGETS} ${INSTALL_DIR}
	${TEST} -d ${LIB_DIR} || ${MKDIR} ${LIB_DIR}
	${CP} ${LIB_TARGETS} ${LIB_DIR}
	${TEST} -d ${INCLUDE_DIR} || ${MKDIR} ${INCLUDE_DIR}
	${CP} --parents ${INCLUDES} ${INCLUDE_DIR}

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

libpicoquant.so: ${LIBPICOQUANT_OBJ}
	${LD} ${LIBPICOQUANT_FLAGS} -o $@ $^ ${LIBPICOQUANT_LIB}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $< 

clean:
	${RM} ${sort \
		${TARGETS} ${PICOQUANT_OBJ} ${CORRELATE_OBJ} ${HISTOGRAM_OBJ} \
		${INTENSITY_OBJ} ${BIN_INTENSITY_OBJ} \
		${LIB_TARGETS}}
