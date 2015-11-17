.PHONY: web run default clean clang webrun

default:
	mkdir -p build
	cd build; cmake ..; make -j

clang:
	mkdir -p build
	cd build; CXX=clang++ cmake ..; make -j

run:
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
