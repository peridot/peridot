# Learn Peridot
This document is a short introduction to all of peridot's syntax elements, you can quickly read this in as less as 30mins or less.

This document covers only syntax mostly, for standard library documentation there is much more possibilities to learn at [stdlib docs](https://peridot.io/docs/stdlib)

This document expects previous programming experience with other languages as we won't reintroduce the concepts like "what is an if-statement?"

## Printing
Printing in peridot is simple using the builtin `println` function.
```rb
println("Hello, World!")
```
As the name says `println` = `print line` it terminates the output with a `\n` (newline) if you don't want that use `print` instead
```rb
print("Hello, ")
print("World!")
print("\n")
```
The output here is same as the way we used `println`

## Comments
Comments in Peridot is the same as in Python and Ruby
```rb
# This is a comment
```
There is currently no multiline comments syntax (I'm still deciding on this)

Personally i never use multiline comments to write comments i just use multiple single lines, but i do use it to comment out a block of  code to disable it termporarily so i would want this in Peridot but feedback is required on how the syntax should look (preferebly something that's **not** like Ruby's `=begin`/`=end` syntax, it's just ugly and i never use it.)

## Variables
Variables are easy just like in Python and Ruby
```rb
x = 5
println(x)
```
Unlike Python but like Ruby, assigning variables is an expression and not a statement.

This allows code like
```lua
while (x = getnext()) != null
  # do some processing with x, loop auto terminates when x becomes null and no extra assigning is needed.
end
```

## Types
Peridot has the following types currently

- Strings (`"This is a string."`)
- Number (`5` or floats `5.2`)
- Boolean (`true` or `false`)
- Arrays (`[5, 2, "hi"]`)
- HashMaps (`{ "hi": 5 }`)

Like JavaScript peridot represents both integers and floating point numbers with the same type, to explicitly check if a number is a float use `(5).is_float()` or the opposite `(5).is_int()` or if you expect integers only you can use `(5).to_i` to ensure it is an integer and if it's not it gets casted to an integer.

Like Python/Ruby/JavaScript/etc Values in Peridot are Objects so each value can have builtin methods. (Internally they aren't all heap-allocated objects, for example numbers live on the stack they just feel like objects from the user side.)

## Strings
Strings are simple just like any other languages, it works with both double (`"`) and single (`'`) quotes.

Strings are interned behind the scenes, this means only one instance of a string with same contents can be alive.
```rb
# Both of this strings is the same one in memory, no duplicate strings are made.
x = "hello"
y = "hello"
```
This reduces memory usage when bunch of strings are being created with a chance for some of them to have same contents.

It also speeds up the `==` equality operator on strings since they both point to same memory location it's compared by address and doesn't have to walk the string and compare each character.

## Dictionaries
Dictionaries maps key/value pairs if you are coming from Python/C# the name should sound familiar, if coming from other languages, Java calls them HashMaps, Ruby calls them hashes, JavaScript calls them objects, Lua calls them tables.

A basic Dictionary is like the following
```rb
hash = { "key": "value" }
println(hash["key"])
```
It's defined same as Python, and indexed same as Python and Ruby, however Peridot also has a special syntax inside dictionaries that isn't true for python, it's called "exec keys", when that key is indexed it runs a function and returns the results, it's like "getters" (JavaScript has a `get` syntax inside object literals) for example

> **NOTE:** This so called "exec keys" is a random idea that popped up on my head, it is currently not implemented and i'm not sure of it and the idea might be removed. More user feedback is helpful.

```rb
hash = { "key" => 5 }
println(hash["key"])
```
If coming from Ruby don't confuse the `=>` it's not the same.

This one does barely anything just returns a constant 5 all the time, nothing special but it can do more!
```rb
i = 0

hash = {
  "key": "value",
  "count" => i++
}

println(hash["count"]) # 0
println(hash["count"]) # 1
```
This example makes a key that increments a variable everytime it is accessed, keeping track of it's usage, we also show that it can be used simultaneously with the normal `:` syntax.

However this syntax can be used only with one expression bodies so usually you make an outer function and make it call it, this syntax is also currently experimental and might get changes to it.

The output is fairly equivalent to the following code
```lua
i = 0

function incr()
  return i++
end

hash = {
  "count": incr
}

println(hash["count"]()) # 0
println(hash["count"]()) # 1
```
But it is much cleaner.

## Scoping
Scopes in Peridot is similar to python, each function is a new scope but regular blocks still use the outer scope.
```lua
if true
  x = 5
end

println(x) # 5

function hi()
  v = 5
end

print(v) # undefined here.
```
However since we do implicit variable declarations (as in there is no need for `var`/`const`/`let` keyword or similar) it raises a few issues:
- If a variable is global and we assign it in a local scope would it modify the global value or create a new one in the scope?
- If it doesn't modify the global one is there any option to modify them somehow?

The answer to the first one is peridot does create a new variable in the scope so this code is still scoped
```lua
x = 5

function modify()
  x = 6
end

modify()
println(x) # Still 5
```
This leads us to answer the second question, we do this by `global` and `assign` keywords.

```lua
x = 5

function modify()
  global x = 6
end

modify()
println(x) # Now 6
```
The assign keyword is for the "outer scope" this is for nested blocks
```lua
function test()
  x = 5
  if true
    assign x = 6
  end
  println(x) # 6
end
```
Technically `assign` also works for globals too
```lua
x = 5

function modify()
  assign x = 6
end

println(x) # 6
```
But `global` always refers to the global scope rather than the outer one so it's useful to bail out of a large nested block and access the global scope directly.

This may seem weird but believe me, it's natural, Python also does the same, it has `global` just like we do (but in a little different syntax usage) and it has `nonlocal` (What we call `assign`, again a little different syntax usage.)

Not like you will need this on everyday code but it's there if you do.
