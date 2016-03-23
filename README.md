# node-svmq
Native System V IPC message queues in Node.js with bindings and an easy to use abstraction. [System V message queues](http://linux.die.net/man/7/svipc) are more widely implemented on UNIX systems than [POSIX message queues](http://linux.die.net/man/7/mq_overview), and are supported on both Linux and OS X.

These are FIFO queues. New messages are pushed to the end and old messages are popped off first.

## Why?
The keyword here is _IPC_. These queues exist in kernel space; they can be accessed by multiple processes,
assuming the correct permissions, etc. This provides a portable method for passing messages between
two completely unrelated processes using potentially different languages/runtimes.

For example, PHP has native support for SysV IPC including message queues. That means you can now easily
serialize PHP objects to JSON and pass them to Node.js (and vice versa) without messing with pipes,
sockets, etc. and without having to spawn processes from within Node.js.

## Installation
`npm install svmq`

Native bindings are written in C/C++ using [NAN](https://github.com/nodejs/nan) and built automatically on install.

## Usage

### Creating or opening a message queue
```javascript
// Opens or creates the queue specified by key 31337
var queue = require('svmq').open(31337);
// OR
var MessageQueue = require('svmq');
var queue = new MessageQueue(31337);
```

### Listen for new messages
```javascript
// Listen for new messages on the queue
// If the queue already contains messages, they will be popped off first (one at a time).
queue.on('data', (data) => {
  // Data is provided as a Buffer. If you're passing Strings back and forth, be sure to use toString()
  // However, data does not have to be a String. It can be any type of data in Buffer form.
  console.log('Message: ' + data.toString());
});
```

### Push message onto queue
```javascript
// Push a new message to the queue
queue.push(new Buffer('TestString1234'), (err) => {
  // This callback is optional; it is called once the message is placed in the queue.
  console.log('Message pushed');
});

// Note that SysV message queues may block under certain circumstances, so you cannot assume that
// the above message will already be in the queue at this point.
// Use the callback to know exactly when the message has been pushed to the queue.
```

### Pop message off queue
```javascript
// Pop a message off of the queue
// Do not use pop() with the 'data' event; use one or the other.
queue.pop((err, data) => {
  if (err) throw err;
  console.log('Popped message: ' + data.toString());
});
```

### Close or dispose of a queue
```javascript
// Close the queue immediately
// Returns true/false, specifying whether or not the queue closed.
// Can be used with a callback to catch errors on close.
//
// Note: this may require escalated privileges on some OSes.
var closed = queue.close();
// OR (closed status will be returned and passed to callback)
var closed = queue.close((err, closed) => {
  if (err) throw err;
});

```

### Access to native bindings
Using these is not recommended, but you're more than welcome to mess around with them.
```javascript
// Simplified JS bindings to the C syscalls.
// Blocking calls use a callback as the last parameter.
//
// msgget, msgsnd, msgrcv, msgctl
// See: http://linux.die.net/man/7/svipc
var svmq = require('svmq');
var msg = svmq.msg;
var MSGMAX = svmq.MSGMAX; // max message data size (hardcoded)

// Open/create a queue with key 31337 and flags 950 (0666 | IPC_CREAT)
// Throws an error on failure
var id = msg.get(31337, 950);

// Push a string to the queue
msg.snd(id, new Buffer('TestString1234'), 1, (err) => {
  if (err) throw err;
});

// Pop message off queue with max buffer size MSGMAX
var bufferSize = MSGMAX;
msg.rcv(id, MSGMAX, 0, 0, (err, data) => {
  if (err) throw err;
  console.log('Received data: ' + data.toString());
});

// Close/delete a queue
// Throws an error on failure
msg.ctl(id, IPC_RMID);
