/**
 * These libraries are used in the Web UI tests in the Node.js environment
 * Author: Udbhav Saxena
 */

const jsdom = require("jsdom");
const canvas = require("canvas");
globalThis.jsdom = jsdom;
globalThis.canvas = canvas;
