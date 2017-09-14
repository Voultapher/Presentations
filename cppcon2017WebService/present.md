# Web | C++

Note:
- Web and C++. How do they mix? >>

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
whilst retaining the same logic. >>

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

### 45.55.173.32

*Trust me, this is "definitely" not a malicious site*
<!-- .element: class="fragment" -->


Note:
Now comes the moment, where you have to make a leap of faith.
Do you trust me enough to visit this non https link? >>
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

### request handler

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

### validation

```cpp
const auto ok = flatbuffers::Verifier(
  reinterpret_cast<const uint8_t*>(message), length
).VerifyBuffer<Strawpoll::Request>(nullptr);
```

Note:
- Flatbuffers are usually used with schema representation.
- A schema allows for more efficient message encoding and also validation.

---

### schema

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

### send

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
- Back to uWebSocket, in order to send a message,
- we need a websocket,
- the message pointer and size, here wrapped with `FlatBufferRef`
- This call is synchronous. That means this function will return once the
message was sent.

---

### broadcast

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

### request switch

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
- Finally to tie everything together, the request switch.
- After validation we switch based on the request type.
- This works given that `request->type()` is an enum.
- To keep things simpler, we'll not talk about the `poll_data` object.
- Just note it takes care of creating and managing our Flatbuffer objects.
- In case of poll request send back a our constant poll object.
- In case of a result request which should come with a vote,
we try to register the vote and give it 2 callbacks.
The first should be triggered if the vote is invalid,
the second if the vote was registered successfully.
In our case we want to broadcast the updated vote array.
- Otherwise we send a error stating an unrecognized request type.

---

### Asynchronous primitives

---

### Socket library

---

### Ip stack

---

# Include OS

---

## Documentation

---

## Threading

Beast clear winner. Very complicated, easy to get wrong.
uWebSockets not sure.
IncludeOS currently, does not support any kind of threading.

---

## SSL

Both uWebSockets and Beast have native support for SSL
However it should be easy to put an IncludeOS application behind a
reverse proxy like NGINX.

---

## Performance

Compilation time per project, 95 percentile

---

# Q&A
