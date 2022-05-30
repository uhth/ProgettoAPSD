
CC := g++
CCW := mingw-w64
CXXFLAGS := --std=c++11
CFLAGS := -I/usr/local/include/allegro5
LDFLAGS := $(shell pkg-config --libs allegro-5 allegro_primitives-5 allegro_color-5 )

MPI_COMPILE_FLAGS := $(shell mpicxx --showme:compile)
MPI_LINK_FLAGS := $(shell mpicxx --showme:link)

LDFLAGS += $(MPI_LINK_FLAGS)
CFLAGS += $(MPI_COMPILE_FLAGS)

SOURCE := main.cpp

all :
	$(CC) $(SOURCE) $(CXXFLAGS) $(CFLAGS) $(LDFLAGS) -o run

