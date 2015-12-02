#!/bin/bash

# n{esrotom,ohajan}@fit.cvut.cz
# 2015
# https://github.com/nohajc/.KEK-on-Rails
# .KEK on Rails test suite

COMPILER_VALGRIND=0 # exec compiler via valgrind
VM_VALGRIND=1 # exec compiler via valgrind
DEBUG=0 # set xev in the test suite
EXIT_ON_ERROR=0 # exit after first kek_error
FORCE_NEW_KEXE=0 # always create new kexe, don't use old ones
ROOTDIR="${0%/*}/.."
KEK_KEXE_DIR="$ROOTDIR/tests/kexes" # .kexe files
KEK_SRC_DIR="$ROOTDIR/tests" # .kek files
KEK_EXPECTED_DIR="$ROOTDIR/tests/expected" # .out files (to be cmp with)
KEK_OUT_DIR="$ROOTDIR/tests/out" # results of the executings
COMPILER_EXE="$ROOTDIR/compiler/kekc" # compiler executable
VM_EXE="$ROOTDIR/vm/kek" # virtual machine executable
VM_ARGS="" # args for kek vm (f.ex.: -d gc -d stack)
PRINT_ERR_FILES=0 # kek_err can print error file
SHOW_OUT=0 # cat .out files

kek_parseopts() {
	while getopts "cC:de:ho:V:x" opt; do
		case $opt in
		c)
			kek_compile $OPTARG
			;;
		C)
			COMPILER_VALGRIND=$OPTARG
			;;
		d)
			DEBUG=1
			;;
		e)
			EXIT_ON_ERROR=$OPTARG
			;;
		h)
			usage
			;;
		o)
			# 1 show just vm
			# 2 show all
			# SHOW_OUT=$OPTARG
			PRINT_ERR_FILES=1
			;;
		V)
			VM_VALGRIND=$OPTARG
			;;
		x)
			FORCE_NEW_KEXE=1
			;;
		\?)
			kek_error "Invalid option: -$OPTARG"
			;;
		:)
			kek_error "Option -$OPTARG requires an argument."
			;;
		esac
	done

	VALGRIND_CMD="valgrind --leak-check=full --track-origins=yes -q"

	if [[ $VM_VALGRIND == 1 ]]; then
		VM_VALGRIND_CMD="$VALGRIND_CMD"
	else
		VM_VALGRIND_CMD=""
	fi

	if [[ $COMPILER_VALGRIND == 1 ]]; then
		COMPILER_VALGRIND_CMD="$VALGRIND_CMD"
	else
		COMPILER_VALGRIND_CMD=""
	fi
}

usage() {
	cat <<__EOF__
.KEK on Rails test suite!
usage: see the first 50 lines of source of $0 :D
__EOF__
	exit 0
}

kek_compile() {
	local kek_name=$1
	local out=$KEK_OUT_DIR/${kek_name}.compile.out

	$COMPILER_VALGRIND_CMD $COMPILER_EXE $KEK_SRC_DIR/${kek_name}.kek \
	    $KEK_KEXE_DIR/${kek_name}.kexe >$out 2>&1
	if (( $? != 0 )); then
		kek_error "compiling of $kek_name has failed" $out
	elif [[ $SHOW_OUT == 2 ]]; then
		cat $out
	fi
}

kek_execute() {
	[[ $DEBUG == 1 ]] && set -xv

	local kek_name=$1
	local out=$KEK_OUT_DIR/${kek_name}.vm.out
	local kek=$KEK_SRC_DIR/${kek_name}.kek
	local kexe=$KEK_KEXE_DIR/${kek_name}.kexe
	local ok=0
	local fail=0
	local skipped=0

	if [[ $FORCE_NEW_KEXE == 1 ]] || \
	    [[ ! -f $kexes/$kek_name.kexe ]]; then
		kek_compile $kek_name
	fi

	runs="$(grep "//KEK_TEST_RUNS" $kek | cut -d ' ' -f 2)"
	if [[ -z $runs ]]; then
		return
	fi

	for (( i=0; i < $runs; i++ )); do
		ok=0
		fail=0
		skipped=0

		args="$(grep "//KEK_TEST_ARGS_$i" $kek | \
		    sed "s/\/\/KEK_TEST_ARGS_$i //")"
		#echo "$i \"$args\""

		out=$KEK_OUT_DIR/${kek_name}.vm.${i}.out
		$VM_VALGRIND_CMD $VM_EXE -t 1 $VM_ARGS $kexe $args >$out 2>&1
		if (( $? != 0 )); then
			kek_error "exexuting of $kek_name has failed" $out
			(( fail++ ))
		else
			(( ok++ ))
		fi

		if [[ -f $KEK_EXPECTED_DIR/${kek_name}.stdout.${i}.exp ]]; then
			diffout=$KEK_OUT_DIR/${kek_name}.stdout.${i}.diff
			diff $KEK_EXPECTED_DIR/${kek_name}.stdout.${i}.exp \
				<(grep -v "@dbg" $out) >$diffout 2>&1
			if (( $? != 0 )); then
				kek_error "stdout exp of $kek_name run $i has failed" \
				    $diffout
				(( fail++ ))
			else
				(( ok++ ))
			fi
		else
			(( skipped++ ))
		fi

		kek_ok "${kek_name} run=$i" $ok $fail $skipped
	done

	[[ $DEBUG == 1 ]] && set +xv
}

#colors
cred="\x1B[31m"
cgreen="\x1B[32m"
cblue="\x1B[34m"
cnormal="\x1B[0m"

kek_error() {
	local error_msg=${1:-"no error msg"}
	local error_file=${2:-"no_file"}
	echo "kek_error: $1" >&2

	if [[ $PRINT_ERR_FILES == 1 ]] && [[ "$error_file" != "no_file" ]]; then
		echo "ERROR FILE BEGIN: $error_file"
		cat $error_file
		echo "ERROR FILE END: $error_file"
	fi
	
	if [[ $EXIT_ON_ERROR == 1 ]]; then
		exit 1
	fi
}

kek_ok() {
	local msg=$1
	local ok=$2
	local fail=$3
	local skipped=$4

	echo -e "KEK_RESULT \"$msg\" OK=$cgreen$ok$cnormal " \
	    "FAIL=$cred$fail$cnormal SKIPPED=$skipped"
}

kek_make() {
	[[ $DEBUG == 1 ]] && set -xv

	local out_compiler=$KEK_OUT_DIR/make_compiler.out
	local out_vm=$KEK_OUT_DIR/make_vm.out
	local ok=0
	local fail=0

	make -C compiler >$out_compiler 2>&1
	if (( $? != 0 )); then
		kek_error "make compiler failed" $out_compiler
		(( fail++ ))
	else
		(( ok++ ))
	fi
	if [[ ! -f $COMPILER_EXE ]]; then
		kek_error "-f compiler exe failed"
		(( fail++ ))
	else
		(( ok++ ))
	fi

	make -C vm >$out_vm 2>&1
	if (( $? != 0 )); then
		kek_error "make vm failed" $out_vm
		(( fail++ ))
	else
		(( ok++ ))
	fi
	if [[ ! -f $VM_EXE ]]; then
		kek_error "-f vm exe failed"
		(( fail++ ))
	else
		(( ok++ ))
	fi

	kek_ok "make" $ok $fail 0

	[[ $DEBUG == 1 ]] && set +xv
}

kek_clear_out() {
	mkdir -p $KEK_OUT_DIR
	mkdir -p $KEK_KEXE_DIR
	rm -f $KEK_OUT_DIR/*.out
}

main() {
	kek_clear_out
	kek_make

	for k in $KEK_SRC_DIR/*.kek; do
		ftmp=${k##*/}
		f=${ftmp%.*}
		kek_execute $f
	done

}

# MAIN #########################################################################

kek_parseopts "$@"

if [[ $DEBUG == 1 ]]; then
	set -xv
fi

main

if [[ $DEBUG == 1 ]]; then
	set +xv
fi

