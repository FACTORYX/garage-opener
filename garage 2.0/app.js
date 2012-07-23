//this app.js will run on the 711 Scott network locally
//it will connected to the arduino via a tcp socket
//it will connect to the external world via a socket.io, port opened on network

var http = require('http')
  , net = require('net')
  , url = require('url')
  , fs = require('fs')
  , io = require('socket.io')
  , sys = require(process.binding('natives').util ? 'util' : 'sys')
  , server;
    
//List of TCP sockets + socketio web sockets
//Later shouldn't be nessecary, socketio keeps list of clients
//Currently crashes because of write to disconnected web socket (they do not get removed from the list on disconnect)
tcpGuests = []; //also not needed because we should only have the 1 arduino connected
webGuests = [];

//create http server to send arduino.html 
//can remove after testing, arduino.html can be loaded locally / anywhere
server = http.createServer(function(req, res){
  var path = url.parse(req.url).pathname;
  switch (path){
    case '/':
      fs.readFile(__dirname + '/arduino.html', function(err, data){
        if (err) return send404(res);
        res.writeHead(200, {'Content-Type': 'text/html'})
        res.write(data, 'utf8');
        res.end();
      });
      break;
      
    default: send404(res);
  }
}),

send404 = function(res){
  res.writeHead(404);
  res.write('404');
  res.end();
};

server.listen(8080);

//create socketio on port 8080
var io = io.listen(server);
io.on('connection', function(webSocket){
 
  //socketio should have own list of open connections
  webGuests.push(webSocket);
  //run like basic web sockets
  webSocket.on('message', function(message){
	console.log(message);
    for (g in tcpGuests) {
		//write to each tcp socket (in our case we should only have the 1 arduino)
        tcpGuests[g].write(message);
    }
  });

});

//create tcp socket server on 1337
var tcpServer = net.createServer(function (tcpSocket) {});

tcpServer.on('connection',function(tcpSocket){
    console.log('num of connections on port 1337: ' + tcpServer.connections);
    //also, not needed, we will only have tcp client
	tcpGuests.push(tcpSocket);
    

    tcpSocket.on('data',function(data){
        console.log('received on tcp socket:'+data);
        for (g in webGuests) {
			//write data directly to the web sockets
            var webSocket = webGuests[g];
            webSocket.send(data.toString('ascii',0,data.length));
            
        }
    })
});

tcpServer.listen(1337);
