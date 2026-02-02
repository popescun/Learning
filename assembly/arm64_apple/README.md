## How compile .s to executable using `clang`
```shell
clang hello_world.s -o hello_world
``` 
Without specifying the output file(`-o` option) it will create a 
default file `o.out`.

## How compile .s file to executable using assembler `as` and linker `ld`
```shell
 as hello_world.s -o hello_world.o
ld -o hello_world hello_world.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path`
```
