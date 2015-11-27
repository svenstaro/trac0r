.PHONY: web run default clean clang webrun

default: gcc

gcc:
	mkdir -p build
	cd build; CXX=g++ cmake ..; make -j

gcc-opencl:
	mkdir -p build
	cd build; CXX=g++ cmake -DOPENCL=1 ..; make -j

clang:
	mkdir -p build
	cd build; CXX=clang++ cmake ..; make -j

clang-opencl:
	mkdir -p build
	cd build; CXX=clang++ cmake -DOPENCL=1 ..; make -j

benchmark:
	mkdir -p build
	cd build; CXX=g++ cmake -DBENCHMARK=1 ..; make -j

memcheck: benchmark
	valgrind --leak-check=full build/trac0r_viewer

cachecheck: benchmark
	perf stat -r 5 -B -e cache-references,cache-misses build/trac0r_viewer

run: default
	build/trac0r_viewer

web:
	mkdir -p build-web
	cd build-web; /usr/lib/emscripten/emcmake cmake ..; make

webrun:
	emrun build-web/index.html

webpublish: web
	cp build-web/index.* ~/prj/trac0r-pages
	cd ~/prj/trac0r-pages; git commit -am "Update"; git push

clean:
	rm -rf build
	rm -rf build-web
