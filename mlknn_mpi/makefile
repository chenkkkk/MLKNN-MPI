all: program

program: DataStruct.o Similar.o MLKNN.o main.o
	mpicxx main.o -o program 

DataStruct.o:DataStruct.h Serializable.h
	mpicxx -c DataStruct.h 
Similar.o:Similar.h DataStruct.h
	mpicxx -c Similar.h
MLKNN.o:MLKNN.h Similar.h
	mpicxx -c MLKNN.h
main.o:MLKNN.h main.cpp
	mpicxx -c main.cpp
	
clean:
	rm main.o program MLKNN.h.gch DataStruct.h.gch Similar.h.gch