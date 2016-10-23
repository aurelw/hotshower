    var mqtt;
    var reconnectTimeout = 2000;

    function MQTTconnect() {
        mqtt = new Paho.MQTT.Client(
                        host,
                        port,
                        "web_" + parseInt(Math.random() * 100,
                        10));
        var options = {
            timeout: 3,
            useSSL: useTLS,
            cleanSession: cleansession,
            onSuccess: onConnect,
            onFailure: function (message) {
                $('#status').val("Connection failed: " + message.errorMessage + "Retrying");
                setTimeout(MQTTconnect, reconnectTimeout);
            }
        };

        mqtt.onConnectionLost = onConnectionLost;
        mqtt.onMessageArrived = onMessageArrived;

        if (username != null) {
            options.userName = username;
            options.password = password;
        }
        console.log("Host="+ host + ", port=" + port + " TLS = " + useTLS + " username=" + username + " password=" + password);
        mqtt.connect(options);
    }

    function onConnect() {
        $('#status').val('Connected to ' + host + ':' + port);
        // Connection succeeded; subscribe to our topic
        mqtt.subscribe(topic, {qos: 0});
        $('#topic').val(topic);
    }

    function onConnectionLost(response) {
        setTimeout(MQTTconnect, reconnectTimeout);
        $('#status').val("connection lost: " + responseObject.errorMessage + ". Reconnecting");

    };

    function guid() {
      function s4() {
        return Math.floor((1 + Math.random()) * 0x10000)
      .toString(16)
      .substring(1);
    }
     return s4() + s4() + '-' + s4() + '-' + s4() + '-' +
       s4() + '-' + s4() + s4() + s4();
    }

var oldcount;
var oldtopic;
var oldpayload;
var olduid;
var timestring;

    function onMessageArrived(message) {

        var topic = message.destinationName;
        var payload = message.payloadString;

	

	if (topic == "mik/bullshit") {
	        $('#ws').prepend('<li><h1> bullshit </h1></li>');
		var audio = new Audio('bullshit.mp3');
	        audio.play();
		}


	var uid = guid();	

	timestring = Date().replace(/z|t/gi,' ').replace(/\ GM.*/,'').trim();

	if (topic === oldtopic && payload === oldpayload) {
		count = oldcount + 1;
		uid = olduid;

		$('#' + olduid).replaceWith('<li id=' + uid + '>' + timestring + ' count: '  + count + ' ' + topic + ' = ' + payload + '</li>');
	} else {
		count = 1;
  if (payload.startsWith("data:image/jpeg;base64") || payload.startsWith("data:image/png;base64")) {
        	$('#ws').prepend('<li id=' + uid + '>' + timestring + ' count: '  + count + ' ' + topic + ' = <img src="' + payload + '"></li>');
	} else {
        	$('#ws').prepend('<li id=' + uid + '>' + timestring + ' count: '  + count + ' ' + topic + ' = ' + payload + '</li>');
	}
	}


	oldcount = count;
	oldtopic = topic;
	oldpayload = payload;
	olduid = uid;
    };


    $(document).ready(function() {
        MQTTconnect();
    });
