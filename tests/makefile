INCL := -I ../src -I ../src/deps
GEN_FLAGS := -g

DEPS_SRC := ../src/deps/streql/strneql_x64_win.obj

base_test: base_test.c ../src/not_mustache.c ../src/not_mustache.h
	gcc $(GEN_FLAGS) $(INCL) base_test.c $(DEPS_SRC) ../src/not_mustache.c -o base_test.exe


../bin/not_mustache.o: ../src/not_mustache.c ../src/not_mustache.h
	gcc -c $(GEN_FLAGS) $(INCL) $(DEPS_SRC) ../src/not_mustache.c -o ../bin/not_mustache.o

odin_test: test.odin ../bin/not_mustache.o
	odin build test.odin -file -out:test.exe -collection:not_mustache_bin=../bin -o:none -debug

test_run:
	./test.exe

clean:
	rm ./*.exe

