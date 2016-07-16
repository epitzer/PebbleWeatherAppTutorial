Pebble.addEventListener("ready", function(e) {
  console.log("Ready for interaction");
});

Pebble.addEventListener("appmessage", function(e) {
  console.log("Got testkey with value" + e.payload.testkey);
  
  var object = {
    testkey: 0
  };
  
  Pebble.sendAppMessage(object);
  
});