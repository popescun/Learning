## Fundamental types

The following types are collectively called `fundamental types`:
- (possibly cv-qualified) `void`
- (possibly cv-qualified) `std::nullptr_t`(since C++11)
- `integral types`
- `floating-point types`

### Data models

The choices made by each implementation about the sizes of the fundamental types are collectively known as `data model`. Four data models found wide acceptance:

32 bit systems:
- LP32 or 2/4/4 (int is 16-bit, long and pointer are 32-bit)
    - Win16 API
- ILP32 or 4/4/4 (int, long, and pointer are 32-bit);
    - Win32 API
    - Unix and Unix-like systems (Linux, macOS)

64 bit systems:

- LLP64 or 4/4/8 (int and long are 32-bit, pointer is 64-bit)
    - Win32 API (also called the Windows API) with compilation target 64-bit ARM (AArch64) or x86-64 (a.k.a. x64)
- LP64 or 4/8/8 (int is 32-bit, long and pointer are 64-bit)
    - Unix and Unix-like systems (Linux, macOS)