APP = vpss_app

APP_OBJS = vpss_app.o

INCLUDE=-I../include 

LINKLIBS= -L../lib

all: build

build: $(APP)

$(APP) : $(APP_OBJS)
	$(CC) $(INCLUDE) -o $@ $< $(LINKLIBS) -lvprocss -lcsc -lhsampler -lhscaler -lvscaler -lxgpio -lvresampler -laxiswitch 

clean:
	rm -rf $(APP_OBJS)
