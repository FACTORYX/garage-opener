var http = require('http')
  , net = require('net')
  , url = require('url')
  , fs = require('fs')
  , sys = require(process.binding('natives').util ? 'util' : 'sys')
  , server;

var tcpGuests = [];
var mementoTCP = null;
var MEMENTO_SITE_PORT;
var MEMENTO_SITE_HOST;

if (process.env.NODE_ENV === 'production') {
    MEMENTO_SITE_PORT = 4116;
    MEMENTO_SITE_HOST = '';
}
else if (process.env.NODE_ENV === 'development') {
    MEMENTO_SITE_PORT = 4116;
    MEMENTO_SITE_HOST = '';
}
else  {
    MEMENTO_SITE_PORT = 4116;
    MEMENTO_SITE_HOST = '';
}

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
        mementoTCP = net.connection({port: MEMENTO_SITE_PORT, host: MEMENTO_SITE_HOST}, function() {
            console.log('connected to the memento site.');
            timeout = 50;
        });

        /*
         * Event: mementoTCP 'data'
         * When we recieve data from memento, broadcast it to all the tcpGuest
         * connections.
         */
        mementoTCP.on('data', function(data) {
            console.log('recieved data from memento site: ' + data);

            for (g in tcpGuests) {
                tcpGuests[g].write(data);
            }
        });

        /*
         * Event: mementoTCP 'end'
         * If the connection is cut off, try to reestablish it.
         */
        mementoTCP.on('end', function() {
            console.log('lost connection to memento site!');
            connectToMemento(timeout*2);
        });

        /*
         * Event: mementoTCP 'close'
         * If the connection is cut off, try to reestablish it.
         */
        mementoTCP.on('close', function(hadError) {
            console.log('Memento site connection closed.');
            if (hadError) {
                console.log('Memento site connection closed because of error: ');
            }
            connectToMemento(timeout*2);
        });
    };

    console.log('attempting to connect to memento...');
    setTimeout(doConnect, timeout);
};

var tcpServer = net.createServer(function (tcpSocket) {});

tcpServer.on('connection',function(tcpSocket){
    console.log('num of connections on port 1337: ' + tcpServer.connections);
    tcpGuests.push(tcpSocket);
    
    tcpSocket.on('data',function(data){
        console.log('received on tcp socket:'+data);
        
        for (g in webGuests) {
            var webSocket = webGuests[g];
            webSocket.send(data.toString('ascii',0,data.length));
            
        }
    })
});

// listen for the arduino connection
tcpServer.listen(1337);
// connect to the memento site
connectToMemento();
