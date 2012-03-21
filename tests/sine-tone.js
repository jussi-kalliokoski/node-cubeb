var SAMPLE_RATE = 44100;
var CHANNEL_COUNT = 1;
var BUFFER_SIZE = 4096;
var PLAY_TIME = 4000;

var Cubeb = require('../build/Release/node-cubeb');

console.error('Creating a context...');

var ctx = new Cubeb.Context('my cubeb context');

console.error('Creating a stream...');

var stream = new Cubeb.Stream(ctx, 'test tone', 2, 44100, 4096, 250);
