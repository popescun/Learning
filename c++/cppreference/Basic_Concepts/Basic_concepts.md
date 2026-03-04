A C++ program is a sequence of text files (typically header and source files) that contain `declarations`. They undergo `translation` to become an executable program, which is executed when the C++ implementation calls its `main` function.

Certain words in a C++ program have special meaning, and these are known as `keywords`. Others can be used as `identifiers`. `Comments` are ignored during translation. C++ programs also contain `literals`, the values of characters inside them are determined by `character sets and encodings`. Certain characters in the program have to be represented with `escape sequences`.

The entities of a C++ program are `values`, `objects`, `references`, `structured bindings`(since C++17), `result bindings`(since C++26), `functions`, `enumerators`, `types`, `class members`, `templates`, `template specializations`, `packs`(since C++11), and `namespaces`. Preprocessor `macros` are not C++ entities.

`Declarations` may introduce entities, associate them with names and define their properties. The declarations that define all properties required to use an entity are `definitions`. A program must contain only one definition of any non-inline function or variable that is `odr-used`.
```cpp
ODR = One Definition Rule
```

#### ODR-use (informal definition)
- An object is odr-used if its value is read (unless it is a compile time constant) or written, its address is taken, or a reference is bound to it,

- A reference is odr-used if it is used and its referent is not known at compile time,

- A function is odr-used if a function call to it is made or its address is taken.

- If an entity is odr-used, its definition must exist somewhere in the program; a violation of that is usually a link-time error.

`Definitions` of functions usually include sequences of `statements`, some of which include `expressions`, which specify the computations to be performed by the program.

Names encountered in a program are associated with the declarations that introduced them using `name lookup`. Each name is only valid within a part of the program called its `scope`. Some names have `linkage` which makes them refer to the same entities when they appear in different scopes or translation units.

Each object, reference, function, expression in C++ is associated with a `type`, which may be `fundamental`, compound, or `user-defined`, complete or `incomplete`, etc.

Declared objects and declared references that are `not non-static data members` are `variables`.