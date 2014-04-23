#
# Copyright (c) 2011-2014, Thomas Bischof
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, 
#    this list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice, 
#    this list of conditions and the following disclaimer in the documentation 
#    and/or other materials provided with the distribution.
# 
# 3. Neither the name of the Massachusetts Institute of Technology nor the 
#    names of its contributors may be used to endorse or promote products 
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.
#

CC = clang
LD = clang 

INSTALL_DIR = ~

TEST = test
MKDIR = mkdir -p
CP = cp 

INCLUDES = 
LIBS = 

CCFLAGS := -Wall 
CCFLAGS += -D VERSION="3.4"
CCFLAGS += -O3 
CCFLAGS += ${INCLUDES}

LDFLAGS = -Wall ${LIBS}  

ALL_OBJ = 
ALL_LIB = 

COMMON_OBJ = error.o files.o options.o modes.o limits.o types.o \
		photon/t2.o photon/t3.o \
		photon/stream.o photon/window.o \
		run.o 

PHOTONS_OBJ = ${COMMON_OBJ} photons_main.o \
		photon/photons.o photon/stream.o photon/conversions.o
ALL_OBJ += ${PHOTONS_OBJ}
PHOTONS_LIB = -lm
ALL_LIB += ${PHOTONS_LIB}
TARGETS += photons

INTENSITY_OBJ = ${COMMON_OBJ} intensity_main.o \
		statistics/counts.o statistics/intensity.o
ALL_OBJ += ${INTENSITY_OBJ}
INTENSITY_LIB = 
ALL_LIB += ${INTENSITY_LIB}
TARGETS += photon_intensity

CORRELATE_OBJ = ${COMMON_OBJ} correlate_main.o \
		queue.o photon/queue.o correlate.o \
		correlation/correlator.o correlation/correlation.o \
		correlation/photon.o correlation/start_stop.o \
		combinatorics/combinations.o combinatorics/permutations.o \
		combinatorics/index_offsets.o \
		correlation/correlator_log.o
ALL_OBJ += ${CORRELATE_OBJ}
CORRELATE_LIB = -lm
ALL_LIB += ${CORRELATE_LIB}
TARGETS += photon_correlate

HISTOGRAM_OBJ = ${COMMON_OBJ} histogram_main.o \
		histogram.o \
		combinatorics/combinations.o combinatorics/permutations.o \
		combinatorics/index_offsets.o \
		correlation/correlation.o correlation/photon.o correlation/correlator.o\
		queue.o photon/queue.o \
		histogram/photon.o histogram/edges.o histogram/values_vector.o \
		histogram/histogram_gn.o histogram/photon.o 
ALL_OBJ += ${HISTOGRAM_OBJ}
HISTOGRAM_LIB = -lm
ALL_LIB += ${HISTOGRAM_LIB}
TARGETS += photon_histogram

BIN_INTENSITY_OBJ = ${COMMON_OBJ} bin_intensity_main.o \
		statistics/bin_intensity.o \
		${filter-out %main.o, ${sort ${HISTOGRAM_OBJ}}}
ALL_OBJ += ${BIN_INTENSITY_OBJ}
BIN_INTENSITY_LIB = -lm
ALL_LIB += ${BIN_INTENSITY_LIB}
TARGETS += photon_bin_intensity

PHOTON_TEMPER_OBJ = ${COMMON_OBJ} photon_temper_main.o \
		photon/temper.o queue.o photon/offsets.o
ALL_OBJ += ${PHOTON_TEMPER_OBJ}
PHOTON_TEMPER_LIB =
ALL_LIB += ${PHOTON_TEMPER_LIB}
TARGETS += photon_temper

GN_OBJ = gn_main.o ${COMMON_OBJ} \
		gn.o \
		${filter-out %main.o, ${sort ${CORRELATE_OBJ} ${HISTOGRAM_OBJ} \
				${INTENSITY_OBJ} ${PHOTON_NUMBER_OBJ}}} \
		statistics/bin_intensity.o
ALL_OBJ += ${GN_OBJ}
GN_LIB = ${sort ${CORRELATE_LIB} ${HISTOGRAM_LIB}}
ALL_LIB += ${GN_LIB}
TARGETS += photon_gn

PHOTON_NUMBER_OBJ = ${COMMON_OBJ} photon_number_main.o \
		statistics/number.o statistics/counts.o
ALL_OBJ += ${PHOTON_NUMBER_OBJ}
PHOTON_NUMBER_LIB = 
ALL_LIB += ${PHOTON_NUMBER_LIB}
TARGETS += photon_number

NUMBER_TO_CHANNELS_OBJ = ${COMMON_OBJ} number_to_channels_main.o \
		queue.o \
		statistics/number_to_channels.o
ALL_OBJ += ${NUMBER_TO_CHANNELS_OBJ}
NUMBER_TO_CHANNELS_LIB = 
ALL_LIB += ${NUMBER_TO_CHANNELS_LIB}
TARGETS += photon_number_to_channels

INTENSITY_CORRELATE_OBJ = ${COMMON_OBJ} intensity_correlate_main.o  \
		correlation/intensity.o statistics/intensity.o statistics/counts.o \
		correlation/multi_tau.o combinatorics/combinations.o
ALL_OBJ += ${INTENSITY_CORRELATE_OBJ}
INTENSITY_CORRELATE_LIB = 
ALL_LIB += ${INTENSITY_CORRELATE_LIB}
TARGETS += intensity_correlate

PHOTON_INTENSITY_CORRELATE_OBJ = photon_intensity_correlate_main.o  \
		photon_intensity_correlate.o \
		${filter-out %main.o, \
				${sort ${INTENSITY_CORRELATE_OBJ} ${INTENSITY_OBJ}}}
ALL_OBJ += ${PHOTON_INTENSITY_CORRELATE_OBJ}
PHOTON_INTENSITY_CORRELATE_LIB = ${sort \
				${INTENSITY_CORRELATE_LIB} ${INTENSITY_LIB}}
ALL_LIB += ${PHOTON_INTENSITY_CORRELATE_LIB}
TARGETS += photon_intensity_correlate

PHOTON_SYNCED_T2_OBJ = ${COMMON_OBJ} synced_t2_main.o photon/synced_t2.o queue.o
ALL_OBJ += ${PHOTON_SYNCED_T2_OBJ}
PHOTON_SYNCED_T2_LIB = -lm
ALL_LIB += ${PHOTON_SYNCED_T2_LIB}
TARGETS += photon_synced_t2

PHOTON_T3_WAIT_OBJ = ${COMMON_OBJ} t3_wait_main.o t3_wait/t3_wait.o
ALL_OBJ += ${PHOTON_T3_WAIT_OBJ}
PHOTON_T3_WAIT_LIB = 
ALL_LIB += ${PHOTON_T3_WAIT_LIB}
TARGETS += photon_t3_wait

# Now the heavy lifting: build the objects and include files for inclusion
# in the library.
LIB_TARGETS = 
LIBPC_OBJ = ${patsubst %.o, %.os, ${sort ${filter-out %main.o, ${ALL_OBJ}}}}
INCLUDES = ${patsubst %.o, %.h, ${LIBPC_OBJ}}
LIBPC_LDFLAGS = ${LDFLAGS} -shared
LIBPC_LIB = ${sort ${ALL_LIB}}
LIB_TARGETS += libphoton_correlation.so

BIN_DIR = ${INSTALL_DIR}/bin
LIB_DIR = ${INSTALL_DIR}/lib
INCLUDE_DIR = ${INSTALL_DIR}/include/photon_correlation

all: ${TARGETS} ${LIB_TARGETS}

install: ${TARGETS} ${LIB_TARGETS}
	${TEST} -d ${BIN_DIR} || ${MKDIR} ${BIN_DIR}
	${CP} ${TARGETS} ${BIN_DIR}
	${TEST} -d ${LIB_DIR} || ${MKDIR} ${LIB_DIR}
	${CP} ${LIB_TARGETS} ${LIB_DIR}
	${TEST} -d ${INCLUDE_DIR} || ${MKDIR} ${INCLUDE_DIR}
	${CP} --parents ${INCLUDES} ${INCLUDE_DIR}

uninstall: 
	${RM} ${addprefix ${INSTALL_DIR}, ${TARGETS}}

photons: ${PHOTONS_OBJ}
	${LD} ${LDFLAGS} ${PHOTONS_LIB} -o $@ $^

photon_correlate: ${CORRELATE_OBJ}
	${LD} ${LDFLAGS} ${CORRELATE_LIB} -o $@ $^

photon_histogram: ${HISTOGRAM_OBJ}
	${LD} ${LDFLAGS} ${HISTOGRAM_LIB} -o $@ $^

photon_intensity: ${INTENSITY_OBJ}
	${LD} ${LDFLAGS} ${INTENSITY_LIB} -o $@ $^ 

photon_bin_intensity: ${BIN_INTENSITY_OBJ}
	${LD} ${LDFLAGS} ${BIN_INTENSITY_LIB} -o $@ $^

photon_temper: ${PHOTON_TEMPER_OBJ}
	${LD} ${LDFLAGS} ${PHOTON_TEMPER_LIB} -o $@ $^ 

libphoton_correlation.so: ${LIBPC_OBJ}
	${LD} ${LIBPC_LDFLAGS} ${LIBPC_LIB} -o $@ $^

photon_gn: ${GN_OBJ}
	${LD} ${LDFLAGS} ${GN_LIB} -o $@ $^

photon_number: ${PHOTON_NUMBER_OBJ}
	${LD} ${LDFLAGS} ${PHOTON_NUMBER_LIB} -o $@ $^

photon_number_to_channels: ${NUMBER_TO_CHANNELS_OBJ}
	${LD} ${LDFLAGS} ${NUMBER_TO_CHANNELS_LIB} -o $@ $^

intensity_correlate: ${INTENSITY_CORRELATE_OBJ}
	${LD} ${LDFLAGS} ${INTENSITY_CORRELATE_LIB} -o $@ $^

photon_intensity_correlate: ${PHOTON_INTENSITY_CORRELATE_OBJ}
	${LD} ${LDFLAGS} ${PHOTON_INTENSITY_CORRELATE_LIB} -o $@ $^

photon_synced_t2: ${PHOTON_SYNCED_T2_OBJ}
	${LD} ${LDFLAGS} ${PHOTON_SYNCED_T2_LIB} -o $@ $^

photon_t3_wait: ${PHOTON_T3_WAIT_OBJ}
	${LD} ${LDFLAGS} ${PHOTON_T3_WAIT_LIB} -o $@ $^

%.o: %.c
	${CC} ${CCFLAGS} -c -o $@ $< 

%.os: %.c
	${CC} ${CCFLAGS} -fpic -c -o $@ $<

clean:
	${RM} ${sort ${TARGETS} ${ALL_OBJ} ${LIBPC_OBJ} ${LIB_TARGETS}}
