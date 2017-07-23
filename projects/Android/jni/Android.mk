#include ../../../ofile/Android.mk

LOCAL_PATH := $(call my-dir)
SRC_ROOT := ../../..

include $(CLEAR_VARS)

SRC_FILES := $(wildcard $(SRC_ROOT)/ofile/*.cpp)

#$(warning $(SRC_FILES))

LOCAL_MODULE    := ofile
#LOCAL_SRC_FILES := $(SRC_ROOT)/ofile/ofile.cpp
LOCAL_SRC_FILES := $(SRC_FILES)
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 



include $(BUILD_STATIC_LIBRARY)
#############################################################################


include $(CLEAR_VARS)

LOCAL_MODULE    := address
LOCAL_SRC_FILES := $(SRC_ROOT)/test/address.cpp $(SRC_ROOT)/test/person.cpp
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

LOCAL_STATIC_LIBRARIES := ofile


include $(BUILD_EXECUTABLE)
#############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE    := permain
LOCAL_SRC_FILES := $(SRC_ROOT)/test/permain.cpp $(SRC_ROOT)/test/person.cpp $(SRC_ROOT)/test/trans.cpp
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

LOCAL_STATIC_LIBRARIES := ofile


include $(BUILD_EXECUTABLE)
#############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := bm
LOCAL_SRC_FILES := $(SRC_ROOT)/test/bm.cpp $(SRC_ROOT)/test/mmyclass.cpp
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

LOCAL_STATIC_LIBRARIES := ofile

include $(BUILD_EXECUTABLE)

#############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := bm_db
LOCAL_SRC_FILES := $(SRC_ROOT)/test/bm_db.cpp $(SRC_ROOT)/test/mmyclass.cpp
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

LOCAL_STATIC_LIBRARIES := ofile


include $(BUILD_EXECUTABLE)
#############################################################################
#include $(CLEAR_VARS)

#LOCAL_MODULE    := bm_mt
#LOCAL_SRC_FILES := $(SRC_ROOT)/test/bm_mt.cpp $(SRC_ROOT)/test/mmyclass.cpp
#LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

#LOCAL_STATIC_LIBRARIES := ofile


#include $(BUILD_EXECUTABLE)
#############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := blobtest
LOCAL_SRC_FILES := $(SRC_ROOT)/test/blobfile.cpp $(SRC_ROOT)/test/blobmain.cpp 
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

LOCAL_STATIC_LIBRARIES := ofile


include $(BUILD_EXECUTABLE)
#############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := blobtestp
LOCAL_SRC_FILES := $(SRC_ROOT)/test/blobfilp.cpp $(SRC_ROOT)/test/blobmain.cpp 
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

LOCAL_STATIC_LIBRARIES := ofile


include $(BUILD_EXECUTABLE)
#############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := main
LOCAL_SRC_FILES := $(SRC_ROOT)/test/myclass.cpp $(SRC_ROOT)/test/mycolct.cpp $(SRC_ROOT)/test/myblob.cpp $(SRC_ROOT)/test/main.cpp 
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

LOCAL_STATIC_LIBRARIES := ofile


include $(BUILD_EXECUTABLE)
#############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := odsettst
LOCAL_SRC_FILES := $(SRC_ROOT)/test/odsettst.cpp  $(SRC_ROOT)/test/myclass5.cpp 
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

LOCAL_STATIC_LIBRARIES := ofile


include $(BUILD_EXECUTABLE)
#############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := odrelm
LOCAL_SRC_FILES := $(SRC_ROOT)/test/odrelm.cpp  $(SRC_ROOT)/test/odrel.cpp 
LOCAL_C_INCLUDES := $(SRC_ROOT)/ofile 

LOCAL_STATIC_LIBRARIES := ofile


include $(BUILD_EXECUTABLE)
#############################################################################
