# Building Peridot
Building Peridot is easy and takes a few seconds if you already have the neccessary stuff.

Prebuilt binaries are currently not available as Peridot is being developed but will be available once stable.

## Dependencies
You need the following installed in your machine to build Peridot.

- `git` Chances are you already have it, only needed to fetch source code, you can fetch source code in a different way as well but git is needed if you want to contribute.
- `libuv` Peridot depends on libuv for cross-platform asynchronous event loop.
- ~~`pcre2`~~ Regexp engine, currently not needed but is a plan for the future.
- A C compiler, `gcc`/`clang` should be fine, `msvc` should also work but isn't tested.
- `make` If on Unix (OS X, Linux, BSD)
- `flex`/`bison` Only needed if you change the Lexer/Parser files, we provide the generated code in the source to ease building.
- `Python3.6+` Only if you plan to contribute, we might use some Python scripts here and there to help development but is not required.

## Clone
First clone the repository no matter what platform you are on.
```sh
$ git clone https://github.com/peridot/peridot
$ cd peridot
```
If you don't have Git and don't plan to contribute then there is an alternative way to fetch the source via downloading from GitHub from the browser then extracting the Zip file, git is recommended because it will also make it easy to update.

Now follow instructions for your platform:

## Unix (gcc/clang)
On all Unix platforms (Linux, OS X, BSD (or Windows with Cygwin/MinGW)) with an installation of `gcc` or `clang` Building is as easy as running:
```sh
$ make
```
Afterwards run the tests to make sure everything is good to go.
```sh
$ make test
```
If something failed, either you did something wrong or we did something wrong, open an issue to discuss.

## Windows
Unfortunately I'm not experienced with building C/C++ stuff on Windows, following the Unix instructions with CygWin or MinGW might work for now.

Windows support is mostly untested i try my best in the code for it to work on Windows but i can't actively test it.

If you have experience with Windows please help in writing a proper msvc build script afterwards we can test it via CI.
