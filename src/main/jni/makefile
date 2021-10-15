#CC=g++ 
CC=g++ 
#CC=clang++
LIB=  -L. -lz  
INC = -I.
CFLAGS +=  -g  -w -Wall -pthread -fPIC $(INCINCPATH) 
SHAREDLIB = libdownload.so
OBJS=  sha1.o lzss.o usbserial.o nlddownload.o arddownload.o nlcdownload.o inirw.o function.o arq.o nlpdownload.o   rsa.o codetools.o nn.o \
des3des.o  nld.o  downloader.o

all:$(SHAREDLIB)
$(SHAREDLIB): $(OBJS)
	$(CC)  -pthread -shared -Wl,-soname,libdownload.so.1 -o libdownload.so  $(OBJS)
	chmod +x libdownload.so
	cp libdownload.so ../build-Downloader-Desktop_Qt_5_9_9_GCC_64bit-Debug



.cpp.o:
	$(CC) $(CFLAGS)  -c $< -o $@ $(INC)
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJS) $(SHAREDLIB)
	# mac rm -f *.cpp
	# mac rm -f *.h

	
copyto:
	cp Downloader_Linux /mnt/hgfs/linux_download/
	
	
from:
	umount /mnt/hgfs/linux_sdk
	mount /mnt/hgfs/linux_sdk
	cp  -r /mnt/hgfs/linux_sdk/dl/*  .
	# cp  -r /Volumes/VMware\ Shared\ Folders/dl/* .
	
to:
	tar cvfz dl.tar.gz Language dl

	cp dl.tar.gz /Volumes/VMware\ Shared\ Folders/dl/dis/
install:
	cp Downloader_Linux test/Downloader_Linux


