all: one two two_bug.s two_bug three four four_2


one: one.c 
	gcc -o one one.c

two: two.c
	gcc -o two two.c

two_bug.s: two_bug.c
	gcc -S two_bug.c

two_bug: two_bug.s
	gcc -o two_bug two_bug.s

three: three.c
	gcc -o three three.c

four: four.c
	gcc -o four four.c

four_2: four_2.c
	gcc -o four_2 four_2.c
