#
# Makefile
# WARNING: relies on invocation setting current working directory to Makefile location
# This is done in .vscode/task.json
#
PROJECT 			?= lvgl-sdl
MAKEFLAGS 			:= -j $(shell nproc)
EXT_C				:= c
EXT_CPP				:= cpp
OBJ_EXT				:= o
CC 					:= gcc
CXX					:= g++

SRC_DIR				:= ./
WORKING_DIR			:= ./build
BUILD_DIR			:= $(WORKING_DIR)/obj
BIN_DIR				:= .

WARNINGS 			:= -Wall -Wextra \
						-Wshadow -Wundef -Wmaybe-uninitialized -Wmissing-prototypes -Wno-discarded-qualifiers \
						-Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized \
						-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default  \
						-Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated  \
						-Wempty-body -Wshift-negative-value -Wstack-usage=2048 \
						-Wtype-limits -Wsizeof-pointer-memaccess -Wpointer-arith

CFLAGS 				:= -O0 -g $(WARNINGS)
CPPFLAGS 			:= -O0 -g -fpermissive

# Add simulator define to allow modification of source
DEFINES				:= -D SIMULATOR=1 -D LV_BUILD_TEST=0

# Include simulator inc folder first so lv_conf.h from custom UI can be used instead
INC 				:= -I./lv_monitor/ -I./ -I./lvgl/ #-I/usr/include/freetype2 -L/usr/local/lib
LDLIBS	 			:= -lSDL2 -lm #-lfreetype -lavformat -lavcodec -lavutil -lswscale -lm -lz -lpthread
BIN 				:= $(BIN_DIR)/demo

COMPILE				= $(CC) $(CFLAGS) $(INC) $(DEFINES)
CPPCOMPILE			= $(CXX) $(CPPFLAGS) $(INC) $(DEFINES)

# Automatically include all source files
SRC_C 				:= $(shell find $(SRC_DIR) -type f -name '*.c' -not -path '*/\.*')
SRC_CPP 			:= $(shell find $(SRC_DIR) -type f -name '*.cpp' -not -path '*/\.*')
INCS 				:= $(shell find $(SRC_DIR) -type f -name '*.h' -not -path '*/\.*')
OBJECTS    			:= $(patsubst $(SRC_DIR)%,$(BUILD_DIR)/%,$(SRC_C:.$(EXT_C)=.$(OBJ_EXT)))
OBJECTS    			+= $(patsubst $(SRC_DIR)%,$(BUILD_DIR)/%,$(SRC_CPP:.$(EXT_CPP)=.$(OBJ_EXT)))

all: default

$(BUILD_DIR)/%.$(OBJ_EXT): $(SRC_DIR)/%.$(EXT_C) $(INCS)
	@echo 'Building project file: $<'
	@mkdir -p $(dir $@)
	@$(COMPILE) -c -o "$@" "$<"

$(BUILD_DIR)/%.$(OBJ_EXT): $(SRC_DIR)/%.$(EXT_CPP) $(INCS)
	@echo 'Building project file: $<'
	@mkdir -p $(dir $@)
	@$(CPPCOMPILE) -c -o "$@" "$<"

default: $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) -o $(BIN) $(OBJECTS) $(LDFLAGS) ${LDLIBS}

clean:
	rm -rf $(WORKING_DIR)

install: ${BIN}
	install -d ${DESTDIR}/usr/lib/${PROJECT}/bin
	install $< ${DESTDIR}/usr/lib/${PROJECT}/bin/
