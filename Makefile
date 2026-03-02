APPNAME := soft_tissues

# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -pedantic -std=c++2a -I./deps/include -I./src -MMD -MP
LDFLAGS := -L./deps/lib/linux -lraylib -limgui -lGL -lpthread -ldl

CXXFLAGS += -O3
# CXXFLAGS += -fsanitize=address -g
# LDFLAGS += -fsanitize=address

# Directories
SRCDIR := ./src
BUILDDIR := ./build/linux
OBJDIR := $(BUILDDIR)/obj
TARGET := $(BUILDDIR)/$(APPNAME)

# Source files and object files
SRCFILES := $(shell find $(SRCDIR) -name '*.cpp')
OBJFILES := $(SRCFILES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
DEPFILES := $(OBJFILES:.o=.d)

# External source compiled alongside (not tracked for deps)
EXTRA_SRCS := ./deps/src/ImGuiFileDialog.cpp

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJFILES)
	$(CXX) -o $@ $^ $(EXTRA_SRCS) $(LDFLAGS)

# Build object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build directories if they don't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Clean up build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

-include $(DEPFILES)

.PHONY: all clean
