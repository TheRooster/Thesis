
cflags = -I/usr/local/include/opencv -I/usr/local/include/opencv2 -L/usr/local/lib/ 
libs = -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching -lopencv_ximgproc



run: install
	./bin/thesis

calib:
	g++ -o camera_calib src/main.cpp  $(cflags) $(libs) 

install: thesis
	mv ./thesis ./bin/thesis

thesis: get
	g++ -o thesis src/stereo_calibrate.cpp  $(cflags) $(libs)

get:
	git pull

debug: get
	g++ -g -o thesis src/stereo_calibrate.cpp $(cflags) $(libs)