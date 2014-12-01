TARGET = cw2_MartingtRowe

SOURCES = \
	cw2_MartinRowe.c \
    CW2_Biquads.c 

INCFOLDERS = \
    -I./include

LIBFOLDERS = \
    -L./lib

LIBS = \
    -lportsf

all:
	gcc -o $(TARGET) -Wall $(INCFOLDERS) $(LIBFOLDERS) $(LIBS) $(FRAMEWORKS) $(SOURCES)

clean:
	rm $(TARGET)