

var http = require('http')
  , net = require('net')
  , url = require('url')
  , fs = require('fs')
  , io = require('socket.io')
  , sys = require(process.binding('natives').util ? 'util' : 'sys')
  , server;
    

tcpGuests = [];
webGuests = [];

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

var io = io.listen(server);
io.on('connection', function(webSocket){

  webGuests.push(webSocket);
  webSocket.on('message', function(message){
	console.log(message);

    for (g in tcpGuests) {
        tcpGuests[g].write(message);
    }
  });

});


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
tcpServer.listen(1337);
