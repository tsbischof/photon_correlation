CC = clang
LD = clang

INCLUDES = 
LIBS = 

CCFLAGS := -Wall 
# On 32-bit systems wtih large files, the file offset is 64 bits. This causes
# errors if not adjusted, so fix it here as needed. This should not (in 
# principle) affect 64-bit systems adversely, though if we ever get to 
# 128-bit filesystems we will need to switch this to 128. 
CCFLAGS += -D _FILE_OFFSET_BITS=64 
CCFLAGS += -D VERSION="2.0"
CCFLAGS += -O3 
CCFLAGS += ${INCLUDES}

LDFLAGS = -Wall ${LIBS}  

ALL_OBJ = 
ALL_LIB = 

COMMON_OBJ = error.o files.o options.o modes.o limits.o types.o \
		photon/t2.o photon/t3.o \
		photon/t2_void.o photon/t3_void.o \
		photon/window.o photon/queue.o photon/stream.o \
		run.o \
		vector/vector.o

PHOTONS_OBJ = ${COMMON_OBJ} photons_main.o \
		photon/photons.o photon/stream.o photon/conversions.o
ALL_OBJ += ${PHOTONS_OBJ}
PHOTONS_LIB = -lm
ALL_LIB += ${PHOTONS_LIB}
TARGETS += photons

CORRELATE_OBJ = ${COMMON_OBJ} correlate_main.o \
		correlate.o correlate_photon.o \
		combinations.o permutations.o \
		correlate_start_stop.o
ALL_OBJ += ${CORRELATE_OBJ}
CORRELATE_LIB = 
ALL_LIB += ${CORRELATE_LIB}
TARGETS += correlate

CORRELATIONS_OBJ = ${COMMON_OBJ} correlations_main.o \
		correlate_photon.o \
		combinations.o permutations.o
ALL_OBJ += ${CORRELATIONS_OBJ}
CORRELATIONS_LIB = 
ALL_LIB += ${CORRELATIONS_LIB}
TARGETS += correlations

HISTOGRAM_OBJ = ${COMMON_OBJ} histogram_main.o \
		histogram.o combinations.o permutations.o correlate_photon.o \
		histogram_photon.o 
ALL_OBJ += ${HISTOGRAM_OBJ}
HISTOGRAM_LIB = -lm
ALL_LIB += ${HISTOGRAM_LIB}
TARGETS += histogram

INTENSITY_OBJ = ${COMMON_OBJ} intensity_main.o \
		statistics/counts.o statistics/intensity.o
ALL_OBJ += ${INTENSITY_OBJ}
INTENSITY_LIB = 
ALL_LIB += ${INTENSITY_LIB}
TARGETS += intensity

BIN_INTENSITY_OBJ = ${COMMON_OBJ} bin_intensity_main.o \
		bin_intensity.o bin_intensity_photon.o \
		${filter-out %main.o, ${HISTOGRAM_OBJ}}
#ALL_OBJ += ${BIN_INTENSITY_OBJ}
#BIN_INTENSITY_LIB = -lm
#ALL_LIB += ${BIN_INTENSITY_LIB}
#TARGETS += bin_intensity

CHANNELS_OBJ = ${COMMON_OBJ} channels_main.o \
		channels.o channels_photon.o
ALL_OBJ += ${CHANNELS_OBJ}
CHANNELS_LIB =
ALL_LIB += ${CHANNELS_LIB}
TARGETS += channels

GN_OBJ = gn_main.o ${COMMON_OBJ} \
		gn.o \
		${filter-out %main.o, ${sort ${CORRELATE_OBJ} ${HISTOGRAM_OBJ}}}
#ALL_OBJ += ${GN_OBJ}
GN_LIB = ${sort ${CORRELATE_LIB} ${HISTOGRAM_LIB}}
ALL_LIB += ${GN_LIB}
#TARGETS += gn

INTENSITY_TO_T2_OBJ = ${COMMON_OBJ} intensity_to_t2_main.o intensity_to_t2.o
ALL_OBJ += ${INTENSITY_TO_T2_OBJ}
INTENSITY_TO_T2_LIB = 
ALL_LIB += ${INTENSITY_TO_T2_LIB}
TARGETS += intensity_to_t2

PHOTON_NUMBER_OBJ = ${COMMON_OBJ} photon_number_main.o \
		statistics/number.o statistics/counts.o
ALL_OBJ += ${PHOTON_NUMBER_OBJ}
PHOTON_NUMBER_LIB = 
ALL_LIB += ${PHOTON_NUMBER_LIB}
TARGETS += photon_number

CORRELATE_VECTOR_OBJ = correlate_vector_main.o ${COMMON_OBJ} \
		correlate_vector.o \
		intensity.o intensity_t2.o intensity_t3.o \
		histogram_gn.o \
		vector_g2.o
#ALL_OBJ += ${CORRELATE_VECTOR_OBJ}
CORRELATE_VECTOR_LIB = 
#ALL_LIB += ${CORRELATE_VECTOR_LIB}
#TARGETS += correlate_vector

COMBINATIONS_OBJ = ${COMMON_OBJ} combinations_main.o \
		combinations.o permutations.o
ALL_OBJ += ${COMBINATIONS_OBJ}
COMBINATIONS_LIB = 
ALL_LIB += ${COMBINATIONS_LIB}
TARGETS += combinations

# Now the heavy lifting: build the objects and include files for inclusion
# in the library.
LIB_TARGETS = 
LIBPC_OBJ = ${patsubst %.o, %.os, ${sort ${filter-out %main.o, ${ALL_OBJ}}}}
INCLUDES = ${patsubst %.o, %.h, ${LIBPC_OBJ}}
LIBPC_LDFLAGS = ${LDFLAGS} -shared
LIBPC_LIB = ${sort ${ALL_LIB}}
#LIB_TARGETS += libphoton_correlation.so

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

photons: ${PHOTONS_OBJ}
	${LD} ${LDFLAGS} ${PHOTONS_LIB} -o $@ $^

correlate: ${CORRELATE_OBJ}
	${LD} ${LDFLAGS} ${CORRELATE_LIB} -o $@ $^

correlations: ${CORRELATIONS_OBJ}
	${LD} ${LDFLAGS} ${CORRELATIONS_LIB} -o $@ $^

histogram: ${HISTOGRAM_OBJ}
	${LD} ${LDFLAGS} ${HISTOGRAM_LIB} -o $@ $^

intensity: ${INTENSITY_OBJ}
	${LD} ${LDFLAGS} ${INTENSITY_LIB} -o $@ $^ 

bin_intensity: ${BIN_INTENSITY_OBJ}
	${LD} ${LDFLAGS} ${BIN_INTENSITY_LIB} -o $@ $^

channels: ${CHANNELS_OBJ}
	${LD} ${LDFLAGS} ${CHANNELS_LIB} -o $@ $^ 

libphoton_correlation.so: ${LIBPC_OBJ}
	${LD} ${LIBPC_LDFLAGS} ${LIBPC_LIB} -o $@ $^

intensity_to_t2: ${INTENSITY_TO_T2_OBJ}
	${LD} ${LDFLAGS} ${INTENSITY_TO_T2_LIB} -o $@ $^

correlate_vector: ${CORRELATE_VECTOR_OBJ}
	${LD} ${LDFLAGS} ${CORRELATE_VECTOR_LIB} -o $@ $^

gn: ${GN_OBJ}
	${LD} ${LDFLAGS} ${GN_LIB} -o $@ $^

combinations: ${COMBINATIONS_OBJ}
	${LD} ${LDFLAGS} ${COMBINATIONS_LIB} -o $@ $^

photon_number: ${PHOTON_NUMBER_OBJ}
	${LD} ${LDFLAGS} ${PHOTON_NUMBER_LIB} -o $@ $^

%.o: %.c
	${CC} ${CCFLAGS} -c -o $@ $< 

%.os: %.c
	${CC} ${CCFLAGS} -fpic -c -o $@ $<

clean:
	${RM} ${sort ${TARGETS} ${ALL_OBJ} ${LIBPC_OBJ} ${LIB_TARGETS}}
