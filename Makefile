CXX=mpic++
CXXFLAGS=-pthread -Wall -O0
TARGET=bittorent

SRCDIR=clients server src utils
OBJDIR=bin

# Ensure directories are treated as sources
SRC_DIRS=$(SRCDIR) .

# Finding all .cpp files in the specified directories
SOURCES=$(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))

# Object files have the same names as source files,
# but are located in the OBJDIR directory
OBJECTS=$(SOURCES:%.cpp=$(OBJDIR)/%.o)

.PHONY: all build clean

all: build

build: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(OBJDIR)
	$(CXX) $(OBJECTS) -o $@ $(CXXFLAGS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
	@rm -rf $(OBJDIR) $(TARGET)
	@rm -rf build/bittorent
