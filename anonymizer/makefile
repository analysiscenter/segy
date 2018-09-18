Debug : all
all : anonymizer
anonymizer : main.o anonymizer.o shift_geo.o path_handler.o
	g++ -o anonymizer main.o anonymizer.o shift_geo.o path_handler.o

main.o : main.cpp
	g++ -std=c++17 -c main.cpp -o main.o

anonymizer.o : anonymizer.cpp
	g++ -std=c++17 -c anonymizer.cpp -o anonymizer.o

shift_geo.o : shift_geo.cpp
	g++ -std=c++17 -c shift_geo.cpp -o shift_geo.o

path_handler.o : path_handler.cpp
	g++ -std=c++17 -c path_handler.cpp -o path_handler.o