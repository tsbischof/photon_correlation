CC = clang
CXX = clang++
LD = clang++

CXXFLAGS = -Wall -std=c++0x -O3
LDFLAGS = -Wall
RM = rm -f

COMMON_OBJ = t2.o t3.o photon.o

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

all: ${TARGETS}

photons: ${PHOTONS_OBJ}
	${LD} ${LDFLAGS} ${PHOTONS_LIB} -o $@ $^

queue: ${QUEUE_OBJ}
	${LD} ${LDFLAGS} ${QUEUE_LIB} -o $@ $^

%o: %.cpp
	${CC} ${CCFLAGS} -o $@ $^

clean:
	${RM} ${OBJ} ${TARGETS}
