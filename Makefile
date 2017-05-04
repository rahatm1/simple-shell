CFLAGS = -Wall -Wextra -pedantic -std=c11 -g
LDFLAGS = -lreadline -ltermcap
OBJS = main.o

all: shell

shell: $(OBJS)
	$(CC) $(CFLAGS) -o shell $(OBJS) $(LDFLAGS)

clean:
	rm -rf $(OBJS) shell
