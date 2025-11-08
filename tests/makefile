INCL := -I ../src -I ../src/deps
GEN_FLAGS := -g

DEPS_SRC := ../src/deps/streql/strneql_x64_win.obj

base_test: base_test.c
	gcc $(GEN_FLAGS) $(INCL) base_test.c $(DEPS_SRC) ../src/not_mustache.c -o base_test.exe

base_test_run:
	./base_test.exe

#build_cli: