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
CFLAGS += -DVERSION_MINOR=5
# For use while debugging.
#CFLAGS += -g -O0
CFLAGS += -O3 
CFLAGS += -fPIC
CFLAGS += -lm
CFLAGS += -fopenmp

LDFLAGS = ${CFLAGS}
INSTALL_DIR = ${HOME}/bin/



# Define the targets and their properties.
TARGETS = 
ALL_OBJ = 
ALL_LIB = 

UTIL = error.o types.o strings.o files.o options.o modes.o limits.o \
		t2.o t3.o 

PICOQUANT_OBJ = picoquant_main.o ${UTIL} \
		picoquant.o \
		picoharp.o picoharp/ph_v20.o \
		hydraharp.o hydraharp/hh_v10.o \
		timeharp.o timeharp/th_v20.o timeharp/th_v30.o \
				timeharp/th_v50.o timeharp/th_v60.o 
ALL_OBJ += ${PICOQUANT_OBJ}
PICOQUANT_LIB = 
ALL_LIB += ${PICOQUANT_LIB}
TARGETS += picoquant

CORRELATE_OBJ = correlate_main.o ${UTIL} \
		correlate.o \
		combinations.o \
		correlate_t2.o \
		correlate_t3.o 
ALL_OBJ += ${CORRELATE_OBJ}
CORRELATE_LIB = 
ALL_LIB += ${CORRELATE_LIB}
TARGETS += correlate

HISTOGRAM_OBJ = histogram_main.o ${UTIL} \
		histogram.o \
		combinations.o \
		histogram_gn.o \
		histogram_t2.o \
		histogram_t3.o 
ALL_OBJ += ${HISTOGRAM_OBJ}
HISTOGRAM_LIB =
ALL_LIB += ${HISTOGRAM_LIB}
TARGETS += histogram

INTENSITY_OBJ = intensity_main.o ${UTIL} \
		intensity.o \
		intensity_t2.o \
		intensity_t3.o
ALL_OBJ += ${INTENSITY_OBJ}
INTENSITY_LIB = 
ALL_LIB += ${INTENSITY_LIB}
TARGETS += intensity

BIN_INTENSITY_OBJ = bin_intensity_main.o ${UTIL} \
		bin_intensity.o \
		bin_intensity_t2.o \
		bin_intensity_t3.o \
		histogram_gn.o
ALL_OBJ += ${BIN_INTENSITY_OBJ}
BIN_INTENSITY_LIB = 
ALL_LIB += ${BIN_INTENSITY_LIB}
TARGETS += bin_intensity

CHANNELS_OBJ = channels_main.o ${UTIL} \
		channels.o
ALL_OBJ += ${CHANNELS_OBJ}
CHANNELS_LIB =
ALL_LIB += ${CHANNELS_LIB}
TARGETS += channels

#GN_OBJ = gn_main.o ${UTIL}
#ALL_OBJ += ${GN_OBJ}
#GN_LIB = 
#ALL_LIB += ${GN_LIB}
#TARGETS += gn

#TIME_DEPENDENT_PL_OBJ = time_dependent_pl.o ${UTIL}
#ALL_OBJ += ${TIME_DEPENDENT_PL_{OBJ}
#TIME_DEPENDENT_PL_LIB = 
#ALL_LIB += ${TIME_DEPENDENT_PL_{LIB}
#TARGETS += time_dependent_pl

#TIME_DEPENDENT_GN_OBJ = time_dependent_gn.o ${UTIL}
#ALL_OBJ += ${TIME_DEPENDENT_GN_OBJ}
#TIME_DEPENDENT_GN_LIB = 
#ALL_LIB += ${TIME_DEPENDENT_GN_LIB}
#TARGETS += time_dependent_gn

T3_AS_T2_OBJ = t3_as_t2_main.o ${UTIL}
ALL_OBJ += ${T3_AS_T2_OBJ}
T3_AS_T2_LIB = 
ALL_LIB += ${T3_AS_T2_LIB}
T3_AS_T2_LIB = 
TARGETS += t3_as_t2

INTENSITY_TO_T2_OBJ = intensity_to_t2_main.o intensity_to_t2.o  ${UTIL}
ALL_OBJ += ${INTENSITY_TO_T2_OBJ}
INTENSITY_TO_T2_LIB = 
ALL_LIB += ${INTENSITY_TO_T2_LIB}
TARGETS += intensity_to_t2

CORRELATE_VECTOR_OBJ = correlate_vector_main.o correlate_vector.o ${UTIL}
ALL_OBJ += ${CORRELATE_VECTOR_OBJ}
CORRELATE_VECTOR_LIB = 
ALL_LIB += ${CORRELATE_VECTOR_LIB}
TARGETS += correlate_vector

# Now the heavy lifting: build the objects and include files for inclusion
# in the library.
LIB_DIR = ${HOME}/lib
INCLUDE_DIR = ${HOME}/include/picoquant
LIB_TARGETS = 
LIBPICOQUANT_OBJ = ${sort ${filter-out %main.o, ${ALL_OBJ}}}
INCLUDES = ${patsubst %.o, %.h, ${LIBPICOQUANT_OBJ}}
LIBPICOQUANT_FLAGS = ${LDFLAGS} -shared
LIBPICOQUANT_LIB = ${sort ${ALL_LIB}}
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

channels: ${CHANNELS_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${CHANNELS_LIB}

libpicoquant.so: ${LIBPICOQUANT_OBJ}
	${LD} ${LIBPICOQUANT_FLAGS} -o $@ $^ ${LIBPICOQUANT_LIB}

t3_as_t2: ${T3_AS_T2_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${T3_AS_T2_LIB}

intensity_to_t2: ${INTENSITY_TO_T2_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${INTENSITY_TO_T2_LIB}

correlate_vector: ${CORRELATE_VECTOR_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${CORRELATE_VECTOR_LIB}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $< 

clean:
	${RM} ${sort \
		${TARGETS} ${ALL_OBJ} $ ${LIB_TARGETS}}
