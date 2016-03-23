'use strict';

let EventEmitter = require('events').EventEmitter;
let inherits = require('util').inherits;
let msg = require('../build/Release/msg');

let MessageQueue = function(key, perms, options) {
  if (typeof key !== 'number') {
    throw new Error('Key must be a number. If you\'re providing a path, use ftok first.')
  }

  if (typeof perms === 'undefined') {
    perms = 0x666;
  } else if (typeof perms !== 'number') {
    throw new Error('Permissions must be in number form, i.e. 0x666');
  }

  this.key = key;
  this.perms = perms;
  this.flags = formatFlags(perms);
  this.options = options || { };

  this.closed = false;
  this.receiveBuffer = [];

  this.on('newListener', (event) => {
    if (event === 'data') {
      setImmediate(() => this._beginReceive());
    }
  });

  this._open();
};

inherits(MessageQueue, EventEmitter);

MessageQueue.open = (key, perms) => new MessageQueue(key, perms);

MessageQueue.prototype._open = function() {
  if (this.id >= 0) throw new Error('Queue already open');

  this.id = msg.get(this.key, this.flags);
};

MessageQueue.prototype.push = function(data, options, callback) {
  if (!Buffer.isBuffer(data)) {
    data = new Buffer(data);
  }

  if (typeof options == 'function') {
    callback = options;
    options = { };
  }

  if (typeof callback === 'undefined') {
    callback = Function.prototype;
  }

  options = options || { };
  if (typeof options.type === 'undefined') {
    options.type = 1;
  } else if (typeof options.type !== 'number' || options.type <= 0) {
    return callback(new TypeError('Message type must be a positive nonzero integer or undefined'));
  }

  if (typeof options.flags === 'undefined') {
    options.flags = 0;
  } else if (typeof options.flags !== 'number') {
    return callback(new Error('Message flags must be a number or undefined'));
  }

  msg.snd(this.id, data, options.type, options.flags, callback);
};

MessageQueue.prototype._beginReceive = function() {
  if (this.listenerCount('data') === 0) return;

  // TODO
  let options = { };

  this._receive(options, (err, data) => {
    if (err) {
      this.emit('error', err);
      return;
    }

    if (this.listenerCount('data') > 0) {
      this.emit('data', data);
    } else {
      this.receiveBuffer.push(data);
    }

    this._beginReceive();
  });
};

MessageQueue.prototype._receive = function(options, callback) {
  options = options || { };

  if (typeof options.type === 'undefined') {
    options.type = 0; // pop
  } else if (typeof options.type !== 'number') {
    return callback(new TypeError('Message type must be a positive nonzero integer or undefined'));
  }

  if (typeof options.flags === 'undefined') {
    options.flags = 0;
  }

  if (this.receiveBuffer.length > 0) {
    return callback(null, this.receiveBuffer.shift());
  }
  // TODO: handle buffered messages with different types

  msg.rcv(this.id, MSGMAX, options.type, options.flags, callback);
};

MessageQueue.prototype.pop = function(options, callback) {
  if (this.listenerCount('data') > 0) {
    return callback(new Error('Do not use \'data\' event with pop()'));
  }

  if (typeof options === 'function') {
    callback = options;
    options = { };
  }

  if (typeof callback === 'undefined') {
    callback = (err) => {
      if (err) throw err;
    };
  }

  options = options || { };
  this._receive(options, callback);
};

MessageQueue.prototype.close = function(callback) {
  callback = callback || Function.prototype;

  if (this.closed) {
    callback(null, true);
    return true;
  }

  try {
    this.closed = msg.close(this.id)
  } catch (err) {
    callback(err, false);
    return false;
  }

  callback(null, true);
  return true;
};

function formatFlags(perms) {
  let uPerms = (perms & 0x006);
  let gPerms = (perms & 0x060) >> 1;
  let oPerms = (perms & 0x600) >> 2;

  perms = uPerms | gPerms | oPerms;

  let flags = 0;
  flags |= MessageQueue.IPC_CREATE;
  flags |= perms;

  return flags;
}

// Static members and constants
const IPC_CREATE = MessageQueue.IPC_CREATE = 512;
const MSGMAX = MessageQueue.MSGMAX = 4052;
MessageQueue.formatFlags = formatFlags;

// Export native bindings
MessageQueue.msg = msg;

module.exports = MessageQueue;
