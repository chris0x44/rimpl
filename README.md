# rimpl - Redacted Implementation

This approach is a variation of the [pimpl idiom (Pointer to Implementation)](https://en.cppreference.com/w/cpp/language/pimpl) that avoids heap allocations by storing the `impl`-instance in a fixed-size buffer in the owning class.

Reasons why you may want to do this:
- Heap allocations are not allowed in your project
- Reduce memory fragmentation and allocation overhead e.g. when frequently created/copied
- Avoid the potential of a failing allocation for the pimpled structure
- Improved inlining/optimization e.g. allowing for [vectorization of rimpl-arrays](https://godbolt.org/z/Pj5hKddWj)

> [!tip]
> Though the advantages may sound pretty exciting, I'd argue that they are not very likely to make a big difference runtime-wise. This is based on the assumption that you're only likely to use this pattern to ensure abstraction of interfaces, which is rarely done in performance critical areas.
>
>_Be sure to let me know, if you find efficient ways to use this!_


## Usage
This is how you use `rimpl` in your code:


> `MyClass.hpp`
> ```cpp
> #pragma once
> #include "rimpl_declare.hpp"
>
> struct RimpledContent;
>
> class Rimpled
> {
> public:
>    Rimpled();
>    ~Rimpled();
>
>    void print() const;
>    void set_value(int value);
>    int value() const;
>
> private:
>    Rimpl::Rimpl<RimpledContent, 8>   m_impl; // <-- Wrap your redacted type in the Rimple class and provide sufficient size
> };
> ```

As when using the `pimpl` idiom, you need to ensure that there is no explicit or implicit use of the internal type in the header. The most common way this happens, is by defaulting c'tor/d'tor or assignment directly in the header. You can still use `= default` but it needs to be done in your implementation.


Source
> `MyClass.cpp`
> ```cpp
> // Note: Including rimpl_implement provides the full implementation, no additional work needed.
> #include <rimpl_implement.h>
>
> struct MyClassInternals {
>
>     MoreDetails details;
>
>     void doSomethingInternally()
>     {
>         details.spill();
>     }
> };
>
> // defaulting in the implementation works fine, this is not visible to the user of the class
> MyClass::MyClass() = default;
> MyClass::~MyClass() = default;
>
>
> void MyClass::doSomething()
> {
>     // Use the internals of MyClass
>     m_internals->doSomethingInternally();
> }
>```


## Buffer sizes and boundary checks
A potantial issue with manually providing buffer sizes is that you may end up with the wrong size.
If the buffer was too small, you'd end up with memory corruption and undefined behavior and nobody wants this.
To prevent his issue, there is a compile-time check to ensure that the buffer is guaranteed to be of sufficient size.

There are no hints or warnings for too large buffer-sizes.
Since compilers and platforms may have diffrering layouts and use of `rimpl` is intended to be portable, there is "one true buffer size".
Only sufficient size or not.

> [!tip]
> Providing a somewhate larger buffer size is a good way to future-proof for extension of the redacted type.
> Especially if ABI-stability is important to you.

### How to determine buffer size?
The simplest way is to use the safeguard against buffer overflow.
Just provide a 1 for buffer-size and the compiler will give you an error that looks something like this:
```cpp
static_assert failed: 'Buffer is too small for type size:  Rimpl::Needs_at_least<struct Rimpl::Type_size<8> >::as_buffer'
```
I know that we only look at type information if we really can't help it, but in this case it pays to take a closer look.
The buffer-check is written in a way that the type error actually reads like a sentence, if you ignore the funny punctuation.
Here it says that it a buffer of at least 8 byte is required to fit a type with size of 8.


## Trade-offs
Similar to the _pimpl idiom_ this approach comes with a collection of trade-offs.
Check to see if this fits your problem or another approach may be more helpful.

### Manual maintenance of the underlying buffer size.
Since the compiler has no way to determine the required size in the class declaration, we have to provide a fitting size manually.

This adds a slightly higher maintenace effort and a very tiny chance of an excessive buffer size being supplied.


### Slightly larger memory footprint due to alignment requirements
The internal buffer may require a bit more memory than the redacted object itself.
Reason is that the buffer is always aligned to match maximum alignment requirements.

Usually this should not be that much since the redacted type often has higher alignment needs and arrays of `rimpl`-ed objects are rare.


### Limited ABI stability
While the real _pimpl idiom_ provides full ABI-stability when changing the hidden type, a rimpled member is only ABI-stable as long as its size does not exceed the buffer size.

Providing a larger size initially to allow for extension, may reduce this limitation somewhat.


## Advantages

### Improved inlining, vectorization and locality of reference
Since the object is part of the owning class, the optimizer can take advantage of the known layout.

Activating _link-time optimization_ may provide some nice improvements in very special cases.
[Compiler Explorer Exmpale](https://godbolt.org/z/Pj5hKddWj)
