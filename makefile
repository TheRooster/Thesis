
cflags = -I/usr/local/include/opencv -I/usr/local/include/opencv2 -L/usr/local/lib/ 
libs = -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching



run: install
	./bin/thesis

install: thesis
	mv ./thesis ./bin/thesis

thesis: get
	g++ -o thesis src/stereo_match.cpp  $(cflags) $(libs)

get:
	git pull