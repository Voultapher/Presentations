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
- Hosting something like a blog, doesn't need to be complicated.
And unless you have very specific requirements, this solution should be
fine in terms of performance. >>

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
- The never ending story of browser support >>
