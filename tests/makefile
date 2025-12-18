#TARGET_MSVC:=-DNOT_MUSTACHE_TARGET_MSVC
TARGET_MSVC:=

INCL := -I ../src -I ../src/deps
C_STD := -std=c99
WERR_FLAGS := -Werror=int-conversion -Werror=pointer-arith  -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -Werror=return-type
GEN_FLAGS := -g $(WERR_FLAGS) ${C_STD}
BUILD_DIR := ./build

DEPS_SRC := ../src/deps/streql/strneql_x64_win.obj

base_test: base_test.c ../src/not_mustache.c ../src/not_mustache.h
	gcc $(GEN_FLAGS) $(INCL) base_test.c $(DEPS_SRC) ../src/not_mustache.c -o $(BUILD_DIR)/base_test.exe

nested_templates: nested_templates.c ../src/not_mustache.c ../src/not_mustache.h
	gcc $(GEN_FLAGS) $(INCL) nested_templates.c $(DEPS_SRC) ../src/not_mustache.c -o $(BUILD_DIR)/nested_test.exe


../bin/not_mustache.o: ../src/not_mustache.c ../src/not_mustache.h
	gcc -c $(GEN_FLAGS) $(INCL) $(DEPS_SRC) $(TARGET_MSVC) ../src/not_mustache.c -o ../bin/not_mustache.o

odin_test: test.odin ../bin/not_mustache.o
	odin build test.odin -file -out:$(BUILD_DIR)/test.exe -crt-mode:dynamic -linker:lld -collection:not_mustache_bin=../bin -o:none -debug

test_run:
	./base_test.exe

clean:
	rm ./*.exe

