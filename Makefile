# the compiler: g++ for C++
CXX = g++

# compiler flags:
CXXFLAGS = -std=c++17 -g -Wall
INC_DIR = -I/usr/include/opencv2
OPENCV   = -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
LDFLAGS  = -lm -lstdc++fs $(INC_DIR) $(OPENCV)

TARGET = image2tiles

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(TARGET).cpp $(LDFLAGS)

clean:
	$(RM) $(TARGET)
