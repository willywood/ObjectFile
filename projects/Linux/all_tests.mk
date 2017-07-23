###########################################################################
# Define test targets based on PROJECT
#
###########################################################################
LOCAL_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SRC_ROOT=../..

# output folder:
OUT:=.
OUTOBJ:=$(OUT)/obj
$(OUTOBJ):
	$(MKDIR_QUIET) $@



OFILE_SRC   = 	$(SRC_ROOT)/ofile/ofile.cpp \
				$(SRC_ROOT)/ofile/ofile2.cpp \
				$(SRC_ROOT)/ofile/oufile.cpp \
				$(SRC_ROOT)/ofile/oio.cpp \
				$(SRC_ROOT)/ofile/oflist.cpp \
				$(SRC_ROOT)/ofile/opersist.cpp \
				$(SRC_ROOT)/ofile/ometa.cpp \
				$(SRC_ROOT)/ofile/ostrm.cpp \
				$(SRC_ROOT)/ofile/oistrm.cpp \
				$(SRC_ROOT)/ofile/oosxml.cpp \
				$(SRC_ROOT)/ofile/oisxml.cpp \
				$(SRC_ROOT)/ofile/ox.cpp \
				$(SRC_ROOT)/ofile/oxmlreader.cpp \
				$(SRC_ROOT)/ofile/oiter.cpp \
				$(SRC_ROOT)/ofile/ox.cpp \
				$(SRC_ROOT)/ofile/oblob.cpp \
				$(SRC_ROOT)/ofile/oblobp.cpp \
				$(SRC_ROOT)/ofile/oconvert.cpp \
				$(SRC_ROOT)/ofile/ConvertUTF.cpp \


#========================================================================
PROJECT=address

$(PROJECT)_ADDITIONAL_SOURCES:= $(SRC_ROOT)/test/address.cpp $(SRC_ROOT)/test/person.cpp $(OFILE_SRC)
																						

include BUILD_TEST_$(TARGET_PLATFORM).mk
#========================================================================
PROJECT=permain

$(PROJECT)_ADDITIONAL_SOURCES:= $(SRC_ROOT)/test/permain.cpp $(SRC_ROOT)/test/person.cpp $(SRC_ROOT)/test/trans.cpp $(OFILE_SRC)
																						

include BUILD_TEST_$(TARGET_PLATFORM).mk
#========================================================================
PROJECT=bm

$(PROJECT)_ADDITIONAL_SOURCES:= $(SRC_ROOT)/test/bm.cpp $(SRC_ROOT)/test/mmyclass.cpp $(OFILE_SRC)
																						

include BUILD_TEST_$(TARGET_PLATFORM).mk
#========================================================================
PROJECT=bm_db

$(PROJECT)_ADDITIONAL_SOURCES:= $(SRC_ROOT)/test/bm_db.cpp $(SRC_ROOT)/test/mmyclass.cpp $(OFILE_SRC)
																						

include BUILD_TEST_$(TARGET_PLATFORM).mk
#========================================================================
PROJECT=blobtest

$(PROJECT)_ADDITIONAL_SOURCES:= $(SRC_ROOT)/test/blobfile.cpp $(SRC_ROOT)/test/blobmain.cpp $(OFILE_SRC)
																						

include BUILD_TEST_$(TARGET_PLATFORM).mk
#========================================================================
PROJECT=blobtestp

$(PROJECT)_ADDITIONAL_SOURCES:= $(SRC_ROOT)/test/blobfilp.cpp $(SRC_ROOT)/test/blobmain.cpp  $(OFILE_SRC)
																						

include BUILD_TEST_$(TARGET_PLATFORM).mk
#========================================================================
PROJECT=main

$(PROJECT)_ADDITIONAL_SOURCES:= $(SRC_ROOT)/test/myclass.cpp $(SRC_ROOT)/test/mycolct.cpp $(SRC_ROOT)/test/myblob.cpp $(SRC_ROOT)/test/main.cpp   $(OFILE_SRC)
																						

include BUILD_TEST_$(TARGET_PLATFORM).mk
#========================================================================
PROJECT=odsettst

$(PROJECT)_ADDITIONAL_SOURCES:= $(SRC_ROOT)/test/odsettst.cpp  $(SRC_ROOT)/test/myclass5.cpp   $(OFILE_SRC)
																						

include BUILD_TEST_$(TARGET_PLATFORM).mk
#========================================================================
PROJECT=odrelm

$(PROJECT)_ADDITIONAL_SOURCES:= $(SRC_ROOT)/test/odrelm.cpp  $(SRC_ROOT)/test/odrel.cpp   $(OFILE_SRC)
																						

include BUILD_TEST_$(TARGET_PLATFORM).mk
#========================================================================

