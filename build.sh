#!/bin/env bash

cmd=$1

# Script Configuration
version="v1.0.0"
source_files=(
	"KawaiServer.java"
)
source_dir="./src"
build_dir="./build"
name="KawaiServer.jar"
entry_point="KawaiServer" # Use KawaiServer.class as the entry point

### --[Helper functions]--

print_msg() {
	RESET='\e[0;0m'
	BLACK='\e[0;30m'
	RED='\e[0;31m'
	GREEN='\e[0;32m'
	YELLOW='\e[0;33m'
	BLUE='\e[0;34m'
	PURPLE='\e[0;35m'
	CYAN='\e[0;36m'
	WHITE='\e[0;37m'

	local color=$1
	local message=$2
	
	case "$color" in
		"black")
			selected_color=$BLACK
		;;
		"red")
			selected_color=$RED
		;;
		"green")
			selected_color=$GREEN
		;;
		"yellow")
			selected_color=$YELLOW
		;;
		"blue")
			selected_color=$BLUE
		;;
		"purple")
			selected_color=$PURPLE
		;;
		"cyan")
			selected_color=$CYAN
		;;
		"white")
			selected_color=$WHITE
		;;
	esac

	echo -en "${selected_color}${message}${RESET}"
}


### --[Main logic of this script]--

print_msg "white" "### BUILD SCRIPT FOR LINUX ###\n"
print_msg "white" "## branch: $(git --no-pager branch --show-current)\n"
print_msg "white" "## name: $name\n"
print_msg "white" "## ver: $version\n"
print_msg "" "\n"

if test "$cmd" == "compile"; then
	
	if ! test -f "$(command -v java)" ||
	   ! test -f "$(command -v javac)" ||
	   ! test -f "$(command -v jar)"; then
		print_msg "red" "Is the java package installed?\n"
		exit 1
	fi

	if test -d $build_dir; then
		print_msg "yellow" "$build_dir directory is already exists. Re-creating\n"
		rm -fr $build_dir
		mkdir $build_dir
	else
		mkdir $build_dir
		print_msg "yellow" "New directory named $build_dir created\n"
	fi
	
	for i in $(seq 0 $((${#source_files[@]}-1))); do
		source_file=${source_files[$i]}
		src_file_path="$source_dir/$source_file"
		dest_file_path="$build_dir/$source_file"

		if test -e $source_file; then
			print_msg "white" "   $src_file_path -> $dest_file_path : Not exists\n"
			exit 1
		fi
		
		# If the source file is java
		if test ${src_file_path:${#src_file_path}-4} == "java"; then
			print_msg "white" "   $src_file_path -> ${dest_file_path:0:${#dest_file_path}-5}.class : "
			
			output_status=$(javac $src_file_path -d $build_dir -cp $source_dir 2>&1)
			if test $? -eq 0; then
				print_msg "green" "Success\n"
			else
				print_msg "red" "Failed\n"
				print_msg "white" "$output_status"
				exit 1
			fi

		# If the source file is non-java
		else
			mkdir -p ${dest_file_path%/*}
			cp -fr $src_file_path $dest_file_path
			print_msg "yellow" "   $src_file_path -> $dest_file_path : Copied\n"
		fi
	done
	
	print_msg "" "\n"
	cd $build_dir
	jar -vcfe $name $entry_point *
	if test $? -eq 0; then
		print_msg "green" "Success to create the jar archive file from $build_dir/*\n"
		mv -f $name ..
		cd ..
	else
		print_msg "red" "Failed to create the jar archive file from $build_dir/*\n"
		cd ..
		exit 1
	fi

	print_msg "" "\n"
	print_msg "yellow" "Compilation successfully :3\n"
	print_msg "white" "try run \"java -jar $name\"\n"

elif test "$cmd" == "clean"; then
	if test -d $build_dir; then
		rm -fr $build_dir
		print_msg "yellow" "$build_dir directory removed\n"
	else
		print_msg "yellow" "$build_dir directory isn't exists. SKIP\n"
	fi

	if test -f $name; then
		rm -fr $name
		print_msg "yellow" "$name removed\n"
	else
		print_msg "yellow" "$name isn't exists. SKIP\n"
	fi
else
	print_msg "red" "What the hell \"$cmd\" command?\n"
	print_msg "yellow" "There are only \"compile\" and \"clean\" command\n"
fi
