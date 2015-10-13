.PHONY: web run default clean clang webrun

default:
	mkdir -p build
	cd build; cmake ..; make -j

clang:
	mkdir -p build
	cd build; CXX=clang++ cmake ..; make -j

run:
	build/trac0r

web:
	mkdir -p build-web
	cd build-web; /usr/lib/emscripten/emcmake cmake ..; make

webrun:
	cd build-web; python -m http.server

clean:
	rm -rf build
	rm -rf build-web
