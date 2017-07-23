###########################################################################
# Define test targets based on PROJECT
# Make files that include this must define PROJECT
# Requirements
# - compiler tools must either be in PATH or GCC_BIN set to their location.
# - LINUX_ROOT must be defined 
# Arguments
#  DEBUG=1 for debug code
# The following targets may also be defined:
# $(PROJECT)_ADDITIONAL_SOURCES
#
###########################################################################


$(info PATH =  $(PATH))
RM = rm -f -r
CAT = cat
ECHO = echo

# Add targets for platform/project combination
.PHONY: $(TARGET_PLATFORM)_all $(TARGET_PLATFORM)_clean $(TARGET_PLATFORM)_check
$(TARGET_PLATFORM)_all: $(TARGET_PLATFORM)_$(PROJECT) 
$(TARGET_PLATFORM)_clean: $(TARGET_PLATFORM)_clean_$(PROJECT)
$(TARGET_PLATFORM)_check: $(TARGET_PLATFORM)_check_$(PROJECT)


#################################################################################################################
# Target platform dependant definitions.
GCC_BIN = 

# All os references relative to the root.
ifeq ($(strip $(LINUX_ROOT)),)
  # FIXME - Change to proper location
  OS_ROOT := 
else
  OS_ROOT := $(LINUX_ROOT)
endif


# Define variables to be used in the recipes of the targets in the context of the main target.
# We do this because the original variables will be overridden by other make files by the time the
# recipe is executed. Note that these variables are recursively inherited by all prerequisites.
$(TARGET_PLATFORM)_$(PROJECT) : INCLUDE_PATHS := -I. 
$(TARGET_PLATFORM)_$(PROJECT) : LIBRARY_PATHS := 
$(TARGET_PLATFORM)_$(PROJECT) : LIBRARIES := 

$(TARGET_PLATFORM)_$(PROJECT) : AS      = $(GCC_BIN)as
$(TARGET_PLATFORM)_$(PROJECT) : CC      = $(GCC_BIN)gcc
$(TARGET_PLATFORM)_$(PROJECT) : CXX     = $(GCC_BIN)g++
$(TARGET_PLATFORM)_$(PROJECT) : CPP     = $(GCC_BIN)gcc -E
$(TARGET_PLATFORM)_$(PROJECT) : LD      = $(GCC_BIN)g++
$(TARGET_PLATFORM)_$(PROJECT) : OBJCOPY = $(GCC_BIN)objcopy
$(TARGET_PLATFORM)_$(PROJECT) : OBJDUMP = $(GCC_BIN)objdump
$(TARGET_PLATFORM)_$(PROJECT) : SIZE    = $(GCC_BIN)size 


$(TARGET_PLATFORM)_$(PROJECT) : CC_SYMBOLS = 
$(TARGET_PLATFORM)_$(PROJECT) : CC_FLAGS =  -c -g -m32 -fno-common -fmessage-length=0 -Wall -Wextra -ffunction-sections -fdata-sections -fomit-frame-pointer -MMD -MP
$(TARGET_PLATFORM)_$(PROJECT) : CC_SYMBOLS += $(patsubst %,-D%, $(CC_TESTS))
$(TARGET_PLATFORM)_$(PROJECT) : LD_FLAGS =  -m32 -Wl,-Map=$(basename $@).map
#$(TARGET_PLATFORM)_$(PROJECT) : LD_SYS_LIBS = -lrt -lpthread  -lm -lc -lgcc 
$(TARGET_PLATFORM)_$(PROJECT) : LD_SYS_LIBS =  -lm -lc -lgcc 

ifeq ($(DEBUG), 1)
  $(info $(TARGET_PLATFORM)_$(PROJECT) "DEBUG")
  $(TARGET_PLATFORM)_$(PROJECT) : CC_FLAGS += -DDEBUG -O0
else
  $(info $(TARGET_PLATFORM)_$(PROJECT) "NO DEBUG")
  $(TARGET_PLATFORM)_$(PROJECT) : CC_FLAGS += -DNDEBUG -Os -g0
endif


$(TARGET_PLATFORM)_$(PROJECT) : INCLUDE_PATHS += -I$(SRC_ROOT)/ofile


#################################################################################################################




# Fixed list of test files for each test executable.
TST_OBJECTS:=	$(patsubst %.c,%.o,$(filter %.c, $($(PROJECT)_ADDITIONAL_SOURCES))) 

CPP_TST_OBJECTS:= $(patsubst %.cpp,%.o,$(filter %.cpp, $($(PROJECT)_ADDITIONAL_SOURCES))) 


# Give the objects a target specific folder
TST_OBJECTS   := $(addprefix  $(OUTOBJ)/,$(notdir $(TST_OBJECTS)))
$(info $(TARGET_PLATFORM) TST_OBJECTS=$(TST_OBJECTS))
CPP_TST_OBJECTS   := $(addprefix  $(OUTOBJ)/,$(notdir $(CPP_TST_OBJECTS)))
$(info $(TARGET_PLATFORM) CPP_TST_OBJECTS=$(CPP_TST_OBJECTS))

# List of all folders under Modules
SOURCE_DIRS = $(SRC_ROOT)/ofile $(SRC_ROOT)/test
$(info SOURCE_DIRS=$(SOURCE_DIRS))
# This is so that it finds the source files. Add more paths if needed.
vpath  %.c  $(SOURCE_DIRS)  
vpath  %.cpp $(SOURCE_DIRS) 

# These rules will be used by target independant files. 
$(OUTOBJ)/%.o:  %.c | $(OUTOBJ) 
	$(CC)   $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99  $(INCLUDE_PATHS) -o $@ $<
$(OUTOBJ)/%.o : %.cpp | $(OUTOBJ) 
	$(CXX)   $(CC_FLAGS) $(CC_SYMBOLS)  $(INCLUDE_PATHS) -o $@ $<
	

# Build executables and listings.
.PHONY: $(TARGET_PLATFORM)_$(PROJECT)
$(TARGET_PLATFORM)_$(PROJECT) :  $(OUTOBJ) $(OUT)/$(PROJECT).lst

$(OUT)/$(PROJECT).out:  $(CPP_TST_OBJECTS) $(TST_OBJECTS) $(OBJECTS) $(CPP_OBJECTS) $(S_OBJECTS) $(SYS_OBJECTS) 
	$(LD) $(LD_FLAGS)   $(LIBRARY_PATHS) -o $@ $^ -Wl,--start-group $(LIBRARIES) $(LD_SYS_LIBS) -Wl,--end-group

$(OUT)/$(PROJECT).lst: $(OUT)/$(PROJECT).out
	$(OBJDUMP) -Sdh $< > $@


$(TARGET_PLATFORM)_$(PROJECT)_DEPS = $(OBJECTS:.o=.d) $(S_OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d) $(TST_OBJECTS:.o=.d)  $(CPP_TST_OBJECTS:.o=.d)
-include $($(TARGET_PLATFORM)_$(PROJECT)_DEPS)

# Create a list of files to delete for each target on the first pass of the make
$(TARGET_PLATFORM)_clean_$(PROJECT) : OUTPUTS:=   $(OUT)/$(PROJECT).out  *.db *.ofl *.xml *.tst *.cp \
		$(OUT)/$(PROJECT).map $(OUT)/$(PROJECT).lst  $(OUT)/$(PROJECT).xml $(TST_OBJECTS)  $(CPP_TST_OBJECTS) \
		$($(TARGET_PLATFORM)_$(PROJECT)_DEPS) \
		$(OUT)/$(PROJECT)_result.txt 

# Remove files in the list $(PROJECT)_OUTPUTS. 
# We dynamically create the list variable from the target
.PHONY: $(TARGET_PLATFORM)_clean_$(PROJECT) 
$(TARGET_PLATFORM)_clean_$(PROJECT): 
	$(RM) $(OUTPUTS)


# This makes sure anyone who is dependant on it always executes its recipe
.FORCE:

# Always run the test.
$(OUT)/$(PROJECT)_result.txt:  $(TARGET_PLATFORM)_$(PROJECT) $(OUT)/$(PROJECT).out .FORCE
	$(word 2, $^) > $(basename $(word 2, $^)).int
	# Output the result file
	$(CAT) $@

# check. Install and run tests
.PHONY: $(TARGET_PLATFORM)_check_$(PROJECT)
$(TARGET_PLATFORM)_check_$(PROJECT):  $(OUT)/$(PROJECT)_result.txt
