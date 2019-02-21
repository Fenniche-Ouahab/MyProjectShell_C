CC = gcc
BIN = myshell
CFLAGS = -DDEBUG -Wall -pedantic -W -O2 -g
LDFLAGS =  -Wall -pedantic -W -O2 -lreadline -ltermcap -g
O =  output/dyn_string.o output/erreur.o output/hash.o output/list_fg.o output/myls.o output/myps.o output/mysh.o output/output_ps.o output/print_ls.o output/utility_ls.o output/varglob.o output/varloc.o output/wildcards.o

$(BIN): $(O)
	$(CC) $(LDFLAGS) -o $(BIN) $(O)

output/dyn_string.o: dyn_string.c common.h dyn_string.h
	$(CC) $(CFLAGS) -c dyn_string.c -o output/dyn_string.o

output/erreur.o: erreur.c erreur.h
	$(CC) $(CFLAGS) -c erreur.c -o output/erreur.o

output/hash.o: hash.c hash.h
	$(CC) $(CFLAGS) -c hash.c -o output/hash.o

output/list_fg.o: list_fg.c common.h dyn_string.h list_fg.h mysh.h
	$(CC) $(CFLAGS) -c list_fg.c -o output/list_fg.o

output/myls.o: myls.c myls.h
	$(CC) $(CFLAGS) -c myls.c -o output/myls.o

output/myps.o: myps.c myps.h output_ps.h
	$(CC) $(CFLAGS) -c myps.c -o output/myps.o

output/mysh.o: mysh.c mysh.h common.h erreur.h dyn_string.h myls.h myps.h \
 list_fg.h wildcards.h
	$(CC) $(CFLAGS) -c mysh.c -o output/mysh.o

output/output_ps.o: output_ps.c myps.h output_ps.h
	$(CC) $(CFLAGS) -c output_ps.c -o output/output_ps.o

output/print_ls.o: print_ls.c myls.h
	$(CC) $(CFLAGS) -c print_ls.c -o output/print_ls.o

output/utility_ls.o: utility_ls.c myls.h
	$(CC) $(CFLAGS) -c utility_ls.c -o output/utility_ls.o

output/varglob.o: varglob.c hash.h varloc.h varglob.h erreur.h common.h \
 dyn_string.h
	$(CC) $(CFLAGS) -c varglob.c -o output/varglob.o

output/varloc.o: varloc.c hash.h varloc.h
	$(CC) $(CFLAGS) -c varloc.c -o output/varloc.o

output/wildcards.o: wildcards.c
	$(CC) $(CFLAGS) -c wildcards.c -o output/wildcards.o

clean:
	rm -f $(O) $(BIN) *~
all:
	make clean
	make
sem:
	./clean.sh
