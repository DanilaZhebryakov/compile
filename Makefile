CC := g++
CFLAGS := -D _DEBUG -g -ggdb3 -std=c++17 -O0 -Wall -Wextra -Wno-missing-field-initializers -I ./
LFLAGS :=
OBJDIR := obj


OBJ_FUNC = $(addprefix $(OBJDIR)/, $(1:.cpp=.o))
DEP_FUNC = $(addprefix $(OBJDIR)/, $(1:.cpp=.d))

LIBDIR  := lib
LIBSRC  := $(wildcard $(LIBDIR)/*.cpp)
LIB_OBJ := $(call OBJ_FUNC, $(LIBSRC))


COMMON_SRC := $(wildcard expr/*.cpp)

FRONT_SRC := $(wildcard frontend/*.cpp)
FRONT_RUN_FILE := run/Frontend.elf

BACK_SRC := $(wildcard backend/*.cpp) $(wildcard program/*.cpp)
BACK_RUN_FILE := run/Backend.elf

MID_SRC := $(wildcard midend/*.cpp)   $(wildcard program/*.cpp)
MID_RUN_FILE := run/Midend.elf

ALL_SRC := main_full.cpp
ALL_RUN_FILE := run/full_compile.elf

.PHONY: all clean run libs proc asm disasm parser prepare-code

all: libs prepare-code front back mid

clean:
	rm -r $(OBJDIR)
	rm    $(RUNFILE)
	

run:
	$(ASM_RUN_FILE)
	$(PROC_RUN_FILE)

libs: $(LIB_OBJ)

front: $(FRONT_RUN_FILE)

back:  $(BACK_RUN_FILE)

mid:   $(MID_RUN_FILE)

OBJECTSF := $(call OBJ_FUNC, $(COMMON_SRC) $(FRONT_SRC))
$(FRONT_RUN_FILE): libs $(OBJECTSF)
	$(CC) $(LIB_OBJ) $(OBJECTSF) -o $@ $(LFLAGS)

OBJECTSB := $(call OBJ_FUNC, $(COMMON_SRC) $(BACK_SRC))
$(BACK_RUN_FILE) : libs $(OBJECTSB)
	$(CC) $(LIB_OBJ) $(OBJECTSB) -o $@ $(LFLAGS)

OBJECTSM := $(call OBJ_FUNC, $(COMMON_SRC) $(MID_SRC))
$(MID_RUN_FILE) : libs $(OBJECTSM)
	$(CC) $(LIB_OBJ) $(OBJECTSM) -o $@ $(LFLAGS)


$(OBJDIR)/%.d : %.cpp
	mkdir -p $(dir $@)
	$(CC) $< -c -MMD -MP -o $(@:.d=.o) $(CFLAGS)

$(OBJDIR)/%.o : %.cpp
	mkdir -p $(dir $@)
	$(CC) $< -c -MMD -MP -o $@ $(CFLAGS)

-include $(wildcard $(OBJDIR)/*.d)