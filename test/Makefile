name=test

lib=libscott.so
obj=alist.o main.o shapefile.o test.o

cc=gcc
#cflags=`mysql_config --cflags` -D_GNU_SOURCE -fPIC -Wall -Wextra -g
#ldflags=`mysql_config --libs` -pthread -shared
cflags=`mysql_config --cflags` -D_GNU_SOURCE -Wall -g
ldflags=`mysql_config --libs` -L./ -lscott -pthread

all: $(lib) $(name)

$(name): $(obj)
	$(cc) -o $@ $^ $(ldflags)

%.o: %.c
	$(cc) -o $@ -c $< $(cflags)

$(lib):
	cd ../src && make
	cp ../src/$(lib) .

clean:
	rm -f $(obj) $(name)

scrub: clean
	rm -f $(lib) && cd ../src && make clean
