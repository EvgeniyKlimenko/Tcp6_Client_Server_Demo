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
C_FLAGS := /c /DWIN64 /D_WIN64 /DUNICODE /D_UNICODE /DDEBUG /D_DEBUG /D_CRT_SECURE_NO_DEPRECATE /ZI /FS /W4 /EHa /GR /MTd /std:c++14
L_FLAGS := /MTd
#PRECOMPILED_HEADER := CommonDefinitions.h 
MSVC_HEADERS_PATH := "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.24.28314/include"
SHARED_HEADERS_PATH := "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/shared"
CRT_HEADERS_PATH := "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/ucrt"
SDK_HEADERS_PATH := "C:/Program Files (x86)/Windows Kits/10/Include/10.0.18362.0/um"
BOOST_HEADERS_PATH := "C:/boost/boost_1_72_0"
INCLUDE_DIRS := /I $(MSVC_HEADERS_PATH) /I $(SHARED_HEADERS_PATH) /I $(CRT_HEADERS_PATH) /I $(SDK_HEADERS_PATH) /I $(BOOST_HEADERS_PATH)
MSVC_LIBS_PATH := "C:/Program Files (x86)/Microsoft Visual Studio/2019/BuildTools/VC/Tools/MSVC/14.24.28314/lib/x64"
CRT_LIBS_PATH := "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/ucrt/x64"
SDK_LIBS_PATH := "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64"
BOOST_LIBS_PATH := "C:/boost/boost_1_72_0/lib64-msvc-14.2"
LIB_DIRS := /LIBPATH:$(MSVC_LIBS_PATH) /LIBPATH:$(CRT_LIBS_PATH) /LIBPATH:$(SDK_LIBS_PATH) /LIBPATH:$(BOOST_LIBS_PATH)
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
LINKTOOL := $(CC)
INC_OPT := -I
C_FLAGS := -std=c++14 -Wall -Wextra -g
L_FLAGS := -pthread
OUT_FILE := -o  
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

	$(if $(3),,
		$(CHECK_BIN_DIR)
	)
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

define get_files
	$(eval PATTERN := $(3))
	$(eval CURRENT := $(1)$(SEP)$(2))
	$(eval CHILDREN := $(strip $(notdir $(wildcard $(CURRENT)$(SEP)*))))
	$(eval SUBDIRS := $(filter-out %$(PATTERN),$(CHILDREN)))
	$(eval FILES := $(addprefix $(2)$(SEP),$(filter %$(PATTERN),$(CHILDREN))))
	retval = $(FILES)
	$(4) += $$(retval)
	$(foreach dir,$(SUBDIRS),$(call get_files,$(1),$(2)$(SEP)$(dir),$(PATTERN),$(4)))
endef

define add_path_prefix
	retval=$(1)$(SEP)$(2)
	$(3) += $$(retval)
endef

define link_library
	$(eval OBJECTS :=)
	$(eval OBJECT_PATHS :=)
	$(eval MODULE_PATH := $(abspath $(LIB)$(SEP)$(SYSTEM)$(SEP)$(1)$(SEP)$(LIB_NAME_PREFIX)$(1)$(LIB_EXT)))
	$(eval L_OPTS := /MACHINE:x64 /DEBUG:FULL)
	$(eval $(call get_files,$(abspath $(BUILD)$(SEP)$(SYSTEM)$(SEP)),$(1),$(OBJ_EXT),OBJECTS))
	$(foreach ofile,$(OBJECTS),$(eval $(call add_path_prefix,$(abspath $(BUILD)$(SEP)$(SYSTEM)$(SEP)),$(ofile),OBJECT_PATHS)))
	$(if $(OS),
		$(eval L_OPTS := /MACHINE:x64),
		$(eval L_OPTS := -rc)
	)
	$(if $(OS),
		$(eval ADDITIONAL_STEP :=),
		$(eval ADDITIONAL_STEP := ranlib $(MODULE_PATH))
	)
	$(eval LINKING := $(LIBTOOL) $(L_OPTS) $(LIB_OUT_FILE)$(MODULE_PATH) $(OBJECT_PATHS))

	$(LINKING)
	$(ADDITIONAL_STEP)
endef

define link_executable
	$(eval OBJECTS :=)
	$(eval OBJECT_PATHS :=)
	$(eval MODULE_PATH := $(abspath $(BIN)$(SEP)$(SYSTEM)$(SEP)$(1)$(SEP)$(1)$(BIN_EXT)))
	$(eval $(call get_files,$(abspath $(BUILD)$(SEP)$(SYSTEM)$(SEP)),$(1),$(OBJ_EXT),OBJECTS))
	$(foreach ofile,$(OBJECTS),$(eval $(call add_path_prefix,$(abspath $(BUILD)$(SEP)$(SYSTEM)$(SEP)),$(ofile),OBJECT_PATHS)))
	
	$(if $(OS),
		$(eval L_OPTS := /MACHINE:x64 /DEBUG:FULL /SUBSYSTEM:CONSOLE /OPT:NOICF /OPT:NOREF $(LIB_DIRS) /LIBPATH:$(abspath $(LIB)$(SEP)$(SYSTEM)$(SEP)$(COMMON)) $(COMMON)$(LIB_EXT) boost_system-vc142-mt-gd-x64-1_72$(LIB_EXT) ws2_32$(LIB_EXT)),
		$(eval L_OPTS := -L$(abspath $(LIB)$(SEP)$(SYSTEM)$(SEP)$(COMMON)) -l$(COMMON) -lboost_system -lboost_thread -lboost_program_options)
	)
	$(if $(OS),
		$(eval ADDITIONAL_STEP :=),
		$(eval ADDITIONAL_STEP := ranlib $(MODULE_PATH))
	)
	$(eval LINKING := $(LINKTOOL) $(OBJECT_PATHS) $(L_OPTS) $(OUT_FILE)$(MODULE_PATH))

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

	$(eval COMMON_INC_DIR := $(abspath $(INCLUDE)$(SEP)$(COMMON)))
	$(eval MODULE_INC_DIR := $(abspath $(INCLUDE)$(SEP)$(3)))
	$(eval INCLUDES := $(INCLUDE_DIRS) $(INC_OPT)$(COMMON_INC_DIR) $(INC_OPT)$(MODULE_INC_DIR))
	$(eval COMPILING := $(CC) $(C_FLAGS) $(SRC_C_FLAGS) $(INCLUDES) $(1))

	$(COMPILING)
endef

define build
	$(eval SOURCES :=)
	$(eval $(call get_files,$(abspath $(SRC)$(SEP)),$(1),.cpp,SOURCES))

	$(foreach sfile,$(SOURCES),$(call build_source,$(abspath $(SRC)$(SEP)$(sfile)),$(1),$(dir $(sfile))))
endef

all:
	$(call create_directories,$(COMMON),$(LIB))
	$(call create_directories,$(COMMON)$(SEP)System,$(LIB),1)
	$(call create_directories,$(SERVER),$(BIN))
	$(call create_directories,$(CLIENT),$(BIN))
	$(call build,$(COMMON))
	$(call link_library,$(COMMON))
	$(call build,$(SERVER))
	$(call link_executable,$(SERVER))
	$(call build,$(CLIENT))
	$(call link_executable,$(CLIENT))

clean:
	$(call remove_directories)
