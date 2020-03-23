
function fib(n::Int)
  if n < 2
    return n
  end
  return fib(n - 2) + fib(n - 1)
end

println(fib(35))
