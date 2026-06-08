CC := g++
CPPFLAGS := -Oz -march=native
INC := -I../library

SRC := src/server.cpp
OUT := server.out

all:
	${CC} ${CPPFLAGS} ${SRC} -o ${OUT}
