CC = clang
CXX = clang++
LD = clang++

CXXFLAGS = -Wall -std=c++0x -O3
LDFLAGS = -Wall
RM = rm -f

LIB = -lboost_program_options

COMMON_OBJ = t2.o t3.o photon.o error.o options.o limits.o 

PHOTONS_OBJ = ${COMMON_OBJ} photons_main.o 
PHOTONS_LIB = 
OBJ += ${PHOTONS_OBJ}
LIB += ${PHOTONS_LIB}
TARGETS += photons


LDFLAGS += ${sort ${LIB}}

all: ${TARGETS}

photons: ${PHOTONS_OBJ}
	${LD} ${LDFLAGS} ${PHOTONS_LIB} -o $@ $^

%o: %.cpp
	${CC} ${CCFLAGS} -o $@ $^

clean:
	${RM} ${OBJ} ${TARGETS}
