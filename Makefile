all:
	make -C compiler
	make -C vm
	make -C kek_scheme
	
run:
	(cd kek_scheme && ../vm/kek.exe scheme.kexe)

clean:
	make -C compiler clean
	make -C vm clean
	make -C kek_scheme clean

