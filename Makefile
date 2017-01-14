interact : interact.cxx
	g++ -Wall -Wundef -Wunused -Wshadow -D_DEBUG -g -O2 -o $@ $< -lusb-1.0 -lrt

clean ::
	rm -f interact

device :
	lsusb -vvv -d a0a0:000a

toggle : interact
	./interact
