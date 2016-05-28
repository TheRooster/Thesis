
cflags = -I/usr/local/include/opencv -I/usr/local/include/opencv2 -L/usr/local/lib/ 
openCVlibs = -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching -lopencv_ximgproc
openGLlibs = -lGLEW -lGL -lglut


all:
	g++ -o thesis src/stereo_calibrate.cpp  $(cflags) $(openCVlibs) $(openGLlibs)

run: install
	./bin/thesis

calib:
	g++ -o camera_calib src/stereo_calibrate.cpp  $(cflags) $(openCVlibs)

install: thesis
	mv ./thesis ./bin/thesis

thesis: get
	g++ -o thesis src/stereo_calibrate.cpp  $(cflags) $(openCVlibs) $(openGLlibs)

get:
	git pull

debug: get
	g++ -g -o thesis src/stereo_calibrate.cpp $(cflags) $(openCVlibs) $(openGLlibs)
