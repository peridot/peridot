
def fib(n)
  if n < 2
    return n
  end
  return fib(n - 2) + fib(n - 1)
end

puts(fib(35))
