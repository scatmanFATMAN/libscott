all:
	cd src/ && $(MAKE)
	cd doc/ && $(MAKE)

clean:
	cd src/ && $(MAKE) clean
	cd doc/ && $(MAKE) clean
