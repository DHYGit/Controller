DEBUG=0
DBUS=1

APP := ./bin/controller
SRC_DIR := ./src
SRCS := \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/led.cpp


OBJS := $(SRCS:.cpp=.o)

CPPFLAGS += \
	-std=c++11 \
	-I"include"
	
LDFLAGS += \
	-L"$(CUDA_PATH)/targets/aarch64-linux/lib" \
	-lpthread \
	-L/usr/local/lib\
	-L/usr/lib/aarch64-linux-gnu/\
	-lm\
	-lz\
	-lanl\
	-Llib\
	-lasound\
	-ljsoncpp\
	-lcurl\
	-lpulse-simple\
	-lpulse \
	

CPP:=g++


all: $(APP)

%.o: %.cpp
	@echo "Compiling: $<"
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(APP): $(OBJS)
	@echo "Linking: $@"
	$(CPP) -o $@ $(OBJS) $(CPPFLAGS) $(LDFLAGS)

#	$(CPP) -g  -o  $@ -O0 $(OBJS) $(CPPFLAGS) $(LDFLAGS)


.PHONY: clean

clean:
	rm -f $(SRC_DIR)/*.o
	rm -f  $(APP)
	rm -f tags
