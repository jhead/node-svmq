const MessageQueue = require('../');

const inputs = [
  Buffer.alloc(0),
  Buffer.from(""),
  Buffer.from("test"),
  Buffer.from("test 1234 foo bar 567890"),
  Buffer.from([1, 2, 3, 4]),
  Buffer.from(new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8])),
  Buffer.alloc(1024)
];

describe("MessageQueue", function () {

  // Test a batch of diverse inputs that should be supported
  inputs.forEach((input, index) =>{
    const msgType = index + 1;
    const selectedInput = input;

    it("push and pop a message: " + selectedInput, function (done) {
      const queue = new MessageQueue(31337);
  
      queue.push(selectedInput, { type: msgType })
      queue.pop({ type: msgType }, function(err, msg) {
        expect(err).toBeNull();
        expect(msg).toEqual(selectedInput);
        done();
      })
    });
  })

});