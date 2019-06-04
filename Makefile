name=libscott.so

obj=buffer.o db.o lock.o scott.o

cc=gcc
cflags=`mysql_config --cflags` -D_GNU_SOURCE -fPIC -Wall -Wextra -g
ldflags=`mysql_config --libs` -pthread -shared

all: $(name)

$(name): $(obj)
	$(cc) -o $@ $^ $(ldflags)

%.o: %.c
	$(cc) -o $@ -c $< $(cflags)

clean:
	rm -f $(obj) $(name)
