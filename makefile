
get:
	git pull

thesis:
	g++ -o thesis src/*.cpp

install: thesis
	mv ./thesis ./bin/thesis

run:
	./bin/thesis