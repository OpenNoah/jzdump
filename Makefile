SRC	:= main.cpp test.cpp
HDR	:= test.h
TRG	:= jzdump

MOC_SRC	:= $(HDR:%.h=moc_%.cpp)
SRC	+= $(MOC_SRC)

TOP	?= $(PWD)/..
INC	+= -I$(TOP)/include/opie-1.2.0 -I$(TOP)/include/qt-2.3.10
LIB	+= -L$(TOP)/lib/opie-1.2.0 -L$(TOP)/lib/qt-2.3.10 \
	   -L$(TOP)/rootfs/lib -L$(TOP)/rootfs/usr/lib \
	   -Wl,-rpath,/opt/QtPalmtop/lib -Wl,-rpath,/opt/qt-2.3.10/lib \
	   -lqpe -lopiecore2 -lqte -lz -ljpeg
DEF	+= -DQWS -D_REENTRANT
#FLAGS	?= -pipe -fno-rtti -fno-exceptions -fPIC -fPIE
FLAGS	?= -g -O0 -fno-rtti -fno-exceptions -fPIC
CROSS	?= $(PWD)/../mipseltools-gcc412-lnx26/bin/mipsel-linux-
MOC	?= ../bin/moc

.PHONY: all
all: $(TRG)

.PHONY: send
send: $(TRG)
	-cat $(TRG) | ncat -l -p 1234 --send-only &
	./expect | strings

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
