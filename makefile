TARGET = cw2_MartinRowe

SOURCES = \
	cw2_MartinRowe.c \
    cw2_MartinRowe_Biquads.c 

INCFOLDERS = \
    -I./include

LIBFOLDERS = \
    -L./lib

LIBS = \
    -lportsf

all:
	gcc -o  $(TARGET) -Wall $(INCFOLDERS) $(LIBFOLDERS) $(LIBS) $(FRAMEWORKS) $(SOURCES)

clean:
	rm $(TARGET)