BIN := bin
BUILD := build
LIB := lib
SRC	:= src
INCLUDE	:= include
SEP := /

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
CC:= cl.exe
LIBTOOL := lib.exe
LINKTOOL := link.exe
PLATFORM_SEP := \\
L_FLAGS := /MTd
PRECOMPILED_HEADER := CommonDefinitions.h 
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
OUT_FILE := /Fe:
RM :=  rmdir /s /q
MKDIR := md
SLEEP := timeout
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
	$(eval MODULE_PATH := $(abspath $(LIB)$(SEP)$(SYSTEM)$(SEP)$(1)$(SEP)$(1)$(LIB_EXT)))
	$(eval BUILD_PATH := $(abspath $(BUILD)$(SEP)$(SYSTEM)$(SEP)$(1)))
	$(eval L_OPTS := /MACHINE:x64)

	$(LIBTOOL) $(L_OPTS) $(wildcard $(BUILD_PATH)$(SEP)*$(OBJ_EXT)) /OUT:$(MODULE_PATH)
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
	$(eval OBJ_FILE = $(subst $(SRC),$(BUILD)$(SEP)$(SYSTEM),$(subst .cpp,$(OBJ_EXT),$(1))))
	$(eval PDB_FILE = $(subst .obj,.pdb,$(OBJ_FILE)))
	$(eval ASM_FILE = $(subst .obj,.asm,$(OBJ_FILE)))
	$(eval MODULE_INC_DIR := $(abspath $(INCLUDE)$(SEP)$(2)))
	$(eval INCLUDES := $(INCLUDE_DIRS)/I $(MODULE_INC_DIR))
	$(eval C_FLAGS := /c /DWIN64 /DDEBUG /D_CRT_SECURE_NO_DEPRECATE /ZI /FS /W4 /EHa /GR /Fo$(OBJ_FILE) /Fa$(ASM_FILE) /Fd$(PDB_FILE) /MTd /std:c++14)
	$(eval COMPILING := $(CC) $(C_FLAGS) $(INCLUDES) $(1))

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
	$(call build,$(SERVER))
	$(call link_executable,$(SERVER))

clean:
	$(call remove_directories)
