INCL := -I ../mustache -I ../mustache/deps
GEN_FLAGS := -g -std=c99

DEPS_SRC := ../mustache/deps/streql/strneql_x64_win.obj

base_test: base_test.c
	gcc $(GEN_FLAGS) $(INCL) base_test.c $(DEPS_SRC) ../mustache/mustache.c -o base_test.exe

base_test_run:
	./base_test.exe

#build_cli: