#!/usr/bin/env node

var hprotocol = require('hprotocol');
var producer = require('./producer');
var subscriber = require('./subscriber');

var addr = process.argv[2] || '127.0.0.1';

var pub = producer(addr);
var sub = subscriber(addr);

var protocol = hprotocol()
	.use('pub channel message')

var p = protocol();

sub(function(channel, message) {
	console.log('> '+channel+' '+message);
});

p.on('pub', function(channel, message) {
	pub(channel, message);
});

process.stdin.pipe(p.stream);