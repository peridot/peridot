# LibUV in Peridot
Peridot uses [libuv](https://libuv.org) for cross-platform Asynchronous I/O this document explains some information about the use of libuv.

## The UV module
Peridot programs can `import uv` and access the underlying libuv functions but is not recommended, these are the low-level bindings given from C to libuv's C API, there are usually higher-level modules that wraps this and makes it more friendlier, such as `timers` for the timers and `fs` for the filesystem operations and so on.

Never the less you can use this module if you ever need to, just keep in mind that peridot uses the default loop, creating a new loop is undefined behaviour because user programs are single-threaded and each loop must be in a seperate thread, therefore the bindings does not expose any `new_loop()` functions or similar and all functions use the default loop, the loop is launched internally by Peridot after execution of code so again there is no `uv.run` for that reason.

## Node.js and Python
If you are coming from Node.js it feels natural i decided to mimic it as closely as possible.

If you are coming from Python the only difference is the event loop is managed by the runtime and not your code so there is no need for `asyncio.get_event_loop().run_until_complete()` or similar.

Additionally another interesting project is [luvit](https://luvit.io) which brings libuv to Lua in a Node.js like manner, Peridot's style should feel natural with it.

## Printing Text to TTY/Console/Terminal (Stdout)
calls to `println()` are sent to libuv to write this allows using ANSI colors on Windows with nothing more since libuv supports that out of the box, you should've noticed that if you used Node.js and also noticed in Python and others that on Windows they print garbage.

## The REPL
Although the benifits of libuv is alot, unfortunately we couldn't get it to work in the REPL, therefore you won't be able to do Async I/O on the repl they just return and never call their callbacks.

This is planned to be fixed though once we have a way to dynamically execute code in the language itself `eval()` then we can have a REPL moduele bootstrapped in Peridot and libuv will be happy.
