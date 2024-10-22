# Directories and files
ROOT_DIR:=.
BUILD_DIR:=${ROOT_DIR}/build
BUILD_BIN_DIR:=${BUILD_DIR}/bin
BUILD_ART_DIR:=${BUILD_DIR}/artifacts
BUILD_INC_DIR:=${BUILD_DIR}/include
BUILD_RESULT:=${BUILD_BIN_DIR}/libUARTKit.a

# Compiler subflags
SYSROOT:=../../sysroot
TARGET:=arm-linux-gnueabihf

# Libraries
LIBRARIES:=../../libpynq
LIBRARIES_ARCHIVE_FILES:=$(foreach library,$(LIBRARIES),$(wildcard $(library)/lib/*.a))
LIBRARIES_INCLUDE_FLAGS:=$(foreach library,$(LIBRARIES),-I$(library)/include)

# Compiler and Linker flags
CFLAGS:=--sysroot=${SYSROOT} --target=${TARGET} ${LIBRARIES_INCLUDE_FLAGS}
LDFLAGS:=--sysroot=${SYSROOT} --target=${TARGET} -fuse-ld=lld -lm -O0 -g3 -ggdb

# File dependencies
SOURCES:=$(wildcard src/*.c)
OBJECTS:=$(SOURCES:.c=.o)
ARTIFACTS:=$(foreach obj,$(OBJECTS),${BUILD_ART_DIR}/$(notdir $(obj)))
INCLUDES:=$(wildcard src/*.h)
COPIED_INCLUDES:=$(foreach inc,$(INCLUDES),${BUILD_INC_DIR}/$(notdir $(inc)))

# Rules
all: make_directories ${BUILD_RESULT} ${COPIED_INCLUDES}

${BUILD_RESULT}: ${ARTIFACTS}
	$(AR) rcs $@ $?

${BUILD_ART_DIR}/%.o: src/%.c
	$(CC) -c -o $@ $^ ${CFLAGS}

${BUILD_INC_DIR}/%.h: src/%.h
	cp $^ $@

make_directories:
	mkdir -p $(BUILD_BIN_DIR)
	mkdir -p $(BUILD_ART_DIR)
	mkdir -p $(BUILD_INC_DIR)

clean:
	rm -rf $(BUILD_BIN_DIR)
	rm -rf $(BUILD_ART_DIR)
	rm -rf $(BUILD_INC_DIR)