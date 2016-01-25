
run: install
	./bin/thesis

install: thesis
	mv ./thesis ./bin/thesis

thesis: get
	g++ -o thesis -lOpenCV src/*.cpp

get:
	git pull