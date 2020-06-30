CXX ?= g++
CXXFlags := -std=c++17 -Wall -Wextra -Werror -g
LDFLAGS := 

SRC_DIR = ./src
SRC_EXT = cc
INC_DIR = ./inc
BUILD = ./bin
TARGET = main

SOURCES := $(wildcard $(SRC_DIR)/*.$(SRC_EXT))
OBJECTS := $(SOURCES:$(SRC_DIR)/%.$(SRC_EXT)=$(BUILD)/%.o)
DEPS = $(OBJECTS:.o=.d)
INCLUDES = -I $(INC_DIR)/

all: $(BUILD)/$(TARGET)
	@echo "Making symlink: $(TARGET) -> $<"
	@$(RM) $(TARGET)
	@ln -s $(BUILD)/$(TARGET) $(TARGET)

.PHONY: all clean debug

debug: CXXFLAGS += -DDEBUG -g
debug: all

clean:
	@rm -rvf $(BUILD)/*.o
	@rm -rvf $(BUILD)/*.d
	@rm -rvf $(BUILD)/$(TARGET)
	@rm -rvf $(TARGET)

$(BUILD)/$(TARGET): $(OBJECTS)
	@echo "Linking: $@"
	$(CXX) $(OBJECTS) -o $@ ${LIBS}

-include $(DEPS)

$(BUILD)/%.o: $(SRC_DIR)/%.$(SRC_EXT)
	@echo "Compiling: $< -> $@"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@
