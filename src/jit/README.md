# JIT
This folder holds code related to just in time compilation, it holds native instructions we need for each arch we currently support.

Note this is nowhere near ready it is here as a boilerplate for future.

## Supported Archs
We will start from ARM to x86_64 and then a few more architectures.

## Design
The JIT will be embedded in the VM so we can continue to use the VM like normal when JIT is not supported.

I still don't have an exact idea on how to do this so this will take time and there is a chance that even the boilerplate code i placed here might have the wrong idea.

## DynASM
Currently i decided to use [LuaJIT's DynASM](https://luajit.org/dynasm.html) for encoding instructions, I'm not sure how this will go, maybe i will fail and completely ditch the idea? or maybe it's gonna be a huge success making the next fast programming language that competes with Java/C#/JavaScript/LuaJIT? I hope so.
