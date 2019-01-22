# the compiler: g++ for C++
CXX = g++

# compiler flags:
CXXFLAGS = -std=c++17 -g -Wall
OPENCV   = -I/usr/include/opencv4/opencv2 -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
LDFLAGS  = -lm -lstdc++fs $(OPENCV)

TARGET = image2tiles

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(TARGET).cpp $(LDFLAGS)

clean:
	$(RM) $(TARGET)
