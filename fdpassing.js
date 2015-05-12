var fdpassing = require('bindings')('fdpassing.node')
var net = require('net');
//console.log('This should be eight:', fdpassing.fdTransfer(3, 5))

net.createServer(function (socket) {
   var fd=socket._handle.fd;   
 console.log("connected==>client fd:%s",fd);
    


     var client = new net.Socket();
     client.connect('/tmp/echo.pipe', function() {
         console.log('Server Connected server fd:%s',client._handle.fd);
        console.log('This should be eight:', fdpassing.fdTransfer(client._handle.fd, 'Hello', fd))

     });
 
client.on('data', function(data) {
console.log('Received: ' + data);
client.destroy(); // kill client after server's response
});
 
client.on('close', function() {
console.log('Connection closed');
}); 
console.dir(socket._handle);
    socket.on('data', function (data) {
        console.log(data.toString());
    });
})
.listen(7000);
