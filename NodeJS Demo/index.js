var request = require('request');

var toddDeviceAuthToken = 'b981de070eea417e9496c17e8c1d8e97';

console.log("Program Started");

// // Demo of how to read data out of the blynk server for a specific device
// request('http://iot.nortcele.win:8080/'+toddDeviceAuthToken+'/get/V1', function (error, response, body) {
//     insideTempStr = JSON.parse(body)[0].substr(0, 4);
//     console.log(insideTempStr);
// });

// Demo of how to send data to a specific device through the blynk server
request('http://iot.nortcele.win:8080/'+toddDeviceAuthToken+'/update/V8?value=hello', function (error, response, body) {
    console.log("Message Sent!");
});
    