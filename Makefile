SRC_FILES = src/App.cpp \
			src/Notechart.cpp \
			src/Canvas.cpp

all: $(SRC_FILES)
	g++ -std=c++20 \
		-o build/main \
		$(SRC_FILES) \
		-Isrc/include \
		`wx-config --cxxflags --libs`;
	build/main;