compile () {
	base=`basename $1 .c`
	echo "Compiling $1"
	../../build/bin/kint-gcc $1 -o $base.o
	echo "Done compiling test files. Press return to test"
	read 
	../../build/bin/klock $base.ll

}
if [ "$1" == "all" ] 
then
	for i in try*.c
	do 
		compile $i
	done
else
	compile $1
fi
