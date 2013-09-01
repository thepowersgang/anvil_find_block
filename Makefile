
CFLAGS := -Wall -std=gnu99 -g
LDFLAGS := -g
LIBS := -lz

BIN := blockfind_anvil
OBJ := main.o anvil.o compression.o nbt.o

OBJ := $(OBJ:%=obj/%)
DEP := $(OBJ:%.o=%.d)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $(BIN) $(OBJ) $(LIBS)

obj/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $< -MMD

-include $(DEP)

