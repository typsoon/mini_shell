
SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin
INC_DIR=include

PARSERDIR=input_parse

CFLAGS=-g -I$(INC_DIR)

COMPILER=gcc
# SRCS=utils.c mshell.c builtins.c builtins_impl.c execution_utils.c
SRCS=mshell.c builtins.c builtins_impl.c execution_utils.c notes_holder.c child_manager.c
OBJS:=$(SRCS:.c=.o)
OBJS:=$(addprefix $(OBJ_DIR)/,$(OBJS))

all: check_dirs $(BIN_DIR)/mshell 

$(BIN_DIR)/mshell: $(OBJS) $(OBJ_DIR)/siparse.a
	$(COMPILER) $(CFLAGS) $(OBJS) $(OBJ_DIR)/siparse.a -o $@ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(COMPILER) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/siparse.a:
	$(MAKE) -C $(PARSERDIR) INSTALL_DIR=$(realpath $(OBJ_DIR)) INC_DIR=$(realpath $(INC_DIR))

check_dirs:
	test -d $(OBJ_DIR) || mkdir $(OBJ_DIR)
	test -d $(BIN_DIR) || mkdir $(BIN_DIR)

clean:
	rm -f $(BIN_DIR)/mshell $(OBJS) 

full_clean: clean
	$(MAKE) -C $(PARSERDIR) clean
	rm -f $(OBJ_DIR)/siparse.a
