CC=		gcc
CFLAGS=		-g -Wall

INCLUDES=	-I../lib -I../rtrace_dc/src/
LIBS=		-lm
ifeq ($(platform),mingw)
LIBS+=		-liberty 
endif

TARGET=		gen_dc
SOURCES=	gen_dc.c calculate_sky_patches.c  write_dds_files.c ../lib/fropen.c ../lib/read_in_header.c ../lib/sun.c \
			../lib/parse.c
DEPENDENDS=	gen_dc.c calculate_sky_patches.c calculate_sky_patches.h ../lib/fropen.c ../lib/read_in_header.c ../lib/sun.c \
			../lib/parse.c \
			write_dds_files.c 




$(TARGET):	$(DEPENDENDS)
		$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET) $(LIBS)


clean:
		rm -f *.o *~ *.bak

veryclean:	clean
		rm -f gen_dc gen_dc.exe




