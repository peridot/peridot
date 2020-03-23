# Lambdas in Peridot
> **NOTE:** This is still undecided, it's a page for the future.

Lambdas in peridot exist partially, it is an alias to callbacks but useful when you need a single expression body and don't need to open a new do block.

```pd
function hi()
  yield 5
end

# Normal way. ugh i don't want to open a new block.
hi() do |x|
  println(x)
end

# Lambdas
hi((x) -> println(x))

## Cleanest with array operations

# ugly
get_array().filter() do |x|
  return
end
# you need to store to result of filter to do further operations, with lambdas let's chain em.

get_array()
  .filter((x) -> x.something())
  .filter((x) -> x.verified())
  .map((x) -> x.get_name())
  .map((x) -> println(x))

# so beautiful!
```
