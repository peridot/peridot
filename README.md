# The Peridot Programming Language
Peridot is a dynamically typed, interpreted and embeddable programming language.

It is aimed for easy concurrency, simplicity and speed.

**Features**

*Note: Peridot is still under heavy development, so not all of this features might be true but it is how we want the final design to look like*

- **Embeddable:** Peridot aims to be embeddable with a simple to use C API, building it is also quick and painless.
- **Garbage Collected:** Peridot is garbage collected meaning you never have to worry about leaking memory.
- **Easy to learn & use:** Peridot's syntax is easy to get used to in about an hour.
- **Big standard library:** Peridot comes with a lot of tools to avoid reinventing the wheel while keeping them simple and small
- **Dynamically typed:** Peridot is dynamically typed so you don't have to keep writing types, focus more on the code.
- **Fast:** Peridot uses a Bytecode Virtual Machine (PVM) to run code.
- **JIT:** Peridot has plans to do Just-In-Time compilation (PDJIT) in the future. (without LLVM)
- **Tools:** Comes packed with tools to make development easier, documentation generator/browser and a code formatter.
- **Asynchronous:** Peridot uses [libuv](https://libuv.org) for Asynchronous event-driven programming.
- **Portable:** Peridot is portable across Windows, Mac and Linux and code written in it will work across platforms.
- **Low Memory:** Peridot's VM itself uses very low memory.
- **Well documented:** Both the language and it's source code are well documented and it's easy to use & contribute.
- **Packages:** Peridot makes it easy to develop and publish re-useable packages.
- **Object Orientated:** Peridot has first class support for classes/objects, they are made as lightweight as possible.

## Look & Feel
```pd
# Printing text
println("Hello, World!")

# Functions
function sum(x, y)
  return x + y
end

# String interpolation
println("5 + 2 = #{sum(5, 2)}")

# Modules
import "math"
println(math.sin(30))

# Classes
class Person
  function init(name, age)
    this.name = name
    this.age = age
  end

  function speak(text)
    println("#{this.name}: #{text}")
  end
end

john = Person("John", 16)
john.speak("Hello, World!")
```
Run it with
```sh
$ peridot main.pd
```
<!--
(to be discussed for the future.)

Or precompile it for faster startup
```sh
$ peridot -c main.pd
$ perdot main.pi
```
-->
A REPL (Read-Eval-Print-Loop) is also available
```sh
$ peridot
```

<!--
Peridot uses the following file extensions

- **.pd** Peridot source code.
- **.pi** Peridot instructions for use with the VM.
- **.phtml** HTML template with embedded peridot code. See `peridot docs template`
-->

<!-- (to be discussed for the future)
### Deploying
Deploying is as easy as
```
$ peridot pack main.pd
# Run the output on the target
$ peridot main.pi
```
`pack` packs a bytecode file but it also packs all it's dependencies this is a little slower but avoids having to install dependencies where you want to deploy it, by slow i mean slower to pack it compared to `-c` flag but it is faster at runtime because it doesn't need to load dependencies from disk.

> **Note:** Packing isn't the same as a traditional AOT (Ahead-of-time) compiler, the code is still ran with Peridot's virtual-machine and you still have access to the compiler so dynamic `eval()` is still allowed.
-->

## Interested? Install today!
<!-- (for the future) - Precompiled binaries can be found on our website at [peridot.io/download](https://peridot.io/download) -->
- Instructions to build from source is available at [BUILDING.md](BUILDING.md)

Prebuilt binaries will be available once we have a stable release.

<!--
Embedding is not ready, this a dummy example of how i expected it to look like.

## Embedding
Embedding is as simple as
```c
#include <stdio.h>
#include <peridot/peridot.h>
#include <peridot/pvm.h>

int main() {
  pvm_t* vm = pvm_new();
  pd_run_str(vm, "println(5)");

  // Compile and run later
  pvm_bc* bc = pd_compile("println(5)");
  pvm_exec_bc(vm, bc);

  // Save to disk for later use and reload with pvm_load_bc and use pvm_exec_bc as normal.
  pvm_write_bc_to_file(bc, "hello.ei");

  pvm_free_bc(bc);
  pvm_free(vm);
}
```
Peridot's namespace is safe from name clashes:
- Header guards are in the form `_PERIDOT_FILENAME_H` where `FILENAME` is the filename.
- Macros are prefixed with `PERIDOT_` or `PD_` (`PVM_` for the VM, `PDJIT_` for the JIT)
- Methods and types are prefixed with `pd_` (`pvm_` for the VM, `pdjit_` for the JIT)

For more information on embedding see [docs/Embedding.md](docs/Embedding.md)
-->

## Documentation
Right now peridot has partial documentation in the [docs](docs/) directory, soon it will be put in a fancy website once stable.

## Contributing
Peridot is still under heavy development, if you wish to contribute see [CONTRIBUTING.md](CONTRIBUTING.md)

### Discord
Peridot has a Discord Server for the community, join the server for discussing ideas or just some general conversations! We like to meet new people so [come and say hi](https://discord.gg/56PqR9d)

### Internals
Peridot documents a lot of it's internals on how they work in comments in the source code, some other general ideas can be found at [docs/Internals.md](docs/Internals.md)

Peridot's code is also very easy to understand at this point so be quick! hop in the development team before it turns complex in the future!

We hope we made peridot as easy as possible to understand the source code and contribute.

### Bugs
Since peridot is in very early stage a lot of unexpected bugs might be there so please keep opening issues as you find them and we will try to fix them.

Using peridot in production is not recommended at this time however writing some interesting projects locally is a good idea to help us find weird bugs that we might have missed.

## Design
This language's design may rather be different, maybe you will have different needs, there's also some hardcoded limits on things that other languages would give you infinitely, now is a good time to discuss some changes that might be needed before it counts as a giant breaking change, if you hit any of the limits in idiomatic code (i.e not on purpose) then let us know and exactly what were you doing and we might consider changing the limits if seen fit.

## Special Thanks
Special Thanks to [Bob Nystrom](https://github.com/munificent) for his excellent [Crafting Interpreters](https://craftinginterpreters.com) book.

This couldn't have been possible without the help the book gives to get started, i followed Part 3 of the book for a bytecode virtual machine but started adding my own modifications as i kept following it, it was harder to modify while following the book but it was fun.

The book is still incomplete and so is my progress with it, the current progress is:

- [x] Chunks of Bytecode.
- [x] A Virtual Machine.
- [x] ~~Scanning on Demand.~~ I use my own lexer using Flex.
- [x] ~~Compiling Expressions.~~ I use my own parser using Bison and made a different compiler using an Abstract Syntax Tree.
- [x] ~~Types of values.~~ I kind of rushed and took the optimized NaN boxed values since i was researching about it previously.
- [x] Strings.
- [x] Hash Tables.
- [x] Global Variables.
- [x] Local Variables.
- [x] Jumping Back and Forth. (missing `for` loops, couldn't decide on a syntax.)
- [x] Calls and Functions.
- [x] Closures.
- [x] Garbage Collection.
- [] Classes and Instances. In Progress...
- [] Methods and Initializers.
- [] Superclasses
- [] ~~Optimization~~ (This page is not written yet in the book.)

That said, i don't want this language to be strictly tied to the book, it's a great start but eventually the code could change and become something else, something advanced.

More specifically, i have plans to replace the GC and the scoping rules (scoping is already different just the code is messy by hacking the book's code.)

## License
Peridot is licensed under the [MIT License](LICENSE)!
