var configSettings = null;

Pebble.addEventListener("ready", function(e) {
	console.log('PebbleKit JS ready');
});

Pebble.addEventListener("showConfiguration", function (e) {
	var url = "http://tiletime.lankamp.org:8111/dutch15.html?";

	console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener("webviewclosed", function(e) {
  if ((typeof e.response === 'string') && (e.response.length > 0)) {
    var configData = JSON.parse(decodeURIComponent(e.response));
	  console.log('Configuration page returned: ' + JSON.stringify(configData));
  
    if (configData.version) {
      console.log("foreground: " + parseInt(configData.foreground, 16));
      console.log("background: " + parseInt(configData.background, 16)); 
      Pebble.sendAppMessage({
        version: parseInt(configData.version),
        foreground: parseInt(configData.foreground, 16),
        background: parseInt(configData.background, 16)
      }, function() {
          console.log('Send succesful!');
      }, function() {
          console.log('Send failed!');
          Pebble.showSimpleNotificationOnPebble("TileTime", "Receiving setting from phone failed!");
      });
    }
  } else {
    if (e.response.length > 0) {
      console.log('Configuration page did not return any value!');      
    }
    if (typeof e.response === 'string') {
      console.log('Configuration page returned no string!');         
    }
  }
});
