
run: install
	./bin/thesis

install: thesis
	mv ./thesis ./bin/thesis

thesis: get
	g++ -o thesis src/*.cpp

get:
	git pull