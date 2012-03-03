CC = cc
LD = cc
CFLAGS = -Wall -std=c99
LDFLAGS = ${CFLAGS}
RM = rm -f
LIB =

PICOQUANT_OBJ = picoquant.o error.o types.o \
		picoharp.o picoharp/ph_v20.o \
		hydraharp.o hydraharp/hh_v10.o \
		timeharp.o timeharp/th_v20.o timeharp/th_v50.o timeharp/th_v60.o
CORRELATE_OBJ = correlate.o correlate_t2.o correlate_t3.o error.o \
		combinations.o files.o modes.o
HISTOGRAM_OBJ = histogram.o error.o histogram_t2.o histogram_t3.o \
		files.o modes.o
TARGETS = picoquant correlate histogram

all: ${TARGETS}

picoquant: ${PICOQUANT_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${LIB}

correlate: ${CORRELATE_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${LIB}

histogram: ${HISTOGRAM_OBJ}
	${LD} ${LDFLAGS} -o $@ $^ ${LIB}

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $< ${LIB}

clean:
	${RM} ${TARGETS} ${PICOQUANT_OBJ} ${CORRELATE_OBJ} ${HISTOGRAM_OBJ}
