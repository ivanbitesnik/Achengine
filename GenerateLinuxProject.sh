#!/usr/bin/bash

echo "Do you want to regenerate makefiles? [Y/n]"
read "r"

if [ "$r" = 'Y' -o "$r" = 'y' ]; then
premake5 gmake
fi

echo "Do you want to recompile the project? [Y/n]"
read "r"

if [ "$r" = 'Y' -o "$r" = 'y' ]; then
make
fi

echo "Do you want to run Achengine? [Y/n]"
read "r"

if [ "$r" = 'Y' -o "$r" = 'y' ]; then
cd bin/Debug-linux-x86_64/Sandbox
./Sandbox
else echo "chupame los dos huevos"
fi
