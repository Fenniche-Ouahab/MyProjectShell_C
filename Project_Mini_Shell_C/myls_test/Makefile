#
# Makefile de l implementation de ls
#
all: myls

clean:
	rm -f myls

myls: main.c
	gcc -Wall main.c print.c utility.c -o myls -D_GNU_SOURCE
	# D_GNU_SOURCE est pour la fonction get_current_dir_name ()
