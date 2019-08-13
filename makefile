myls:myls.c liblist.a
	gcc myls.c ./liblist.a -g -o myls -Wall
#liblist.a: list.o
#	ar -rc liblist.a  list.o
#list.o:list.c list.h
#	gcc -g -c list.c -o list.o -Wall
.PHONY:clean
clean:
	rm  myls   
