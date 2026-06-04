# macOS CppUTest Makefile (separate C and C++)

# Compilers
CXX = g++
CC = gcc

# Homebrew paths (for Apple Silicon/Homebrew)
CPPUTEST_INC = -I/opt/homebrew/include
CPPUTEST_LIB = -L/opt/homebrew/lib -lCppUTest -lCppUTestExt

# Project include paths
PROJECT_INC = -I../Core/App -I./mocks -I../Drivers/drv_microphone/inc -I../Middlewares/Custom/Cellular/inc

# Build flags
CFLAGS = -Wall -g $(CPPUTEST_INC) $(PROJECT_INC)
CXXFLAGS = -Wall -g $(CPPUTEST_INC) $(PROJECT_INC)

# Targets
all: clean test

test:
	@echo "==> 1. Compiling C sources (gcc)"
	$(CC) $(CFLAGS) -c ../Core/App/audio_dsp.c -o audio_dsp.o
	$(CC) $(CFLAGS) -c ../Core/App/ai_app.c -o ai_app.o
	$(CC) $(CFLAGS) -c ../Core/App/mqtt.c -o mqtt.o
	$(CC) $(CFLAGS) -c ../Drivers/drv_microphone/src/microphone.c -o microphone.o
	$(CC) $(CFLAGS) -c ../Middlewares/Custom/Cellular/src/cellular.c -o cellular.o

	@echo "==> 2. Compiling C++ tests (g++)"
	$(CXX) $(CXXFLAGS) -c main_test.cpp -o main_test.o
	$(CXX) $(CXXFLAGS) -c test_audio_dsp.cpp -o test_audio_dsp.o
	$(CXX) $(CXXFLAGS) -c test_ai_app.cpp -o test_ai_app.o
	$(CXX) $(CXXFLAGS) -c test_mqtt.cpp -o test_mqtt.o
	$(CXX) $(CXXFLAGS) -c test_microphone.cpp -o test_microphone.o
	$(CXX) $(CXXFLAGS) -c test_cellular.cpp -o test_cellular.o

	@echo "==> 3. Linking"
	$(CXX) audio_dsp.o ai_app.o mqtt.o microphone.o cellular.o main_test.o test_audio_dsp.o test_ai_app.o test_mqtt.o test_microphone.o test_cellular.o $(CPPUTEST_LIB) -o run_tests

	@echo "==> 4. Running tests"
	./run_tests

clean:
	rm -f run_tests *.o