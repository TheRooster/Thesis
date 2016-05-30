
cflags =-I/opt/vc/include -I/usr/local/include/opencv -I/usr/local/include/opencv2 -L/usr/local/lib/ -L/opt/vc/lib -std=c++11 -DRPI_NO_X 
openCVlibs = -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching -lopencv_ximgproc
openGLlibs = -lGLESv2 -lEGL -lm -lbcm_host


all:
	g++ -o thesis src/stereo_calibrate.cpp  $(cflags) $(openCVlibs) $(openGLlibs)

run: install
	./bin/thesis

calib:
	g++ -o camera_calib src/stereo_calibrate.cpp  $(cflags) $(openCVlibs)

cvRect:
	g++ -o cv_rectify src/CVRectify.cpp $(cflags) $(openCVlibs)
glRect:
	g++ -o gl_rectify src/GLRectify.cpp src/Common/*.c $(cflags) $(openGLlibs)

install: thesis
	mv ./thesis ./bin/thesis

thesis: get
	g++ -o thesis src/stereo_calibrate.cpp  $(cflags) $(openCVlibs) $(openGLlibs)

get:
	git pull

debug: get
	g++ -g -o thesis src/stereo_calibrate.cpp $(cflags) $(openCVlibs) $(openGLlibs)
