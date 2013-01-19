CC = clang
CXX = clang++
LD = clang++

CXXFLAGS = -Wall -std=c++0x -O3
LDFLAGS = -Wall
RM = rm -f

OBJ = photon.o t2.o t3.o photons_main.o
TARGET = photons

all: ${TARGET}

${TARGET}: ${OBJ}
	${LD} ${LDFLAGS} -o $@ $^

%o: %.cpp
	${CC} ${CCFLAGS} -o $@ $^

clean:
	${RM} ${OBJ} ${TARGET}
