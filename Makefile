CC=gcc
CFLAGS=-O2 -Wall
OBJC=$(patsubst %.c,%.o,$(wildcard *.c))
TARGET_FILE=test
$(TARGET_FILE):$(OBJC)
	$(CC) $(CFLAGS) -o $(TARGET_FILE) $(OBJC)
$(OBJC):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(TARGET_FILE)

