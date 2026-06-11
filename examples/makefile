
NM_DIR := ../not_mustache
NMI_DIR := ../not_mustache/inc/not_mustache


INCL := -I $(NM_DIR)/src -I $(NM_DIR)/deps -I./ -I $(NM_DIR)/inc
C_STD := -std=c99
WERR_FLAGS := -Werror=int-conversion -Werror=pointer-arith  -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -Werror=return-type

GEN_FLAGS := -g $(WERR_FLAGS) ${C_STD}

DEPS := $(NM_DIR)/deps/streql/strneql_x64_win.obj $(NM_DIR)/src/not_mustache.c

BDIR := ./bin

length:	length.c example_common.h 		$(NM_DIR)/src/not_mustache.c $(NMI_DIR)/not_mustache.h
	gcc $(GEN_FLAGS) $(INCL) $(DEPS) length.c -o $(BDIR)/length.exe

foreach: foreach.c example_common.h 		$(NM_DIR)/src/not_mustache.c $(NMI_DIR)/not_mustache.h
	gcc $(GEN_FLAGS) $(INCL) $(DEPS) foreach.c -o $(BDIR)/foreach.exe



engine_usage: engine_usage.c example_common.h 		$(NM_DIR)/src/not_mustache.c $(NMI_DIR)/not_mustache.h
	gcc $(GEN_FLAGS) $(INCL) $(DEPS) engine_usage.c -o $(BDIR)/engine_usage.exe

parse: parse.c example_common.h 		$(NM_DIR)/src/not_mustache.c $(NMI_DIR)/not_mustache.h
	gcc $(GEN_FLAGS) $(INCL) $(DEPS) parse.c -o $(BDIR)/parse.exe





../bin/not_mustache.o: $(NM_DIR)/src/not_mustache.c $(NMI_DIR)not_mustache.h
	gcc  $(GEN_FLAGS) $(INCL) $(DEPS_SRC) $(NMI_DIR)/src/not_mustache.c -c -o ../bin/not_mustache.o


clean:
	rm ./bin/*