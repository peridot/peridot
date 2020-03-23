# Peridot Internals
This page is mostly a TODO but i put some basic information here so it won't stay empty.

## The Lexer/Parser
The lexer is written with Flex and the Parser is written with Bison.

If i get better at Bison, specifically in how to report errors in a better way and get satisfied then i will stay with these tools otherwise i might research a handwritten lexer/parser in the future.

The lexer copies strings and gives them to the parser, the parser gives to the AST and frees it, the ast makes it's own copy, this is kinda redundant and might need to be improved in the future. For now it assumes that each phase holds their own copies and we don't worry about string ownership, e.g lex -> free lex state -> parse -> free parse state etc, instead of lex -> be careful not to free something the parser needs -> parse -> be careful not to free something the ast needs. I don't know if I'm making any sense but let's continue on.

## PVM
Currently unlike JVM, PVM is only specific to Peridot and depends on it's compiler so you can't target PVM from a different frontend but that is planned to change as we think about a bytecode file format.
