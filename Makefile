INCL := -I src -I src/deps
GEN_FLAGS := -g

DEPS_SRC := src/deps/streql/strneql_x64_win.obj

# -DNOT_MUSTACHE_TARGET_MSVC -- if you want to target MSVC / Odin

bin/not_mustache.o: src/not_mustache.c src/not_mustache.h
	gcc -c $(GEN_FLAGS) $(INCL) $(DEPS_SRC) src/not_mustache.c -o bin/not_mustache.o
