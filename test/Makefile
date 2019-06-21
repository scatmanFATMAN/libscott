name=test

obj=alist.o main.o test.o

cc=gcc
#cflags=`mysql_config --cflags` -D_GNU_SOURCE -fPIC -Wall -Wextra -g
#ldflags=`mysql_config --libs` -pthread -shared
cflags=`mysql_config --cflags` -D_GNU_SOURCE -Wall -g
ldflags=`mysql_config --libs` -L./ -lscott -pthread

all: $(name)

$(name): $(obj)
	$(cc) -o $@ $^ $(ldflags)

%.o: %.c
	$(cc) -o $@ -c $< $(cflags)

clean:
	rm -f $(obj) $(name)
