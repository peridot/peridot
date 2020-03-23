# Why Peridot
This doc explains some answers about why i created peridot and how it compares to other languages and some information about design decisions i made.

**Note:** There is a lot of personal opinions here that may not be true for everyone so please don't get triggered.

## Memory Usage
I needed a language to use very little memory as possible, here's my experience with this topic about other languages i used:

### JavaScript (Node.js)
For a long time JavaScript with Node.js was my favourite option, but memory usage in Node.js wasn't that good, it uses about 40mb already for simple scripts, while there are a ton of JavaScript engines and runtimes that use less memory they wouldn't be as good in performance compared to Node.js (which uses V8, which is the fastest JS engine at this time.), You pick memory or performance but not both, and JavaScript is a very complex language with a huge spec so it really needs the performance than memory, i tried to balance peridot between both of the worlds, peridot isn't that complex as JavaScript, you can easily learn it in a single day therefore it leads to a simpler design and balances both but i'm not claiming that peridot is faster than V8 or something.

### Python
Python was my first programming language to start, i used it for a long time, the memory usage is decent it would use about 17mb for simple scripts, better than Node.js but could've been better. i hear a lot of complains about Python being slow so i try to make peridot faster than Python atleast, never the less i don't intend hate to Python, i still use it as a little scripting language to automate some jobs here and there.

### Java
Java was also one of the earliest languages i learned, performance is decent and all but not memory usage, infact Java is the heaviest in this list, it uses as much memory as 100mb depending on the program, but its a lot, you would need a memory rich server to deploy servers written in Java, never the less Java is still good at some points, for example on Android it has it's own VM (DalvikVM and on later versions ART) which performs better than JVM in terms of memory since memory is more critical on handheld devices such as Smartphones.

### Ruby
Yet another one of early languages i learned was Ruby, Ruby is great but in terms of memory usage i haven't exactly measured that yet but it does feel light, there isn't much to say about Ruby about this topic.

### Golang
Go was one of the last languages i learned, in terms of memory usage it is very good, simple scripts use as much as a few KBs, but Go compiles to machine code directly rather than using Virtual Machines as the other languages in this list so there is less overhead on runtime, no parser/lexer/compiler available at runtime and code can start executing immediately, in Peridot i wanted to balance between memory usage of Go and dynamic VM behavior of other languages.

### Lua
Lua was one of the late languages i tried, it is very easy, extremely lightweight and fast, memory usage in lua is good since it is minimal, Peridot isn't as minimal as lua but i tried to keep it simple and light.

Heck lua is so fast that now we even have LuaJIT that JIT compiles Lua to machine code now no one can beat the performance of LuaJIT unless you go even more barebones than Lua is but then what's fun programming in a language that doesn't even have functions.

## IDE
Some languages like Java/C# really really needs an IDE, it is possible to use them without IDE but it is harder, you waste more time setting up an environment when you could simply use an IDE, however i don't like IDEs to hide a lot of magic behind me, Peridot is designed in so it doesn't need an IDE at all, the most basic environment you could ask for is the CLI and yet it's easy there without lot of magic behind, never the less IDEs can be good with peridot too just that it isn't needed as much as Java/C# needs it.

## Compatibility
Peridot hopes to be compatible on all platforms but mainly on all environments, because for example it has a module system builtin where as JavaScript doesn't always have a module system and introduces various incompatibilities with module systems available, CommonJS, AMD, ES Modules and many more you would have to use a bundler such as webpack and such to be more compatible which introduces more difficulty when developing libraries that work on multiple environments.

## Syntax
When developing a language you get to choose the syntax and that's one of the fun parts!

I did make sure the syntax i picked makes sense to everyone, it is mainly inspired by Ruby and Lua, while i tried to remove the confusing parts of Ruby, for example consider this webserver example in sinatra
```rb
require "sinatra"

get "/" do
  "Hello, World!"
end
```
This type of code becomes confusing, because one could ask what is `get` where is it defined? it could also easily clash with an existing method, not to mention the various helpers accessible inside `get` which uses weird scoping to allow a subset of methods to exist in that scope, this code can become confusing and clash with existing names easily especially when refactoring a large program, in peridot a webserver could look like
```pd
import ...your web framework...

app = framework.App()

app.get("/") do |req, res|
  res.send("Hello, World!")
end

app.listen(3000)
```
This is more cleaner, you can know the get method is on an app instance and the request and response are passed to the handler so we know only those variables exist on our scope and we can name them anything.

Another is the method calls without `()` ruby allows that, and by mentioning the method name alone it calls it with no arguments, if you intent to hold the function itself it calls it, not what you want! While it is possible with lambdas and procs it only complicates it even more.

Say
```rb
def hi
  return 5
end

puts hi # 5
puts hi() # 5
```
While in peridot
```em
function hi()
  return 5
end

println(hi) # <function "hi" at 0xSomeAddress>
println(hi()) # 5
```
Function itself is also an object and has methods to easily get name, source and disassembly of a function.

This means peridot always requires `()` for function calls, in my opinion this is more readable and cleaner.

From lua it borrows `function` keyword and lua has the behavior i intend with `()` as described above.

Ruby also has the implicit return where the last expression is the function's return value, in my opinion this is not a good idea, you have to pay more attention to what your last expression is so you don't return something you don't intend to!

## Standard Library
I wanted peridot to have a big standard library covering most common use cases without bloating the project with external dependencies, say, have you used Node.js? ever ran npm install and got hundreds of dependencies? i want to solve that and have lightweight implementations builtin and maintained alongside the language. Never the less we support packaging and installing modules i never said peridot will have "everything" builtin, there are still a lot of modules one can publish and by the design of peridot hopefully module writers will make small packages that won't be as bloated as NPM.

<!-- 
TODO: module system is not decided quite yet, this was my initial thoughts but it might need some changes.

Modules are also installed on a global directory, some people won't like it but hear me out:

- Modules are then installed once and ran everywhere no need to reinstall same module over and over for every project.
- Modules are still suffixed with version numbers so there shouldn't be version issues.

Think the way Ruby and Python does it with gems and pip, that's the behaviour i like.

When importing a module on a personal project it will load the latest installed (because new projects should always begin with latest versions!) while a module project can specify the versions they need to function.
-->

## Why Flex/Bison
So you saw the source and want to argue about why i used flex and bison for the lexer and parser, don't you?

First flex and bison are great tools existing since long time so it is stable and all, i started using it and i liked it, so since i have previous experience with it why not use something you know?

As Bob Nystrom once said:
```
I’m going to make a claim here that will be unpopular with some compiler and language people. It’s OK if you don’t agree. Personally, I learn more from strongly-stated opinions that I disagree with than I do from several pages of qualifiers and equivocation. My claim is that parsing doesn’t matter.

Over the years, many programming language people, especially in academia, have gotten really into parsers and taken them very seriously. Initially, it was the compiler folks who got into compiler-compilers, LALR and other stuff like that. The first half of the Dragon book is a long love letter to the wonders of parser generators.

All of us suffer from the vice of “when all you have is a hammer, everything looks like a nail”, but perhaps none so visibly as compiler people. You wouldn’t believe the breadth of software problems that miraculously seem to require a new little language in their solution as soon as you ask a compiler hacker for help.

Yacc and other compiler-compilers are the most delightfully recursive example. “Wow, writing compilers is a chore. I know, let’s write a compiler to write our compiler for us.”

For the record, I don’t claim immunity to this affliction.

Later, the functional programming folks got into parser combinators, packrat parsers and other sorts of things. Because, obviously, if you give a functional programmer a problem, the first thing they’ll do is whip out a pocketful of higher-order functions.

Over in math and algorithm analysis land, there is a long legacy of research into proving time and memory usage for various parsing techniques, transforming parsing problems into other problems and back, and assigning complexity classes to different grammars.

At one level, this stuff is important. If you’re implementing a language, you want some assurance that your parser won’t go exponential and take 7,000 years to parse a weird edge case in the grammar. Parser theory gives you that bound. As an intellectual exercise, learning about parsing techniques is also fun and rewarding.

But if your goal is just to implement a language and get it in front of users, almost all of that stuff doesn’t matter. It’s really easy to get worked up by the enthusiasm of the people who are into it and think that your front end needs some whiz-bang generated combinator parser factory thing. I’ve seen people burn tons of time writing and rewriting their parser using whatever today’s hot library or technique is.

That’s time that doesn’t add any value to your user’s life. If you’re just trying to get your parser done, pick one of the bog-standard techniques, use it, and move on. Recursive descent, Pratt parsing, and one of the popular parser generators like ANTLR or Bison are all fine.

Take the extra time you saved not rewriting your parsing code and spend it improving the compile error messages your compiler shows users. Good error handling and reporting is more valuable to users than almost anything else you can put time into in the front end.
```
From [http://craftinginterpreters.com/compiling-expressions.html#design-note](http://craftinginterpreters.com/compiling-expressions.html#design-note)

Handmade lexers and parsers sure sounds better than a generated one but i have zero experience with writing parsers and it will only delay development further because i would need to focus more on the parser than acually running the code, parsing is only ran on startup, the most imporant and critical part is the runtime, would you want a language that takes 1 nanosecond to parse but a few minutes to run it? Sure it's nice for parsing to be fast too but don't overdo it (though Bison is not even that slow.), i may or may not consider a handwritten lexer/parser in the future but that's when the language is stable enough that i have time to play with the parser.

