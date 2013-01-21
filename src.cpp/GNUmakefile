CC = clang
CXX = clang++
LD = clang++

CXXFLAGS = -Wall -std=c++0x -O3
LDFLAGS = -Wall
RM = rm -f

LIB = -lboost_program_options

COMMON_OBJ = t2.o t3.o photon.o error.o options.o

PHOTONS_OBJ = ${COMMON_OBJ} photons_main.o
PHOTONS_LIB = 
OBJ += ${PHOTONS_OBJ}
LIB += ${PHOTONS_LIB}
TARGETS += photons

QUEUE_OBJ = ${COMMON_OBJ} queue_main.o
QUEUE_LIB = 
OBJ += ${QUEUE_OBJ}
LIB += ${QUEUE_LIB}
TARGETS += queue

GN_OBJ = ${COMMON_OBJ} gn_main.o histogram.o correlate.o
GN_LIB = 
OBJ += ${GN_OBJ}
LIB += ${GN_LIB}
TARGETS += gn



LDFLAGS += ${sort ${LIB}}

all: ${TARGETS}

photons: ${PHOTONS_OBJ}
	${LD} ${LDFLAGS} ${PHOTONS_LIB} -o $@ $^

queue: ${QUEUE_OBJ}
	${LD} ${LDFLAGS} ${QUEUE_LIB} -o $@ $^

gn: ${GN_OBJ}
	${LD} ${LDFLAGS} ${GN_LIB} -o $@ $^

%o: %.cpp
	${CC} ${CCFLAGS} -o $@ $^

clean:
	${RM} ${OBJ} ${TARGETS}
