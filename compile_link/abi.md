# ABI

ABI is about how the compiler builds an application. It defines things (but is not limited to):

* How parameters are passed to functions. using registers or stack?
* Who cleans parameters from the stack. caller or callee?
* Where the return value is placed for return.
* How exceptions propagate.

C和C++语言都没有标准化的ABI规范，不同编译器（或不同版本）、操作系统之间的ABI都可能出现不一致。调用预编译发布的C/C++动态链接库的风险也来源于ABI兼容性。安全的做法是所有依赖从源码重新编译。

One easy way to understand "ABI" is to compare it to "API".

You are already familiar with the concept of an API. If you want to use the features of, say, some library or your OS, you will use an API. The API consists of data types/structures, constants, functions, etc that you can use in your code to access the functionality of that external component.

An ABI is very similar. Think of it as the compiled version of an API (or as an API on the machine-language level). When you write source code, you access the library through an API. Once the code is compiled, your application accesses the binary data in the library through the ABI. The ABI defines the structures and methods that your compiled application will use to access the external library (just like the API did), only on a lower level.

ABIs are important when it comes to applications that use external libraries. If a program is built to use a particular library and that library is later updated, you don't want to have to re-compile that application (and from the end-user's standpoint, you may not have the source). If the updated library uses the same ABI, then your program will not need to change. The interface to the library (which is all your program really cares about) is the same even though the internal workings may have changed. Two versions of a library that have the same ABI are sometimes called "binary-compatible" since they have the same low-level interface (you should be able to replace the old version with the new one and not have any major problems).

Sometimes, ABI changes are unavoidable. When this happens, any programs that use that library will not work unless they are re-compiled to use the new version of the library. If the ABI changes but the API does not, then the old and new library versions are sometimes called "source compatible". This implies that while a program compiled for one library version will not work with the other, source code written for one will work for the other if re-compiled.

For this reason, library writers tend to try to keep their ABI stable (to minimize disruption). Keeping an ABI stable means not changing function interfaces (return type and number, types, and order of arguments), definitions of data types or data structures, defined constants, etc. New functions and data types can be added, but existing ones must stay the same. If you expand, say, a 16-bit data structure field into a 32-bit field, then already-compiled code that uses that data structure will not be accessing that field (or any following it) correctly. Accessing data structure members gets converted into memory addresses and offsets during compilation and if the data structure changes, then these offsets will not point to what the code is expecting them to point to and the results are unpredictable at best.

An ABI isn't necessarily something you will explicitly provide unless you are expecting people to interface with your code using assembly. It isn't language-specific either, since (for example) a C application and a Pascal application will use the same ABI after they are compiled.

Edit: Regarding your question about the chapters regarding the ELF file format in the SysV ABI docs: The reason this information is included is because the ELF format defines the interface between operating system and application. When you tell the OS to run a program, it expects the program to be formatted in a certain way and (for example) expects the first section of the binary to be an ELF header containing certain information at specific memory offsets. This is how the application communicates important information about itself to the operating system. If you build a program in a non-ELF binary format (such as a.out or PE), then an OS that expects ELF-formatted applications will not be able to interpret the binary file or run the application. This is one big reason why Windows apps cannot be run directly on a Linux machine (or vice versa) without being either re-compiled or run inside some type of emulation layer that can translate from one binary format to another.

IIRC, Windows currently uses the Portable Executable (or, PE) format. There are links in the "external links" section of that Wikipedia page with more information about the PE format.

Also, regarding your note about C++ name mangling: The ABI can define a "standardized" way for a C++ compiler to do name mangling for the purpose of compatibility. That is, if I create a library and you develop a program that uses the library, you should be able to use a different compiler than I did and not have to worry about the resulting binaries being incompatible due to different name mangling schemes. This is really only of use if you are defining a new binary file format or writing a compiler or linker.

## References

* [什么是ABI](https://stackoverflow.com/questions/2171177/what-is-an-application-binary-interface-abi)
* [DSO How TO](ref/dsohowto.pdf)