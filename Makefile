EM_VERSION = 1.39.8-upstream
EM_DOCKER = docker run --rm -w /src -v $$PWD:/src trzeci/emscripten:$(EM_VERSION)
EMCC = $(EM_DOCKER) emcc
EMXX = $(EM_DOCKER) em++
WASM2WAT = $(EM_DOCKER) wasm2wat

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-c++11-extensions -Os
LDFLAGS := -s MODULARIZE=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s ALLOW_MEMORY_GROWTH=1

SRC_DIR = ./src
SRC_EXT = cc
INC_DIR = ./inc
BUILD = ./dist
TARGET = gb.wasm
WASM = $(TARGET:.js=.wasm)
WAST = $(WASM:.wasm=.wast)

TSC = npx tsc
TSC_FLAGS = -p ./

SOURCES := $(wildcard $(SRC_DIR)/*.$(SRC_EXT))
OBJECTS := $(SOURCES:$(SRC_DIR)/%.$(SRC_EXT)=$(BUILD)/%.o)
DEPS = $(OBJECTS:.o=.d)
INCLUDES = -I $(INC_DIR)/

all: $(BUILD)/$(TARGET) $(BUILD)/$(WAST) .ts
# 	@echo "Making symlink: $(TARGET) -> $<"
# 	@$(RM) $(TARGET)
# 	@ln -s $(BUILD)/$(TARGET) $(TARGET)

.PHONY: all clean debug

debug: CXXFLAGS += -DDEBUG -g
debug: all

clean:
	@rm -rvf $(BUILD)/*.wasm
	@rm -rvf $(BUILD)/*.wast
	@rm -rvf $(BUILD)/*.bin
	@rm -rvf $(BUILD)/*.o
	@rm -rvf $(BUILD)/*.d
	@rm -rvf $(BUILD)/$(TARGET)
	@rm -rvf $(TARGET)

$(BUILD)/$(WAST): $(BUILD)/$(TARGET)
	$(WASM2WAT) $(BUILD)/$(WASM) -o $(BUILD)/$(WAST)

$(BUILD)/$(TARGET): $(OBJECTS)
	@echo "Linking: $@"
	$(EMXX) $(OBJECTS) -o $@ ${LIBS} $(LDFLAGS)
	@cp $(BUILD)/$(WASM) $(BUILD)/$(WASM).bin

-include $(DEPS)

$(BUILD)/%.o: $(SRC_DIR)/%.$(SRC_EXT)
	@echo "Compiling: $< -> $@"
	$(EMXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@

.ts: $(TS_SRC)
	$(TSC) $(TSC_FLAGS)
