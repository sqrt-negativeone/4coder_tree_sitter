#!/bin/bash

code_dir="$PWD"

mkdir -p $code_dir/bld
cd $code_dir/bld > /dev/null

if [ ! -f "libtree-sitter.a" ]; then
	# Set path variables
	tree_sitter_root="${code_dir}/code/tree_sitter/"
	include_paths="-I${tree_sitter_root}include/ -I${tree_sitter_root}../"

	echo $tree_sitter_root

	langs_root_dir="${code_dir}/langs"

	# Clean up previous object files
	rm -f *.o

	# Build tree-sitter lib
	echo "Building tree-sitter lib"
	gcc -c -O2 ${include_paths} -fPIC "${tree_sitter_root}src/lib.c" -o tree-sitter-lib.o

	# NOTE: add parsers and scanners for languages here

	echo "Building parsers"

	# Iterate through language directories
	for dir in "${langs_root_dir}/"*/; do
  	  # Extract directory name (language name)
    	dir_name=$(basename "$dir")
    	echo "Building parser for ${dir_name}:"
    
    	# Compile each file in the language directory
    	for file in "${dir}"*.*; do
      	  if [[ -f "$file" && ! "$file" =~ ".scm" ]]; then
        	    file_name=$(basename "$file")
          	  outname="tree-sitter-${dir_name}-${file_name}.o"
            	gcc -c -O2 ${include_paths} -fPIC "$file" -o "$outname"
        	fi
    	done
	done

	# Create static library from all object files
	echo "Creating library"
	
	ar rcs libtree-sitter.a *.o
	rm -f *.o
	echo done building tree sitter library
fi

$code_dir/custom/bin/tree_sitter_buildsuper_x64-linux.sh $code_dir/code/4coder_tree_sitter.cpp $PWD/libtree-sitter.a 

cd $code_dir > /dev/null