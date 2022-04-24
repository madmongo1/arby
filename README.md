# arby

A financial arbitrage and market making robot demonstrating the following techniques and libraries:
- C++20
- Asio with C++20 coroutines
- Boost.Beast
- Boost.JSON
- Boost.Signals2
- Boost.Multiprecision
- fmtlib


In general:
Connector implementations run on a strand of the io_context.
Polymorphic operations and associated objects may run either on the same executor as the associated exchange connector, or may run on in the general thread pool configured by the main program. Objects that share an executor are _tightly coupled and logically form part of the same meta-object_. Objects that manage their own executor are loosely coupled and must take care to marshal incoming signals onto their own executor.

I am building this for my own purposes. Use at your own risk.

Contributions are welcome, but we should have a discussion first. I have a way of doing things that is informed by experience. There are other ways of doing things and organising code, and they may well be right.

