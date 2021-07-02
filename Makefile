SRC	:= main.cpp test.cpp
HDR	:= test.h
TRG	:= jzdump

MOC_SRC	:= $(HDR:%.h=moc_%.cpp)
SRC	+= $(MOC_SRC)

TOP	?= $(CURDIR)/../../build/qt-2.3.10
INC	+= -I$(TOP)/include -I$(TOP)/src/3rdparty/freetype/include
LIB	+= -L$(TOP)/lib -Wl,-rpath,/opt/qt-2.3.10/lib -lqte
DEF	+= -DQWS -D_REENTRANT
CFLAGS	:= -Os -pipe -D_FILE_OFFSET_BITS=64
FLAGS	?= -Os -pipe -fno-rtti -D_FILE_OFFSET_BITS=64
CROSS	?= mipsel-linux-
MOC	?= ../../bin/moc

#TOP	?= $(PWD)/..
##INC	+= -I$(TOP)/include/opie-1.2.0 -I$(TOP)/include/qt-2.3.10
##LIB	+= -L$(TOP)/lib/opie-1.2.0 -L$(TOP)/lib/qt-2.3.10 \
##	   -L$(TOP)/rootfs/lib -L$(TOP)/rootfs/usr/lib \
##	   -Wl,-rpath,/opt/QtPalmtop/lib -Wl,-rpath,/opt/qt-2.3.10/lib \
##	   -lqpe -lopiecore2 -lqte -lz -ljpeg
#INC	+= -I$(TOP)/include/qt-2.3.10.new -I$(TOP)/include/freetype
#LIB	+= -L$(TOP)/lib/qt-2.3.10.new \
#	   -L$(TOP)/rootfs/lib -L$(TOP)/rootfs/usr/lib \
#	   -Wl,-rpath,/opt/qt-2.3.10.new/lib \
#	   -lqte #-lqpe #-lz -ljpeg
#DEF	+= -DQWS -D_REENTRANT
##FLAGS	?= -pipe -fno-rtti -fno-exceptions -fPIC -fPIE
#FLAGS	?= -g -Os -pipe -fno-rtti -fno-exceptions -fPIC
#CROSS	?= $(PWD)/../mipseltools-gcc412-lnx26/bin/mipsel-linux-
#MOC	?= ../bin/moc

.PHONY: all
all: $(TRG)

.PHONY: send
send: $(TRG)
	-cat $(TRG) | ncat NP1380 1234 --send-only
	#-cat $(TRG) | ncat -l -p 1234 --send-only &
	#./expect | strings

$(TRG): $(SRC:%.cpp=%.o)
	$(CROSS)g++ -s -o $@ $^ $(LIB) $(FLAGS)

%.o: %.cpp
	$(CROSS)g++ -o $@ -c $(INC) $^ $(DEF) $(FLAGS)

moc_%.cpp: %.h
	$(MOC) -o $@ $<

.PHONY: clean
clean:
	rm -f $(TRG) $(SRC:%.cpp=%.o) $(MOC_SRC)

.PHONY: gdb
gdb: $(TRG)
	mipsel-linux-gdb $(TRG) \
		-ex 'target extended-remote NP1380:1234' \
		-ex 'set sysroot $(TOP)/rootfs'
