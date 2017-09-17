# Web | C++

Note:
- Web and C++. Name a less iconic couple. >>

---

Serve static Webpage
```py
import SimpleHTTPServer
import SocketServer

Handler = SimpleHTTPServer.SimpleHTTPRequestHandler
httpd = SocketServer.TCPServer(("", 8000), Handler)

httpd.serve_forever()
```
<!-- .element: class="fragment" -->

Note:
- If all you want to do, is serve static content,
than those 5 lines of python, >> will most likely be enough. **5s**
- Hosting something like a blog, doesn't need to be complicated. >>

---

## Web Service

Note:
- In this talk I want to focus on web services.
- Quoting W3C: A web service is a software system designed to support
interoperable machine-to-machine interaction over a network.

---

### What makes Web Services so popular?

* Minimal first use effort
<!-- .element: class="fragment" -->
* Out of the box cross platform support
<!-- .element: class="fragment" -->

Note:
- What makes Web Services so popular? >>
- Your service is just one click away >>
- Basically everyone can use it. The tools to interact with your application,
are installed and widely used,
on the vast majority of todays consumer platforms >>

---

## Web Development

Note:
- That leads us into web development
- I want to take some time, and talk about the advantage and challenges,
of modern web development. Coming from a C++ background.

---

### Advantages

* Ecosystem
<!-- .element: class="fragment" -->
* Tools
<!-- .element: class="fragment" -->
* Frameworks
<!-- .element: class="fragment" -->

Note:
- Let's talk about the advantages. >>
- Feature rich ecosystem, with npm as its backbone. >>
- A wide variety of Tools. Coming from the assemble it yourself world,
that is C++ tooling. The feeling of just writing the word `debugger;`
in your code. Followed by a sane, intuitive debugging experience
inside your browser. Is a welcome breath of fresh air, after years of
fighting tools to achieve, seemingly simple goals.
And all that without having installed anything special. And your browser >>
- Frameworks: React, Angular, you name it. These days there a so many,
that sites like todomvc exist. >>

---

### Challenges

* Javascript (still)
<!-- .element: class="fragment" -->
* Browser support
<!-- .element: class="fragment" -->

Note:
- Challenges
- Given a system that was never intended to be used in todays manner
and even less at such scale. >>
- No matter what you do, at the end of the day, your tool chain will be spitting
out javascript. That leads to a lack of interoperability with older languages
and tools. However, that might be a completely different story, once
WebAssembly gets DOM bindings. >>
- The never ending story of browser support. >>

---

### Development Cycle

1. Edit .jsx .scss .tx ...
<!-- .element: class="fragment" -->
2. Compile to ECMAScript version X and plain css
<!-- .element: class="fragment" -->
3. Uglify
<!-- .element: class="fragment" -->
4. Bundle
<!-- .element: class="fragment" -->

Note:
This brings us to the development cycle. >>

After editing your source files. >>

You compile to javascript of a target ECMAScript version, and plain css.

The ES version usually dictates browser support.
However even older browsers can be support modern features,
with the help of polyfills. Polyfills try to emulate missing,
language or library features.
Usually they first check if there is native support.

With javascript you have the unique problem, you need to send source code
over a resource constrained channel.
So there is a notable advantage of shorter variable names and no whitespace.
Of course we don't want source code where all variables have single character
names. >>

That's the step where we use a tool to uglify our code.
The name says it all. It makes our code more ugly. A popular choice is
UglifyJS. A parser with the goal, of producing the shortest possible javascript,
whilst retaining the same logic.
On their GitHub page they have an example where the original file is 451kb
and turns into 220kb with mangling enabled. Gziped the original is 109kb,
whilst the minified version only takes up 73kb. >>

And finally, we bundle the produced files. This comes down to application
preference. Some people like single page applications.
They bundle all their logic into a single file often called `main.js`.
With this approach we can quickly interact with the application.
As navigating to a different section, does not require fetching new source code.
However this quickly leads to bloated websites with a horrendous first load
experience. The average website these days sends you more bytes source code,
than you'd need for the entire original doom binary.

Another approach is to split up your application into a set of sections.
And bundle only the code required for those.
This can help to drastically reduce average bandwidth usage.
---

### Example Service

* Persistence
<!-- .element: class="fragment" -->
* Bidirectional communication
<!-- .element: class="fragment" -->

Note:
- To give you a practical showcase of the technology we'll be talking about,
let us create a small example service.
- I want to focus on 2 major requirements. >>
- Persistence. User interaction will have lasting changes.
Otherwise we could just ship a client only solution.
Which could be much easier, and we'd save server costs. >>
- Bidirectional communication. We want to keep the user up to date,
without requiring him to pro actively request new information. >>

---

### var.bz

*Trust me, this is "definitely" not a malicious site*
<!-- .element: class="fragment" -->


Note:
Now comes the moment, where you have to make a leap of faith.
Do you trust me enough to visit this non https address? >>
Anyone brave enough?
Ok, ok, I'll do it myself.
As you can see this is just a little strawpoll service.

---

### 3 Libraries

* Beast
<!-- .element: class="fragment" -->
* uWebSockets
<!-- .element: class="fragment" -->
* IncludeOS
<!-- .element: class="fragment" -->

Note:
- Of course we don't want to reinvent the wheel ourself.
We'll use a websocket library.
- Rather than just using one library.
I want to compare 3 popular open source choices. >>
- Beast, >> uWebSockets >> and IncludeOS
- We'll compare their interfaces,
- their build requirements,
- the code needed for our example service,
- attack vectors,
- common causes of bugs
- and of course performance. >>

---

### Requirements

* request handler
<!-- .element: class="fragment" -->
* send
<!-- .element: class="fragment" -->
* broadcast
<!-- .element: class="fragment" -->

Note:
- The 3 main requirements are a: >>
- request handler.
Each user request should trigger a call to our request handler function. >>
- We want to send a message back to the client websocket. >>
- And finally, we want to broadcast a message,
to all open websocket connections. >>

---

## uWebSockets
<!-- .element: class="fragment" -->

Note:
- Let us start with a batteries included library. >>
- uWebSockets. It directly provides support for our 3 core requirements. >>

---

### Setup

```cpp
#include <uWS/uWS.h>

uWS::Hub h;
h.onMessage(request_handler);
h.listen(3003);
h.run();
```

Note:
- Construct hub
- Attach request handler
- Bind to port
- Start event loop >>

---

### Request Handler

```cpp
h.onMessage([&h, &poll_data]( //) md fix
  uWS::WebSocket<uWS::SERVER>* ws,
  char* message,
  size_t length,
  uWS::OpCode
) {

}
```

Note:
- Each time a user sends a message our request handler should be called with
- The websocket itself
- A pointer to the message buffer plus its size
- And the message opcode, usually text or binary

---

## Serialization

Note:
- Before we continue. We have to talk about serialization.
- Both the client and server need some kind of shared language.
- Of course we could use json.
That would lend itself very nicely in the frontend.
Javascript objects are basically json objects.
- However json in C++ requires a library and we lack validation,
not to mention performance. >>

---

## Flatbuffers

Note:
- That's why I decided to use Flatbuffers.
- Flatbuffers is a schema based serialization library.
- What makes Flatbuffers great,
is the virtual lack of serializing and deserializing.
- Instead of deserializing the message and constructing
an in memory representation.
- We offset jump to the place we care about. And directly read it's memory.
- Compared to json we can be 3 orders of magnitude,
faster and less memory hungry.

---

### Validation

```cpp
const auto ok = flatbuffers::Verifier(
  reinterpret_cast<const uint8_t*>(message), length
).VerifyBuffer<Strawpoll::Request>(nullptr);
```

Note:
- Flatbuffers are usually used with schema representation.
- A schema allows for more efficient message encoding and also validation.

---

### Schema

```
enum RequestType:byte { Poll, Result }

table Request {
  type:RequestType;
  vote:long;
}
```

Note:
- Here you see the entire schema for our user requests.
- Flatbuffers support pods, structs, enums, unions and tables.
- In this case all we need is a type variable representing the request type,
and optionally a vote.

---

### Send

```cpp
void sendResponse(
  uWS::WebSocket<uWS::SERVER>* ws,
  FlatBufferRef buffer
) {
  ws->send(
    reinterpret_cast<const char*>(buffer.data),
    buffer.size,
    uWS::OpCode::BINARY
  );
}
```

Note:
- Back to uWebSockets, in order to send a message,
- we need a websocket,
- the message pointer and size, here wrapped with `FlatBufferRef`
- This call is synchronous. That means this function will return once the
message was sent.

---

### Broadcast

```cpp
void broadcastResponse(
  uWS::Hub& h,
  FlatBufferRef buffer
) {
  h.getDefaultGroup<uWS::SERVER>().broadcast(
    reinterpret_cast<const char*>(buffer.data),
    buffer.size,
    uWS::OpCode::BINARY
  );
}
```

Note:
- Sending a message to every open websocket connection is very similar.
- Instead of calling `send()` on a websocket object, we need the hub.
- `getDefaultGroup()` returns a group object representing all it's open
connections.
- Conveniently, this group object provides a method to broadcast a message.
- The call to `broadcast()` and `send()` share the same signature.

---

### Request Switch

```
switch(request->type()) {
  case Strawpoll::RequestType_Poll:
    sendResponse(ws, poll_data.poll_response.ref());
    break;
  case Strawpoll::RequestType_Result:
    poll_data.register_vote(
      request->vote(),
      {},
      [&ws](FlatBufferRef br) { sendResponse(ws, br); },
      [&h](FlatBufferRef br) { broadcastResponse(h, br); }
    );
    break;
  default:
    sendResponse(ws, poll_data.invalid_type.ref());
}
```

Note:
- Finally, to tie everything together, the request switch.
- After validation we switch based on the request type.
- This works given that `request->type()` is an enum.
- To keep things simpler, we'll not talk about the `poll_data` object.
- Just note it takes care of creating and managing our Flatbuffer objects.
- In case of a poll request. We send back our constant poll object.
- In case of a result request, which should come with a vote,
we try to register the vote and give it 2 callbacks.
The first should be triggered if the vote is invalid,
the second if the vote was registered successfully.
In our case we want to broadcast the updated vote array.
- Otherwise we send a error stating an unrecognized request type.

---

## Client

* WebSocket support
<!-- .element: class="fragment" -->
* FlatBuffers support
<!-- .element: class="fragment" -->

Note:
- Of course we need a client.
- In our case a static web application.
- The 2 requirements we have for our client are: >>
- WebSocket support, technically not part of any ECMAScript standard yet.
However it has been implemented in nearly every browser by now.
- Giving us 93% of the global webbrowser users. >>
- And we need support for our choice of serialization language.
- FlatBuffer currently ships with bindings for 8 languages,
including javascript.

---

### Setup

```js
setupWebSocketCommunication() {
  this.socket = new WebSocket(this.props.apiUrl);
  this.socket.binaryType = 'arraybuffer';

  this.socket.addEventListener('open', this.fetchPoll);
  this.socket.addEventListener('message',
    this.handleServerResponse
  );
  this.socket.addEventListener('close', this.handleDisconnect);
}
```

Note:
- First we create a new WebSocket.
- Set it's binary type to arraybuffer and then add our EventListeners.
- A WebSocket connections usually has 3 event types.
- `open` the event triggered once the http request upgrade,
and handshake were successful.
- `message` the event triggered each time the server sends a message
- `close` the event triggered by the either the server sending a close frame,
or a failed ping pong request. >>

---

### Response Switch

```js
switch(response.type()) {
  case Strawpoll.ResponseType.Poll:
    this.updatePoll(response.poll());
    break;
  case Strawpoll.ResponseType.Result:
    this.updateResult(response.result());
    break;
  case Strawpoll.ResponseType.Error:
    console.error("Error: ", response.error());
    break;
  default:
    console.error("Invalid response type: ", response.type());
};
```

Note:
- Again we switch based on an enum. This time the response type.
- There are 3 known response types. Poll, Result and Error.
- Unless there is an error we just trigger the relevant object update. >>

---

## IncludeOS

Note:
- Let us take a look at our second high level WebSocket library.
- However IncludeOS is much more than a WebSocket library.
- Who of you went to Alfred's talk earlier this week?
- For those unfamiliar with IncludOS, it's a young unikernel project.
Which allows you to create applications that are compiled into bootable,
special purpose operating systems. All the relevant pieces of the OS are
statically linked into your application.
- The result can be a bootable image with network stack, with a size,
in the area of a megabyte. That can boot in milliseconds.
- Onto why someone would want this, later. >>

---

### Setup

```cpp
#include <net/inet4>
#include <service>
#include <net/ws/websocket.hpp>
#include <net/http/server.hpp>

auto& inet = net::Inet4::stack<0>();
auto server = std::make_unique<http::Server>(inet.tcp());
server->on_request(request_handler);
server->listen(3003);
```

Note:
- First we include the relevant os components.
- Retrieve a reference to the os internal network stack.
- Attach our request_handler and bind to our port. >>

---

### Request Handler

```cpp
server->on_request([] (auto req, auto rw)
{
  if(req->method() != http::GET) {
    rw->write_header(http::Not_Found);
    return;
  }
  if(req->uri() == "/ws") {
    auto ws = net::WebSocket::upgrade(*req, *rw);
    handle_ws(std::move(ws));
  }
  else {
    rw->write_header(http::Not_Found);
  }
});
```

Note:
- This time we have to trigger the HTTP Upgrade ourself.
- We try to upgrade,
if the request is both GET and comes from a WebSocket address >>

---

### Message Loop

```cpp
ws->on_read = [ws = ws.get()](auto msg)
{
  ws->write(
    msg->data(),
    msg->size(),
    msg->opcode()
  );
};
```

Note:

- Our request handler will only be triggered each time a connection is
established. So we still need an event handler for the client messages.
- This is what a simple echo server could do. >>

---

### Send

```cpp
void sendResponse(
  WebSocket_ptr ws,
  FlatBufferRef buffer
) {
  ws->write(
    reinterpret_cast<const char*>(buffer.data),
    buffer.size,
    net::op_code::BINARY
  );
}
```

Note:
- For those of you thinking they are having a dejavu,
yes it look very similar to the uWebSockets interface.
- Again we write a pointer, size and opcode.
- IncludeOS does not have native broadcast support.
We'll learn how to implement it once we look at the Beast example. >>

---

## Beast

---

### Setup

```cpp
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

boost::asio::io_service ios{1};
listener lis{ios, tcp::endpoint{address, port}};
ios.run();
```

Note:
- Setting up a Beast server seems harmless enough.
- Let's look at listener, implemented by us. >>

---

## Event Loops

Note:
- To recap the event loop model,
let's first take a quick look at the synchronous Beast example. >>

---

### Connection loop

```cpp
tcp::acceptor acceptor{ios, {address, port}};

for(;;)
{
    tcp::socket socket{ios};
    acceptor.accept(socket);

    std::thread{[so = std::move(socket)]()
      { do_session(std::move(so)); }
    }.detach();
}
```

Note:
- Each time we create a new tcp socket.
- `accept` should block until we get a connection.
- Once we have a new session, we spawn a worker thread and
transfer ownership of the socket. >>

---

### Message Loop

```cpp
void do_session(tcp::socket&& socket)
{
  websocket::stream<tcp::socket> ws{std::move(socket)};
  ws.accept();

  for(;;)
  {
    boost::beast::multi_buffer buffer;
    ws.read(buffer);

    ws.text(ws.got_text());
    ws.write(buffer.data());
  }
}
```

Note:
- First we construct the WebSocket stream by moving in the socket.
- Then we block until the handshake was resolved.
- Again we go into an endless loop.
- The read function should block until we receive a client message.
- Here we just echo back the message. >>

---

# Async IO

Note:
- Of course we want to take advantage of asios async interface.
- For those of you unfamiliar with the concept of async IO.
- The core idea is, rather than blocking until an IO event is done,
to start the operation, and to query the result state.
- Usually this means that synchronization bubbles up to some kind of event loop.

---

### Non Sequential Loop

Note:
- We need some kind of loop that resumes at arbitrary points.
- Neither `for` or `while` are going to cut it.
- What can a loop be reduced down to? >>

---

`goto`

Note:
- Yep, `goto`.
- Only problem, as children we've all been told the story of the big bad `goto`.
- Maybe there is a better way of doing this. >>

---

### Cyclic Call Graph

```cpp
void foo();

void bar() { foo(); }
void foo() { bar(); }
```

Note:
- This has the advantage, that we can now suspend one part of our function.
- With that, let's take a look at the actual Beast example. >>

---

### Accepting Connection

```cpp
explicit listener()
{
  if (!acceptor_.is_open()) return;
  do_accept();
}

void do_accept()
{
  acceptor_.async_accept(
    socket_,
    strand_.wrap([this](boost::system::error_code ec)
      { on_accept(ec); }
    )
  );
}
```

Note:
- Of course there is a lot of code missing.
- Constructing `listener` starts the event loop.
- `async_accept` takes a socket reference, to move into and
a continuation handler. >>

---

### New Session

```cpp
void on_accept(boost::system::error_code ec)
{
  if (ec) fail(ec, "accept");
  else
  {
    sessions_.try_emplace(
      session_id_counter_++,
      std::move(socket_)
    );
  }

  do_accept();
}
```

Note:
- This creates a new session.
- Note that `on_accept` always calls `do_accept`,
which in turn calls `on_accept`. >>

---

### Message Loop | Start

```cpp
explicit session(tcp::socket&& socket)
    : ws_{std::move(socket)}
{
  ws_.async_accept(
    strand_.wrap([this](boost::system::error_code ec)
      { on_accept(ec); }
    )
  );
}
```

Note:
- This should look familiar.
- It's basically the same as the `listener` constructor.
- It kick starts the session event loop. >>

---

### Message Loop | Read

```cpp
void do_read()
{
  buffer_.consume(buffer_.size());

  ws_.async_read(
    buffer_,
    strand_.wrap(
      [this](
        boost::system::error_code ec,
        size_t bytes_transferred
      )
      { on_read(ec, bytes_transferred); }
    )
  );
}
```

Note:
- After clearing our target buffer we subscribe a read operation
- Once the read is completed our handler should be called. >>

---

### Message Loop | Write

```cpp
void do_write()
{
  ws_.async_write(
    std::array<boost::asio::const_buffer, 1>{{
      std::move(message_queue_.back())
    }},
    strand_.wrap(
      [this](
        boost::system::error_code ec,
        size_t bytes_transferred
      )
      { on_write(ec, bytes_transferred); }
    )
  );
}
```

Note:
- Writing looks very similar.
- One thing to note, Beast expects to write a range of buffers.
- However they will not be represented as individual messages.
- The purpose is to avoid copying buffers if possible. >>

---

### Session Event Chain

* `on_accept` ->
<!-- .element: class="fragment" -->
* `do_read` ->
<!-- .element: class="fragment" -->
* `on_read` ->
<!-- .element: class="fragment" -->
* `do_write` ->
<!-- .element: class="fragment" -->
* `on_write` ->
<!-- .element: class="fragment" -->
* `do_read` ->
<!-- .element: class="fragment" -->

Note:
- Which brings us to the session event chain. >>
- `on_accpet` calls >> `do_read`, which in turn calls >> `on_read`,
which calls `do_write`, which calls >> `on_write` which then calls >> `do_read`
again.
- And with that the loop is closed. >>

---

### Broadcast

```cpp
using session_t = session<on_session_close_t, broadcast_t>;
using sessions_t = std::unordered_map<size_t, session_t>;

void broadcast(FlatBufferRef br)
{
  for (auto& [key, session] : sessions_)
  {
    session.add_message(br);
    session.flush_message_queue();
  }
}
```

Note:
- Earlier I mentioned manual broadcast.
- We store all our sessions in an unordered map.
- That way we can quickly add and remove sessions.
- A broadcast, queues a message for all open sessions.
- Explaining the queuing mechanism and pitfalls would break the scope of
this presentation.

---

Note who Beast and Asio use the much faster system error codes instead of exceptions.
---

### Asynchronous Primitives

---

### Socket library

---

### Ip Stack

- We'll compare their interfaces, DONE
- the code needed for our example service, DONE
- attack vectors,
- common causes of bugs
- and of course performance. >>

---

### Example Code Size

* uWebSockets | ~1.6kb
<!-- .element: class="fragment" -->
* IncludeOS | ~3kb
<!-- .element: class="fragment" -->
* Beast | ~9kb
<!-- .element: class="fragment" -->

Note:
- To give you a rough idea about the effort required to use each library.
ran each
-

---

### Example Compile Time

* uWebSockets | Debug: 1.4s | Release: 1.8s
<!-- .element: class="fragment" -->
* IncludeOS | Default: 2.9s
<!-- .element: class="fragment" -->
* Beast | Debug: 8.5s | Release: 17.1s
<!-- .element: class="fragment" -->

Note:
- Note, the results are the 90 percentile compiled with clang 4. >>
- uWebSockets is linked as dynamic library, making a breeze to work with. >>
- I couldn't get IncludeOS to build as Debug,
so I measured the default settings. With 90 percent being under 3 seconds,
still nice to quickly iterate with. >>
- Beast, being part of Boost,
carries the burden of the template heavy header only approach. >>

---

## Documentation

Note:
- In terms of documentation, uWebSockets barley has any. It's a little better
for IncludOS. Still you'll find yourself reading mostly source code.
- Beast on the other hand has excellent documentation. >>

---

## Threading

Note:
- To keep the example smaller and simpler, all of the exmples were purely
single threaded. That gives me little experience in this context.
However Beast has clearly been designed with multi threading in mind.
- Still very complicated and easy to get wrong.
- Not sure about uWebSockets.
- IncludeOS currently, does not support any kind of threading. >>

---

## SSL

Note:
- Both uWebSockets and Beast have native support for SSL via OpenSSL.
- However it should be easy to put an IncludeOS application behind a
reverse proxy like NGINX. >>

---

## Security

Note:
- Looking at possible attack vectors, both Beast and uWebSockets are
regular binaries, allowing for wide variety of exploits,
paired with a mature set of tools for reverse engineering.
- IncludeOS' unique execution environment, makes that harder.
- What IncludeOS can't protect you from is application flaws.
Let's say there is a bug in our vote registration code.
That way someone could perform vote manipulation. A unikernel is not going
to protect you from that.
- What it can do is limit damage of things like remote code execution.
- Many exploits rely on finding libc and using it to spawn a shell.
- It can mitigate a set of known exploit propagation,
like the aforementioned system call.
- Remember, the biggest attack vector, is and remains social engineering. >>

---

## Performance

Note:
- This is my first time benchmarking a network application,
take my results with a lot of salt.
- Again the results are the 90 percentile,
from running uWebSockets scalability test locally.
- Sadly I didn't manage to benchmark IncludeOS. >>

---

## 100 concurrent connections

Note:
- I ran the benchmark in 2 settings.
- First with 100 concurrent connections. >>

---

### Memory Usage | User Space

* uWebSockets: 4.6mb
* Beast: 4.2mb

Note:
- No big difference here. >>

---

### Connections per ms

* uWebSockets: 7.1
* Beast: 5.3

Note:
- Onto the connections per millisecond.
- At this small sample size, this shouldn't be all that representative. >>

---

## 10.000 concurrent connections

Note:
- Now with 10.000 concurrent connections. >>

---

### Memory Usage | User Space

* uWebSockets: 6.4mb
* Beast: 49.3mb

Note:
- With it's extremely special purpose implementation, uWebSockets manages to
have an impressively small memory footprint.
- At that rate, you can host Google esque traffic,
with less RAM than your phone has.
- Beast does well enough. >>

---

### Connections per ms

* uWebSockets: 35.0
* Beast: 26.4

Note:
- At this timescale, your application is much more likely to become the
bottleneck.
- This laptop managed to request and serve 10.000 connections,
in a third of a second.
- With that kind of traffic,
you should be able to afford better hardware for sure. >>

---

## Best Library?

Note:
- So, who is the winner? Which library is the best? >>

---

# Wrong Question!

Note:
- Wrong question!
- The 3 libraries I presented, all aim to achieve vastly different goals.
- The better question is, what do you want? >>

---

### Key Strength

* Easy | uWebSockets
<!-- .element: class="fragment" -->
* Secure | IncludeOS
<!-- .element: class="fragment" -->
* Modular | Beast
<!-- .element: class="fragment" -->

Note:
- To summarize, I'll associate each library with a key strength. >>
- If you want an easy, fast and lightweight solution,
take a look at uWebSockets. >>
- Albeit being a young project, IncludeOS shows promising progress towards
being a solid choice for security oriented services. >>
- With its aim, to be as modular and adaptable as possible,
Beast comes with inherent complexity and trade offs.
- Given its aspiration to become part of the standard library,
it could very well be core of many upcoming libraries.

---

## Takeaway

---

### Source

https://github.com/Voultapher/Presentations

---

## Vote Result

---

# Q&A
