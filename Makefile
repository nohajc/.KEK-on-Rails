all:
	make -C compiler
	make -C vm
	make -C kek_scheme
	
run:
	vm/kek kek_scheme/scheme.kexe

clean:
	make -C compiler clean
	make -C vm clean
	make -C kek_scheme clean

