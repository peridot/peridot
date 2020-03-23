# Peridot Modules
Modules in Peridot is designed to be simple and allow reusable code.

## Import
The `import` keyword is used to import a file it loads files from the module path or a relative file.
```
import random
```
You can also alias the imported name
```
import random as rng
```
Unlike Python we don't allow importing multiple packages in one line `import random, io` this is to reduce syntax arguments and keep everyone happy.
