# BrainfuckCompiler
A very simple Brainfuck compiler made for windows and x64 NASM

## How to compile
First you are going to need to install [NASM](https://www.nasm.us/) and [mingw-w64](https://www.mingw-w64.org/).
Once you have them configured correctly it should be as simple as running

`g++ main.cpp -o bfcompiler.exe`

## How to use
You can compile brainfuck code to assembly like this:

`bfcompiler.exe <path to your brainfuck file>`

The compiler will always output the assembly in the file `out.asm`

Now you need to generate the `.obj` file, link it to the c standard library and turn it into an `.exe` file.

If you are using the tools I previously linked and they are installed correctly this should work:

`nasm -f win64 out.asm`

`gcc out.obj -o <your desired executable name>.exe`
