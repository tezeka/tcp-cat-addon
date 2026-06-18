## tcp-cat-addon

A lightweight TCP client addon built in C for the Bare runtime, using libuv for asynchronous networking.

This project demonstrates how to build native extensions for Bare that interact with JavaScript via the C API, performing low-level TCP communication.

---

## 🚀 What it does
```js
`tcpCat(host, port, request)`:
```

- Opens a TCP connection to a remote host
- Sends a raw request (e.g. HTTP request)
- Reads the response asynchronously
- Returns a Promise that resolves with a `Buffer` (ArrayBuffer in Bare)

---

## 📦 Example usage
```js
const { tcpCat } = require('.')

const response = await tcpCat(
  'example.com',
  80,
  `GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n`
)

console.log(Buffer.from(response).toString())
```
---

## 🧠 How it works

The addon is implemented in C using:

Bare C API (bare.h, bare/module.h)
libuv for async TCP networking
Promises (deferreds) for async JS interop
Flow:
JS calls tcpCat(host, port, request)
C creates a TCP socket via libuv
Connects to the server
Sends raw request
Reads response in chunks
Aggregates into a buffer
Resolves JS Promise with an ArrayBuffer

---

## 📁 Project structure
```md
├── binding.c        # Native addon implementation (C)
├── index.js         # JS wrapper for the addon
├── test.js          # Integration tests (brittle)
├── CMakeLists.txt   # Build system
└── prebuilds/       # Compiled binary output
```
---

## 🧪 Running tests
```bash
npm install
npm test
```
Uses brittle-bare test runner.

---

## 🔧 Build manually
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
Then copy the binary:
```bash
mkdir -p prebuilds/linux-x64
cp build/*.bare prebuilds/linux-x64/bare-addon.bare
```
---

## ⚠️ Notes

This is a low-level experimental addon
Handles raw TCP (no HTTP parsing)
Uses manual memory management (malloc/realloc/free)
Designed for learning Bare + libuv internals

---

📚 Concepts demonstrated

Native addons (C / Bare runtime)
libuv TCP networking
Async callback flow in C
JavaScript Promise bridging
Buffer / ArrayBuffer interop
