# TODO
- ~~Improve the ability to call functions anytime (in other words, `pvm_run()` should not assume it's doing everything, the VM can be used to call functions any time while it's not freed.) (required for proper libuv support)~~ Somewhat done, not satisfied though.
- Add classes/instances
- Add modules and import (needs classes done first for the property support in the compiler.)
- Add builtin modules. (Right now we expose random globals which should be refactored when we have actual modules.)
- Wrap libuv. (Currently we have partial libuv support, very dirty though)
- Support escape sequences in strings.
- Fix libuv GC issues. (The VM overall is not prepared with how libuv works so the behaviour is terrible and easily crashes.) (update: i kind of fixed the main issue see `src/tests/asynccl.pd` but there might be other hidden stuff.)
- Allow NaN tagging to be disabled? For supporting some edge case hardware i guess? I'm not sure where will NaN tagging fail. I'll consider it after finding a reason to do so.
- Fix libuv REPL issues, `setTimeout` etc blocks because we run the event loop every line, we will need the language to become more mature and implement the REPL in the language itself so the event loop can be managed better.

## Even more things to do.
See also the source code for comments containing `TODO`, on linux i usually run this command to quickly find them (inside `src/`)
```sh
$ find . -type f -exec grep -i --color=always 'TODO' \{\} +
```
It recursively checks all files for the pattern `TODO` (case-insensitive) and returns those comments along with their filename.

Only the major discussions are written in this list other TODOs that are directly related to the code is almost always written as TODO comments in the source file itself.
