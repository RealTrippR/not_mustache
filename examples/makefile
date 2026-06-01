INCL := -I ../src -I ../src/deps -I./
C_STD := -std=c99
WERR_FLAGS := -Werror=int-conversion -Werror=pointer-arith  -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -Werror=return-type

GEN_FLAGS := -g $(WERR_FLAGS) ${C_STD}

DEPS := ../src/deps/streql/strneql_x64_win.obj ../src/not_mustache.c

BDIR := ./bin


length:	length.c example_common.h 		../src/not_mustache.c ../src/not_mustache.h
	gcc $(GEN_FLAGS) $(INCL) $(DEPS) length.c -o $(BDIR)/length.exe

foreach: foreach.c example_common.h 		../src/not_mustache.c ../src/not_mustache.h
	gcc $(GEN_FLAGS) $(INCL) $(DEPS) foreach.c -o $(BDIR)/length.exe




../bin/not_mustache.o: ../src/not_mustache.c ../src/not_mustache.h
	gcc -c $(GEN_FLAGS) $(INCL) $(DEPS_SRC) ../src/not_mustache.c -o ../bin/not_mustache.o


clean:
	rm ./bin/*