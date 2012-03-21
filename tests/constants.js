var DEBUG = console.error.bind(console);

function ASSERT (assertion, message, code) {
	if (!assertion) {
		DEBUG("Assertion failed:", message);
		process.exit(typeof code === 'undefined' ? 1 : code);
	}
}

function TEST_CONSTANT (name) {
	ASSERT(typeof Cubeb[name] === 'number', 'typeof ' + name + ' === "number"');
}

var Cubeb = require('../build/Release/node-cubeb');

DEBUG("Asserting constants...")

TEST_CONSTANT("SAMPLE_INT16LE")
TEST_CONSTANT("SAMPLE_INT16BE")
TEST_CONSTANT("SAMPLE_INT16NE")
TEST_CONSTANT("SAMPLE_FLOAT32LE")
TEST_CONSTANT("SAMPLE_FLOAT32BE")
TEST_CONSTANT("SAMPLE_FLOAT32NE")

TEST_CONSTANT("CUBEB_OK")
TEST_CONSTANT("CUBEB_ERROR")
TEST_CONSTANT("CUBEB_ERROR_INVALID_FORMAT")

TEST_CONSTANT("STATE_STARTED")
TEST_CONSTANT("STATE_STOPPED")
TEST_CONSTANT("STATE_DRAINED")
TEST_CONSTANT("STATE_ERROR")

DEBUG("Success.")
