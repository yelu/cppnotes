# 左值和右值

## RVO v.s. std::move

[[link]](https://isocpp.org/blog/2015/07/rvo-v.s.-stdmove-zhao-wu)

To summarize, RVO is a compiler optimization technique, while std::move is just an rvalue cast, which also instructs the compiler that it’s eligible to move the object. The price of moving is lower than copying but higher than RVO, so never apply std::move to local objects if they would otherwise be eligible for the RVO.

## References

Thomas Becker
* [Introduction](http://thbecker.net/articles/rvalue_references/section_01.html)
* [Move Semantics](http://thbecker.net/articles/rvalue_references/section_02.htmll)
* [Rvalue References](http://thbecker.net/articles/rvalue_references/section_03.html)
* [Forcing Move Semantics](http://thbecker.net/articles/rvalue_references/section_04.html)
* [Is an Rvalue Reference an Rvalue?](http://thbecker.net/articles/rvalue_references/section_05.html)
* [Move Semantics and Compiler Optimizations](http://thbecker.net/articles/rvalue_references/section_06.html)
* [Perfect Forwarding: The Problem](http://thbecker.net/articles/rvalue_references/section_07.html)
* [Perfect Forwarding: The Solution](http://thbecker.net/articles/rvalue_references/section_08.html)
* [Rvalue References and Exceptions](http://thbecker.net/articles/rvalue_references/section_09.html)
* [The Case of the Implicit Move](http://thbecker.net/articles/rvalue_references/section_10.html)
* [Acknowledgments and Further Reading](http://thbecker.net/articles/rvalue_references/section_11.html)

Microsoft
* [Rvalue Reference Declarator](https://docs.microsoft.com/en-us/cpp/cpp/rvalue-reference-declarator-amp-amp?view=msvc-160) [[cache]](ref/Rvalue_Reference_Declarator.html)
