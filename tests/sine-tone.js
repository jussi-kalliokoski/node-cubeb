require('./tools/testing')

var SAMPLE_FORMAT = CUBEB.SAMPLE_INT16LE
var CHANNEL_COUNT = 1
var SAMPLE_RATE = 44100
var BUFFER_SIZE = 4096
var PLAY_TIME = 4000
var LATENCY = 250
var CTX_NAME = "my cubeb context"
var STREAM_NAME = "test tone"

DEBUG('Creating a context...')

var ctx = new CUBEB.Context(CTX_NAME);

DEBUG('Using backend', JSON.stringify(ctx.backendID))
DEBUG('Asserting context properties...')

ASSERT(ctx.name === CTX_NAME, "(context) name doesn't match")

DEBUG('Creating a stream...')

var p = 0;
var k = Math.PI * 2 * 440 / SAMPLE_RATE;

function datacb (size) {
	DEBUG('Data callback:', size)

	ASSERT(size > 0, "Size is not positive")

	var data = new Buffer(size * 2 * CHANNEL_COUNT);

	for (var i=0; i<size*2; i+=2*CHANNEL_COUNT, p++) {
		data.writeInt16LE(~~(16000 * Math.sin(k * p)), i);
	}

	stream.write(data);
	stream.release();

	DEBUG('Playback position:', stream.position);
}

function statecb (state) {
	DEBUG('State changed:', state)
}

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
);

DEBUG('Asserting stream properties...')

ASSERT(stream.name === STREAM_NAME, "(stream) name doesn't match")
ASSERT(stream.sampleFormat === SAMPLE_FORMAT, "(stream) sampleFormat doesn't match")
ASSERT(stream.channelCount === CHANNEL_COUNT, "(stream) channelCount doesn't match")
ASSERT(stream.sampleRate === SAMPLE_RATE, "(stream) sampleRate doesn't match")
ASSERT(stream.bufferSize === BUFFER_SIZE, "(stream) bufferSize doesn't match")
ASSERT(stream.latency === LATENCY, "(stream) latency doesn't match")

DEBUG('Starting stream...')

stream.start();

DEBUG('Stopping stream after', PLAY_TIME, 'ms')

setTimeout(function () {
	DEBUG('Stopping stream...');
	stream.stop();
	DEBUG('Stopped');
}, PLAY_TIME);
