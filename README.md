# node-cubeb

This is a node.js native binding module to the cross-platform audio library [cubeb](https://github.com/kinetiknz/cubeb).

The purpose of this module is to allow you to write to the user's soundcard.

## Installation

You need to install [cubeb](https://github.com/kinetiknz/cubeb) in order for node-cubeb to work. Refer to directions provided in cubeb on how to do that.

IMPORTANT:

If you're on Linux, make sure that you [have ``` /usr/local/lib ``` in your library path](http://stackoverflow.com/questions/4743233/is-usr-local-lib-searched-for-shared-libraries).

After installing cubeb, installing node-cubeb works like installing any other library in npm:

```
$ npm install cubeb
```

## Usage

cubeb is quite a low level library, so that applies to node-cubeb as well. cubeb has two main concepts, contexts and streams. Ideally, you should use one context per program, while the context can have multiple streams. In order to setup a basic example of playing back pure noise, you would do as follows:

```javascript

var CUBEB = require('cubeb')

var SAMPLE_FORMAT = CUBEB.SAMPLE_INT16LE
var CHANNEL_COUNT = 1
var SAMPLE_RATE = 44100
var BUFFER_SIZE = 4096
var PLAY_TIME = 4000
var LATENCY = 250
var CTX_NAME = "my cubeb context"
var STREAM_NAME = "noisy stream"

function stateCallback (stateID) {
	/* here you can report the state */
}

function dataCallback (frameCount) {
	/* int16 is 2 bytes, doh */
	stream.write(new Buffer(frameCount * 2))
	/* releases used buffers */
	stream.release()

	/*
	 note that we get the noise for free because
	 node buffers aren't initialized to zero.
	*/
}

var ctx = new CUBEB.Context(CTX_NAME)
var stream = new CUBEB.Stream(
	ctx,
	STREAM_NAME,
	SAMPLE_FORMAT,
	CHANNEL_COUNT,
	SAMPLE_RATE,
	BUFFER_SIZE,
	LATENCY,
	datacb,
	statecb
)

stream.start()

```
