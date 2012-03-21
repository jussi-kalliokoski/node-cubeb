global.DEBUG = console.error.bind(console);

global.ASSERT = function (assertion, message, code) {
	if (!assertion) {
		DEBUG("Assertion failed:", message);
		process.exit(typeof code === 'undefined' ? 1 : code);
	}
};

global.TEST_CONSTANT = function (name) {
	ASSERT(typeof CUBEB[name] === 'number', 'typeof ' + name + ' === "number"');
};

global.CUBEB = require('../../build/Release/node-cubeb');
