/*********************************************************\
Project: m3m3n70.com 
File: app.js
Description: TCP Forwarder between Arduino and m3m3n70.com
Authors: Turner Bohlen (turnerbohlen.com), Marwan Hilmi
Copyright 2012 Scott St. Technology Company
\*********************************************************/

var http = require('http')
  , net = require('net')
  , url = require('url')
  , fs = require('fs')
  , sys = require(process.binding('natives').util ? 'util' : 'sys')
  , server;
  
var deviceSockets = [];    //We can have multiple local devices sending data to this single server
var mementoSocket = null;  //Only the one "homebase" server for now
var MEMENTO_SITE_PORT;
var MEMENTO_SITE_HOST;

if (process.env.NODE_ENV === 'production') {
    MEMENTO_SITE_PORT = 4116;
    MEMENTO_SITE_HOST = '50.97.248.62';
}
else if (process.env.NODE_ENV === 'development') {
    MEMENTO_SITE_PORT = 4116;
    MEMENTO_SITE_HOST = '50.97.248.62';
}
else if (process.env.NODE_ENV === 'test'){
    MEMENTO_SITE_PORT = 4116;
    MEMENTO_SITE_HOST = '10.0.0.25';
}
else  {
    MEMENTO_SITE_PORT = 4116;
    MEMENTO_SITE_HOST = '10.0.0.25';
}


////////////////////////////////////////////////////////////////////////////////
//////////////////////// m3m3n70.com TCP SOCKET CONNECTION /////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
 * Function: reconnect
 * Attempts to reestablish the connection to the memento site.
 */
var connectToMemento = function(timeout) {
    /*
     * Function: doConnect
     * Attempts to open a tcp connection to the memento site. If it fails,
     * connectToMemento will be called with a slightly longer timout.
     */
    var doConnect = function() {
        console.log('found timeout: ' + timeout.toString());
	console.log('connecting to: ' + MEMENTO_SITE_HOST);
        mementoSocket = net.connect(MEMENTO_SITE_PORT, MEMENTO_SITE_HOST, function() {
            console.log('Connected to the m3m3n70!');
	    timeout = 50;
        });

        /*
         * Event: mementoSocket'data'
         * When we recieve data from memento, broadcast it to all the tcpGuest
         * connections.
         */
        mementoSocket.on('data', function(data) {
            console.log('recieved data from m3m3n70: ' + data);

            for (g in deviceSockets) {
		console.log('Sending data to device socket: ' + g);
                deviceSockets[g].write(data);
            }
        });

        /*
         * Event: mementoTCP 'end'
         * If the connection is cut off, try to reestablish it.
         */
        mementoSocket.on('end', function() {
            console.log('lost connection to m3m3n70!');
            connectToMemento(timeout*2);
        });

        /*
         * Event: mementoTCP 'close'
         * If the connection is cut off, try to reestablish it.
         */
        mementoSocket.on('close', function(hadError) {
            console.log('m3m3n70 site connection closed!');
            if (hadError) {
                console.log('m3m3n70 site connection closed because of error: ');
            }
            connectToMemento(timeout*2);
        });
    };

    console.log('Attempting to connect to m3m3n70...');
    setTimeout(doConnect, timeout);
};

////////////////////////////////////////////////////////////////////////////////
//////////////////////// ARDUINO TCP SOCKET SERVER /////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//TODO : Buffer data if m3m3n70 connection drops

// Sanitize Text for processing/parsing
function sanitizeText(text) {
	return text.toString().replace(/(\r\n|\n|\r)/gm,"");
}

// Socket Data
function receiveData(socket, data) {
	console.log('Received data on device socket: '+data);
	var cleanData = sanitizeText(data);
	//Do something with cleanData if needed
	if(mementoSocket) {
		mementoSocket.write(data);
	}
}
 
// Socket Closed
function closeSocket(socket) {
	//Check for socket and delete
	var i = deviceSockets.indexOf(socket);
	if (i != -1) {
		sockets.splice(i, 1);
	}
}
 
// Socket Opened
function openSocket(socket) {
	deviceSockets.push(socket);
	socket.on('data', function(data) {
		receiveData(socket, data);
	})
	socket.on('end', function() {
		closeSocket(socket);
	})
}



//Create TCP Server for devices on 1337
var deviceSocketServer = net.createServer(openSocket);
deviceSocketServer.listen(1337);
//Connect to Memento first ... ?
connectToMemento(1000);

