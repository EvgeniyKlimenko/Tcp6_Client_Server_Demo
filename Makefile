BIN := bin
BUILD := build
LIB := lib
SRC	:= src
INCLUDE	:= include
SEP := /
RM :=  rmdir /s /q

COMMON := Common
CLIENT := Client
SERVER := Server

ifeq ($(OS),Windows_NT)
SYSTEM := Windows
BIN_EXT := .exe
LIB_EXT := .lib
OBJ_EXT := .obj
PROG_DB_EXT := .pdb
ASM_EXT := .asm
LIB_NAME_PREFIX :=
CC:= cl.exe
LIBTOOL := lib.exe
LINKTOOL := link.exe
PLATFORM_SEP := \\
INC_OPT := /I
C_FLAGS := /c /DWIN64 /DDEBUG /D_CRT_SECURE_NO_DEPRECATE /ZI /FS /W4 /EHa /GR /MTd /std:c++14
L_FLAGS := /MTd
#PRECOMPILED_HEADER := CommonDefinitions.h 
MSVC_HEADERS_PATH := "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.24.28314/include"
SHARED_HEADERS_PATH := "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/shared"
CRT_HEADERS_PATH := "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/ucrt"
SDK_HEADERS_PATH := "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/um"
BOOST_HEADERS_PATH := "C:/boost/include/boost-1_60"
INCLUDE_DIRS := /I $(MSVC_HEADERS_PATH) /I $(SHARED_HEADERS_PATH) /I $(CRT_HEADERS_PATH) /I $(SDK_HEADERS_PATH) #/I $(BOOST_HEADERS_PATH)
MSVC_LIBS_PATH := "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.24.28314/lib/x64"
CRT_LIBS_PATH := "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/ucrt/x64"
SDK_LIBS_PATH := "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64"
BOOST_LIBS_PATH := "C:/boost/lib"
LIB_DIRS := /LIBPATH:$(MSVC_LIBS_PATH) /LIBPATH:$(CRT_LIBS_PATH) /LIBPATH:$(SDK_LIBS_PATH) #/LIBPATH:$(BOOST_LIBS_PATH)
L_OPTS := /link /machine:X64 $(LIB_DIRS)
OUT_FILE := /OUT:
LIB_OUT_FILE := $(OUT_FILE)
MKDIR := md
else
SYSTEM := Linux
PLATFORM_SEP := $(SEP)
BIN_EXT :=
LIB_EXT := .a
OBJ_EXT := .o
LIB_NAME_PREFIX := lib
CC:= g++
LIBTOOL := ar
INC_OPT := -I
C_FLAGS := -std=c++14 -Wall -Wextra -g
L_FLAGS := -pthread
OUT_FILE := -o:
LIB_OUT_FILE :=
MKDIR := mkdir -p
endif

define create_directories
	$(eval BIN_BASE_DIR := $(abspath $(2)$(SEP)$(SYSTEM)$(SEP)))
	$(eval BUILD_BASE_DIR := $(abspath $(BUILD)$(SEP)$(SYSTEM)$(SEP)))
	$(eval BIN_DIR := $(BIN_BASE_DIR)$(SEP)$(1))
	$(eval BUILD_DIR := $(BUILD_BASE_DIR)$(SEP)$(1))
	$(eval CREATE_BIN_DIR := $(MKDIR) $(subst $(SEP),$(PLATFORM_SEP),$(BIN_DIR)))
	$(eval CREATE_BUILD_DIR := $(MKDIR) $(subst $(SEP),$(PLATFORM_SEP),$(BUILD_DIR)))
	$(eval BIN_DIR_PRESENT:= $(strip $(wildcard $(BIN_BASE_DIR)$(SEP)$(1))))
	$(eval BUILD_DIR_PRESENT:= $(strip $(wildcard $(BUILD_BASE_DIR)$(SEP)$(1))))
	$(eval CHECK_BIN_DIR := $(if $(BIN_DIR_PRESENT),$(info Directory $(BIN_DIR) present.),$(CREATE_BIN_DIR)))
	$(eval CHECK_BUILD_DIR := $(if $(BUILD_DIR_PRESENT),$(info Directory $(BUILD_DIR) present.),$(CREATE_BUILD_DIR)))

	$(CHECK_BIN_DIR)
	$(CHECK_BUILD_DIR)
endef

define remove_directories
	$(eval BIN_BASE_DIR := $(abspath $(BIN)$(SEP)$(SYSTEM)$(SEP)))
	$(eval LIB_BASE_DIR := $(abspath $(LIB)$(SEP)$(SYSTEM)$(SEP)))
	$(eval BUILD_BASE_DIR := $(abspath $(BUILD)$(SEP)$(SYSTEM)$(SEP)))

	$(RM) $(subst $(SEP),$(PLATFORM_SEP),$(BIN_BASE_DIR))
	$(RM) $(subst $(SEP),$(PLATFORM_SEP),$(LIB_BASE_DIR))
	$(RM) $(subst $(SEP),$(PLATFORM_SEP),$(BUILD_BASE_DIR))
endef

define link_library
	$(eval MODULE_PATH := $(abspath $(LIB)$(SEP)$(SYSTEM)$(SEP)$(1)$(SEP)$(LIB_NAME_PREFIX)$(1)$(LIB_EXT)))
	$(eval BUILD_PATH := $(abspath $(BUILD)$(SEP)$(SYSTEM)$(SEP)$(1)))
	$(eval L_OPTS := /MACHINE:x64)
	$(eval OBJECTS := $(wildcard $(BUILD_PATH)$(SEP)*$(OBJ_EXT)))
	$(if $(OS),
		$(eval L_OPTS := /MACHINE:x64),
		$(eval L_OPTS := -rc)
	)
	$(if $(OS),
		$(eval ADDITIONAL_STEP :=),
		$(eval ADDITIONAL_STEP := ranlib $(MODULE_PATH))
	)
	$(eval LINKING := $(LIBTOOL) $(L_OPTS) $(LIB_OUT_FILE)$(MODULE_PATH) $(OBJECTS))

	$(LINKING)
	$(ADDITIONAL_STEP)
endef

define link_executable
	$(eval MODULE_PATH := $(abspath $(BIN)$(SEP)$(SYSTEM)$(SEP)$(1)$(SEP)$(1)$(BIN_EXT)))
	$(eval BUILD_PATH := $(abspath $(BUILD)$(SEP)$(SYSTEM)$(SEP)$(1)))
	$(eval L_OPTS := /MACHINE:x64 /SUBSYSTEM:CONSOLE /OPT:NOICF /OPT:NOREF $(LIB_DIRS) /LIBPATH:$(abspath $(LIB)$(SEP)$(SYSTEM)$(SEP)$(COMMON)))
	$(eval OBJECTS := $(wildcard $(BUILD_PATH)$(SEP)*$(OBJ_EXT)))
	$(eval LINKING := $(LINKTOOL) $(L_OPTS) $(OBJECTS) $(COMMON)$(LIB_EXT) /OUT:$(MODULE_PATH))

	$(LINKING)
endef

define build_source
	$(if $(OS),
		$(eval OBJ_FILE = $(subst $(SRC),$(BUILD)$(SEP)$(SYSTEM),$(subst .cpp,$(OBJ_EXT),$(1))))
		$(eval PDB_FILE = $(subst .obj,.pdb,$(OBJ_FILE)))
		$(eval ASM_FILE = $(subst .obj,.asm,$(OBJ_FILE)))
		$(eval SRC_C_FLAGS := /Fo$(OBJ_FILE) /Fa$(ASM_FILE) /Fd$(PDB_FILE)),
		$(eval OBJ_FILE = $(subst $(SRC),$(BUILD)$(SEP)$(SYSTEM),$(subst .cpp,$(OBJ_EXT),$(1))))
		$(eval SRC_C_FLAGS := -c -o $(OBJ_FILE))
	)

	$(eval MODULE_INC_DIR := $(abspath $(INCLUDE)$(SEP)$(2)))
	$(eval INCLUDES := $(INCLUDE_DIRS) $(INC_OPT)$(MODULE_INC_DIR))
	$(eval COMPILING := $(CC) $(C_FLAGS) $(SRC_C_FLAGS) $(INCLUDES) $(1))

	$(COMPILING)
endef

define build
	$(eval MODULE_SRC_DIR := $(abspath $(SRC)$(SEP)$(1)))
	$(eval SOURCES := $(strip $(abspath $(wildcard $(MODULE_SRC_DIR)/*.cpp))))

	$(foreach sfile,$(SOURCES),$(call build_source,$(sfile),$(1)))
endef

all:
	$(call create_directories,$(COMMON),$(LIB))
	$(call create_directories,$(SERVER),$(BIN))
	$(call create_directories,$(CLIENT),$(BIN))
	$(call build,$(COMMON))
	$(call link_library,$(COMMON))

clean:
	$(call remove_directories)
