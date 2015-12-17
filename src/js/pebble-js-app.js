//host = "192.168.1.173"
host = "101.5.230.29"
port = "9001"
wsUrl = 'ws://' + host + ':' + port + '/';
var client = new WebSocket(wsUrl);

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

client.sendPebbleMessage = function(msg) {
	var dictionary = {
		"KEY_STATUS": 0,
		"KEY_TITLE": "title",
		"KEY_AUTHOR": "author",
		"KEY_CURRENT": 0,
		"KEY_DURATION": 0,
		"KEY_VOLUME": 0,
	};

	Pebble.sendAppMessage(dictionary,
		function(e) {
			console.log("Status send successfully");
		},
		function(e) {
			console.log("Error in sending");
		}
	);
}

var getStatus = function() {
	client.send("status");
}

Pebble.addEventListener('ready',
	function(e) {
		console.log("Pebble ready");
		getStatus();
	}
);

var sendQuery = function(client, payload) {
	switch (payload["KEY_STATUS"]) {
		case 0:
			client.send("status");
			break;
		case 1:
			client.send("idle");
			break;
		case 2:
			client.send("play");
			break;
		case 3:
			client.send("pause");
			break;
		case 4:
			client.send("previous");
			break;
		case 5:
			client.send("next");
			break;
		case 6:
			client.send("increase_volume");
			break;
		case 7:
			client.send("decrease_volume");
			break;
	}
}

Pebble.addEventListener('appmessage',
	function(e) {
		console.log("App Message received!" + JSON.stringify(e.payload));
		sendQuery(client, e.payload);
	}
);
