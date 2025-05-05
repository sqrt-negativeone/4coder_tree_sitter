
#!/bin/bash

# If any command errors, stop the script
set -e

# Store the real CWD
ME="$(readlink -f "$0")"
LOCATION="$(dirname "$ME")"
CODE_HOME="$(dirname "$LOCATION")"

# Find the most reasonable candidate build file
SOURCE="$1"
if [ -z "$SOURCE" ]; then
    SOURCE="$(readlink -f "$CODE_HOME/4coder_default_bindings.cpp")"
fi
echo SOURCE = $SOURCE

extra_libs=$2
echo "extra libs: $extra_libs"

opts="-Wno-write-strings -Wno-null-dereference -Wno-comment -Wno-switch -Wno-missing-declarations -Wno-logical-op-parentheses -g -DOS_LINUX=1 -DOS_WINDOWS=0 -DOS_MAC=0"
arch=-m64

debug=-g

preproc_file=4coder_command_metadata.i
meta_macros="-DMETA_PASS"
g++ -I"$CODE_HOME" $meta_macros $arch $opts $debug -std=c++11 "$SOURCE" -E -o $preproc_file

echo bulding metaprogram
g++ -I"$CODE_HOME" $opts $debug -std=c++11 "$CODE_HOME/4coder_metadata_generator.cpp" -o "$CODE_HOME/metadata_generator"

echo running meta program
"$CODE_HOME/metadata_generator" -R "$CODE_HOME" "$PWD/$preproc_file"

echo meta program pass done

g++ -I"$CODE_HOME" $arch $opts $debug -std=gnu++0x "$SOURCE" $extra_libs -shared -o custom_4coder.so -fPIC

rm "$CODE_HOME/metadata_generator"
rm $preproc_file