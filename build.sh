#!/bin/bash

echo "Project compile has started..."

OUTPUT_FILE="project"

make clean
echo "Old compiled code is cleaned..."

echo "⚙️ compiling..."
make

if [ $? -eq 0 ]; then
    echo "compile success."
    if [ -f "$OUTPUT_FILE" ]; then
        echo "✅ $OUTPUT_FILE compiled and ready."
    else
        echo "⚠️ $OUTPUT_FILE file couldn't be found!"
        exit 1
    fi
else
    echo "❌ Compile failed."
    exit 1
fi

echo "Build script has ended."

# Default arguments if none provided
if [ $# -eq 0 ]; then
    echo "No arguments provided, running with default arguments..."
    set -- --rom-size 64 --rom-content test/rom.txt --cycles 20000 test/input.csv  --block-size 4
fi

# Run the program with arguments (default or passed)
echo "Running ./project $@"
./project "$@"
