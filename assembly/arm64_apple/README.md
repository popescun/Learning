## How compile .s to executable using `clang`
```shell
clang hello_world.s -o hello_world
``` 
Without specifying the output file(`-o` option) it will create a 
default file `o.out`.

## How compile .s file to executable using assembler `as` and linker `ld`
```shell
 as hello_world.s -o hello_world.o
ld -o hello_world.x hello_world.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path`
```

## How compile with debug information
Pass `-g` option to `clang` or `as` command. Notice `clang` generates and additional folder `.dSYM` on
`macos`, while `as` doesn't. 
```shell
as hello_world.s -o hello_world.o -g
```
and then link it as above. We can use `lldb` to debug the executable:
```shell
lldb ./hello_world.x

(lldb) target create "./hello_world_2.x"
Current executable set to '/Users/popescun/github/Learning/assembly/arm64_apple/hello_world_2.x' (arm64).
```
Put a breakpoint on `main` function:
```shell
(lldb) b main
Breakpoint 1: where = hello_world_2.x`main + 4, address = 0x00000001000002dc
```
Run the process
```shell
(lldb) run
Process 4449 launched: '/Users/popescun/github/Learning/assembly/arm64_apple/hello_world_2.x' (arm64)
Process 4449 stopped
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
    frame #0: 0x00000001000002dc hello_world_2.x`main at hello_world_2.s:7
   4   	_main:
   5   	  // print
   6   	  mov x0, #1 // stdout = `1`
-> 7   	  adr x1, hello_world // adr = address to vector
   8   	  mov x2, 14 // 14 characters in our string
   9   	  mov x16, #4 // 4 is write syscall
   10  	  svc #0x80 // supervisor call
Target 0: (hello_world_2.x) stopped.
```
Print the registers:
```shell
(lldb) p $x0
(unsigned long) 1
```
Run next instruction:
```shell
(lldb) ni
Process 6363 stopped
* thread #1, queue = 'com.apple.main-thread', stop reason = instruction step over
    frame #0: 0x00000001000002e0 hello_world_2.x`main at hello_world_2.s:8
   5   	  // print
   6   	  mov x0, #1 // stdout = `1`
   7   	  adr x1, hello_world // adr = address to vector
-> 8   	  mov x2, 14 // 14 characters in our string
   9   	  mov x16, #4 // 4 is write syscall
   10  	  svc #0x80 // supervisor call
   11  	
Target 0: (hello_world_2.x) stopped.
```

