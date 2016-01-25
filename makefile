


thesis: get
	g++ -o thesis src/*.cpp

install: thesis
	mv ./thesis ./bin/thesis

run:
	./bin/thesis

get:
	git pull