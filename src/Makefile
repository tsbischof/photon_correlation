CP = cp -p
MKDIR = mkdir -p
TEST = test
CC = cc
LD = cc
CFLAGS = -Wall -std=c99
LDFLAGS = ${CFLAGS}
RM = rm -f
INSTALL_DIR = ~tsbischof/bin/

PICOQUANT_OBJ = picoquant.o error.o types.o \
		t2.o t3.o \
		picoharp.o picoharp/ph_v20.o \
		hydraharp.o hydraharp/hh_v10.o \
		timeharp.o timeharp/th_v20.o timeharp/th_v30.o \
				timeharp/th_v50.o timeharp/th_v60.o 
PICOQUANT_LIB = 
CORRELATE_OBJ = correlate.o error.o combinations.o files.o modes.o \
		correlate_t2.o \
		correlate_t3.o t3.o
CORRELATE_LIB = 
HISTOGRAM_OBJ = histogram.o error.o files.o modes.o combinations.o \
		histogram_gn.o \
		histogram_t2.o \
		histogram_t3.o t3.o 
HISTOGRAM_LIB = -lm
TARGETS = picoquant correlate histogram

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

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $< 

clean:
	${RM} ${TARGETS} ${PICOQUANT_OBJ} ${CORRELATE_OBJ} ${HISTOGRAM_OBJ}
