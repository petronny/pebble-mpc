var W3CWebSocket = require('websocket').w3cwebsocket;
 
//host = "192.168.1.173"
host = "127.0.0.1"
port = "9001"
wsUrl = 'ws://'+host+':'+port+'/';
var client = new W3CWebSocket(wsUrl);
 
client.onerror = function() {
    console.log('Connection Error');
};
 
client.onopen = function() {
    console.log('WebSocket Client Connected');
};
 
client.onclose = function() {
    console.log('echo-protocol Client Closed');
};
 
client.onmessage = function(e) {
    if (typeof e.data === 'string') {
        console.log("Received: '" + e.data + "'");
    }
};

client.sendPebbleMessage = function (msg) {
  var dictionary = {
    "KEY_STATUS": 0,
    "KEY_TITLE"::  "title",
    "KEY_AUTHOR":  "author",
    "KEY_CURRENT": 0,
    "KEY_DURATION": 0,
    "KEY_VOLUME": 0,
  };

  Pebble.sendAppMessage(dictionary,
    fucntion(e) {
      console.log(Status send successfully");
    },
    function(e) {
      console.log(Error in sending");
    }
  );
}

var getStatus = function () {
  client.send("status");
}

Pebble.addEventListener('ready',
  function(e) {
    console.log("Pebble redady");
    getStatus();
  }
);

var sendQuery = function(client,payload) {
     switch (payload["0"]) {
      case 0:
        client.send("0");
        break;
      case 1:
        client.send("1");
        break;
      case 2:
        client.send("2");
        break;
      case 3:
        client.send("3");
        break;
      case 4:
        client.send("4");
        break;
   }
}
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("App Message received!" + JSON.stringify(e.payload) ); 
    sendQuery(client,e.payload);
  }
);
