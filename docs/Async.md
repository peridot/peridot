# Asynchronous Peridot
Peridot is aimed for asynchronous usage, it uses `libuv` for the event loop, if you are coming from Node.js that's exactly what Node.js also uses therefore you will feel comfortable using peridot.

**Note:** This feature isn't implemented yet but rather acting like a reference on how it will look like in the future.

## Async/Await
Callback hell was hell in JavaScript, promises was cleaner but still it took callbacks, even better: async/await came out, peridot decided to immediately pick async/await as it's initial decision for asynchronous programming.

If you are coming from JavaScript you will feel home, if coming from Python it's also mostly same but functions still execute without await and the event loop is managed by the language runtime.
```pd
async function hello()
  return 5
end

hello() // AsyncTask

async function main()
  value = await hello()
  println(value) # 5
end

main()
```

Like JavaScript, we have an implementation of `setTimeout`/`setInterval` in a builtin module
```pd
import timers

timers.schedule(2000) do
  println("Hello, World!") # runs after 2000 milliseconds (2 seconds)
end

timers.schedule_interval(1000) do
  println("Hello, World!") # runs every second.
end
```
`schedule_interval` returns an `IntervalTask` that has methods to cancel it, `schedule` returns a `ScheduledTask` that can also be used to wait for the task to execute. (like promisifying setTimeout in JavaScript)
```pd
import timers

async function main()
  task = timers.schedule(3000) do
    println("Hello, World!")
  end
  await task.wait()
  println("Text printed.")
end

main()
```

Asynchronous web-requests example
```pd
import http

async function main()
  res = await http.get("https://google.com")
  println(res.text())
end

main()
```

## How would callbacks look like in Peridot?
Let's rewrite the async web request to use callbacks, this won't work but shows how it will look like.
```pd
import http

http.get("https://google.com") do |res|
  println(res.text())
end
```
Not too bad but remember callbacks gets worse everytime you need to do more asynchronous jobs inside them
```pd
http.get("url 1") do |res1|
  http.get("url 2") do |res2|
    http.get("url 3") do |res3|
      println(res3.text())
    end
  end
end
```
With async/await that looks like
```pd
async function main()
  res1 = await http.get("url 1")
  res2 = await http.get("url 2")
  res3 = await http.get("url 3")
  println(res3.text())
end

main()
```
So much smoother don't ya think?

## Event-driven
Like Node.js we like event driven code
```pd
import events

async function get_value()
  return 5
end

ee = events.Emitter()

ee.on("event") async do |x|
  println(await get_value())
end

ee.emit("event")
```
