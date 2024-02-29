
Lcli: Lcli.o
	ld -T Llinker.ld -static -nostdlib -o Lcli Lcli.o Lbio.o Ldiskio.o walkfunctions.o -L. -l4490

walkfunctions.o: walkfunctions.c
	gcc -Wall -c walkfunctions.c

Lcli.o: Lcli.c
	gcc -Wall -c Lcli.c
