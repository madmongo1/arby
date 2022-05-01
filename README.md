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

# Contributing

## .gitignore

Please add any .gitignore entries you need to your local .gitignore

For example:

~/.gitignore:
```gitignore
bin/
bin64/
Win32/
x64/
.idea/
!./idea/codeStyles/*
cmake-build-*/
.vscode/
build/
__pycache__/
```

You can then enable this user-wide .gitignore by modifying the .gitconfig in your home directory:

~/.gitconfig:
```
# This is Git's per-user configuration file.
[user]
        name = <your name here>
        email = <your email here>
        signingkey = <your signing key if you have made one>

[alias]
        l = log --pretty=oneline --abbrev-commit
        co = checkout

[pull]
        rebase = true
[core]
        excludesfile = /home/rhodges/.gitignore
        
[filter "lfs"]
        clean = git-lfs clean -- %f
        smudge = git-lfs smudge -- %f
        process = git-lfs filter-process
        required = true
[init]
        defaultBranch = master
```
