/**
 * Add gobals here
 */
var seconds 	= null;
var otaTimerVar =  null;
var wifiConnectInterval = null;



/**
 * Initialize functions here.
 */
$(document).ready(function(){
	getSSID();
	getwifiSSID();
	getUpdateStatus();
	getChartvalues();
	startadcSensorInterval();
	startChartvaluesInterval();
	startwifiSSIDInterval();
	getConnectInfo();
	startLocalTimeInterval();
	$("#connect_wifi").on("click", function(){
		checkCredentials();
	}); 
	$("#disconnect_wifi").on("click", function(){
		disconnectWifi();
	}); 
});   

/**
 * Gets file name and size for display on the web page.
 */        
function getFileInfo() 
{
    var x = document.getElementById("selected_file");
    var file = x.files[0];

    document.getElementById("file_info").innerHTML = "<h4>File: " + file.name + "<br>" + "Size: " + file.size + " bytes</h4>";
}

/**
 * Handles the firmware update.
 */
function updateFirmware() 
{
    // Form Data
    var formData = new FormData();
    var fileSelect = document.getElementById("selected_file");
    
    if (fileSelect.files && fileSelect.files.length == 1) 
	{
        var file = fileSelect.files[0];
        formData.set("file", file, file.name);
        document.getElementById("ota_update_status").innerHTML = "Uploading " + file.name + ", Firmware Update in Progress...";

        // Http Request
        var request = new XMLHttpRequest();

        request.upload.addEventListener("progress", updateProgress);
        request.open('POST', "/OTAupdate");
        request.responseType = "blob";
        request.send(formData);
    } 
	else 
	{
        window.alert('Select A File First')
    }
}

/**
 * Progress on transfers from the server to the client (downloads).
 */
function updateProgress(oEvent) 
{
    if (oEvent.lengthComputable) 
	{
        getUpdateStatus();
    } 
	else 
	{
        window.alert('total size is unknown')
    }
}

/**
 * Posts the firmware udpate status.
 */
function getUpdateStatus() 
{
    var xhr = new XMLHttpRequest();
    var requestURL = "/OTAstatus";
    xhr.open('POST', requestURL, false);
    xhr.send('ota_update_status');

    if (xhr.readyState == 4 && xhr.status == 200) 
	{		
        var response = JSON.parse(xhr.responseText);
						
	 	document.getElementById("latest_firmware").innerHTML = response.compile_date + " - " + response.compile_time

		// If flashing was complete it will return a 1, else -1
		// A return of 0 is just for information on the Latest Firmware request
        if (response.ota_update_status == 1) 
		{
    		// Set the countdown timer time
            seconds = 10;
            // Start the countdown timer
            otaRebootTimer();
        } 
        else if (response.ota_update_status == -1)
		{
            document.getElementById("ota_update_status").innerHTML = "!!! Upload Error !!!";
        }
    }
}

/**
 * Displays the reboot countdown.
 */
function otaRebootTimer() 
{	
    document.getElementById("ota_update_status").innerHTML = "OTA Firmware Update Complete. This page will close shortly, Rebooting in: " + seconds;

    if (--seconds == 0) 
	{
        clearTimeout(otaTimerVar);
        window.location.reload();
    } 
	else 
	{
        otaTimerVar = setTimeout(otaRebootTimer, 1000);
    }
}

function getADCsensorvalues(){
	$.getJSON('/adcSensor.json', function(data) {
		$("#adc_reading").text(data["adc_voltage"]);
	})
}
function startadcSensorInterval(){
	setInterval(getADCsensorvalues, 1000);
}
function getChartvalues(){
	$.getJSON('/fft.json', function(data) {
		 // Assuming you have a <div> element with the id "output" in your HTML
  	//var outputDiv = document.getElementById('fft_values');
  
  	// Create a JSON string from the data for better visualization
  	//var jsonString = JSON.stringify(data, null, 2);

  	// Update the content of the <div> with the JSON data
  	//outputDiv.textContent = jsonString;


        
	// Get the canvas element
	var label =[0.000000,195.312500,390.625000,585.937500,781.250000,
976.562500,1171.875000,1367.187500,1562.500000,1757.812500,1953.125000,
2148.437500,2343.750000,2539.062500,2734.375000,2929.687500,3125.000000,
3320.312500,3515.625000,3710.937500,3906.250000,4101.562500,4296.875000,
4492.187500,4687.500000,4882.812500,5078.125000,5273.437500,5468.750000,
5664.062500,5859.375000,6054.687500,6250.000000,6445.312500,6640.625000,
6835.937500,7031.250000,7226.562500,7421.875000,7617.187500,7812.500000,
8007.812500,8203.125000,8398.437500,8593.750000,8789.062500,8984.375000,
9179.687500,9375.000000,9570.312500,9765.625000,9960.937500,10156.250000,
10351.562500,10546.875000,10742.187500,10937.500000,11132.812500,11328.125000,
11523.437500,11718.750000,11914.062500,12109.375000,12304.687500,12500.000000,
12695.312500,12890.625000,13085.937500,13281.250000,13476.562500,13671.875000,
13867.187500,14062.500000,14257.812500,14453.125000,14648.437500,14843.750000,
15039.062500,15234.375000,15429.687500,15625.000000,15820.312500,16015.625000,
16210.937500,16406.250000,16601.562500,16796.875000,16992.187500,17187.500000,
17382.812500,17578.125000,17773.437500,17968.750000,18164.062500,18359.375000,
18554.687500,18750.000000,18945.312500,19140.625000,19335.937500,19531.250000,
19726.562500,19921.875000,20117.187500,20312.500000,20507.812500,20703.125000,
20898.437500,21093.750000,21289.062500,21484.375000,21679.687500,21875.000000,
22070.312500,22265.625000,22460.937500,22656.250000,22851.562500,23046.875000,
23242.187500,23437.500000,23632.812500,23828.125000,24023.437500,24218.750000,
24414.062500,24609.375000,24804.687500,25000.000000,25195.312500,25390.625000,
25585.937500,25781.250000,25976.562500,26171.875000,26367.187500,26562.500000,
26757.812500,26953.125000,27148.437500,27343.750000,27539.062500,27734.375000,
27929.687500,28125.000000,28320.312500,28515.625000,28710.937500,28906.250000,
29101.562500,29296.875000,29492.187500,29687.500000,29882.812500,30078.125000,
30273.437500,30468.750000,30664.062500,30859.375000,31054.687500,31250.000000,
31445.312500,31640.625000,31835.937500,32031.250000,32226.562500,32421.875000,
32617.187500,32812.500000,33007.812500,33203.125000,33398.437500,33593.750000,
33789.062500,33984.375000,34179.687500,34375.000000,34570.312500,34765.625000,
34960.937500,35156.250000,35351.562500,35546.875000,35742.187500,35937.500000,
36132.812500,36328.125000,36523.437500,36718.750000,36914.062500,37109.375000,
37304.687500,37500.000000,37695.312500,37890.625000,38085.937500,38281.250000,
38476.562500,38671.875000,38867.187500,39062.500000,39257.812500,39453.125000,
39648.437500,39843.750000,40039.062500,40234.375000,40429.687500,40625.000000,
40820.312500,41015.625000,41210.937500,41406.250000,41601.562500,41796.875000,
41992.187500,42187.500000,42382.812500,42578.125000,42773.437500,42968.750000,
43164.062500,43359.375000,43554.687500,43750.000000,43945.312500,44140.625000,
44335.937500,44531.250000,44726.562500,44921.875000,45117.187500,45312.500000,
45507.812500,45703.125000,45898.437500,46093.750000,46289.062500,46484.375000,
46679.687500,46875.000000,47070.312500,47265.625000,47460.937500,47656.250000,
47851.562500,48046.875000,48242.187500,48437.500000,48632.812500,48828.125000,
49023.437500,49218.750000,49414.062500,49609.375000,49804.687500];
var ctx = document.getElementById('myChart').getContext('2d');
// Define your data
  var data = {
    labels: label,
    datasets: [{
      label: 'My Chart',
      data: data ,
      backgroundColor: [
        'rgba(255, 99, 132, 0.2)',
        'rgba(54, 162, 235, 0.2)',
        'rgba(255, 206, 86, 0.2)'
      ],
      borderColor: [
        'rgba(255, 99, 132, 1)',
        'rgba(54, 162, 235, 1)',
        'rgba(255, 206, 86, 1)'
      ],
      borderWidth: 1
    }]
  };

// Create the chart
var myChart = new Chart(ctx, {
  type: 'line',
  data: data,
  options: {
    elements: {
      point: {
        radius: 1, // Set the point radius to 0 to make them invisible
      },
    },
  },
});
	})
	// Assuming you already have the chart created (myChart)
myChart.canvas.parentNode.style.width = '800px'; // Set the new width
myChart.canvas.parentNode.style.height = '400px'; // Set the new height
myChart.resize(); // Resize the chart
}

function startChartvaluesInterval(){
	setInterval(getChartvalues, 5000);
}



/**
 * Clears the connection status interval.
 */
function stopWifiConnectStatusInterval()
{
	if (wifiConnectInterval != null)
	{
		clearInterval(wifiConnectInterval);
		wifiConnectInterval = null;
	}
}

/**
 * Gets the WiFi connection status.
 */
function getWifiConnectStatus()
{
	var xhr = new XMLHttpRequest();
	var requestURL = "/wifiConnectStatus";
	xhr.open('POST', requestURL, false);
	xhr.send('wifi_connect_status');
	
	if (xhr.readyState == 4 && xhr.status == 200)
	{
		var response = JSON.parse(xhr.responseText);
		
		document.getElementById("wifi_connect_status").innerHTML = "Connecting...";
		
		if (response.wifi_connect_status == 2)
		{
			document.getElementById("wifi_connect_status").innerHTML = "<h4 class='rd'>Failed to Connect. Please check your AP credentials and compatibility</h4>";
			stopWifiConnectStatusInterval();
		}
		else if (response.wifi_connect_status == 3)
		{
			document.getElementById("wifi_connect_status").innerHTML = "<h4 class='gr'>Connection Success!</h4>";
			stopWifiConnectStatusInterval();
			getConnectInfo();
		}
	}
}

/**
 * Starts the interval for checking the connection status.
 */
function startWifiConnectStatusInterval()
{
	wifiConnectInterval = setInterval(getWifiConnectStatus, 1000);
}

/**
 * Connect WiFi function called using the SSID and password entered into the text fields.
 */
function connectWifi()
{
	// Get the SSID and password
	selectedSSID = $("#wifi_selection").val();
	pwd = $("#connect_pass").val();
	
	$.ajax({
		url: '/wifiConnect.json',
		dataType: 'json',
		method: 'POST',
		cache: false,
		headers: {'my-connect-ssid': selectedSSID, 'my-connect-pwd': pwd},
		data: {'timestamp': Date.now()}
	});
	
	startWifiConnectStatusInterval();
}

/**
 * Checks credentials on connect_wifi button click.
 */
function checkCredentials()
{
	errorList = "";
	credsOk = true;
	
	selectedSSID = $("#connect_ssid").val();
	pwd = $("#connect_pass").val();
	
	if (selectedSSID == "")
	{
		errorList += "<h4 class='rd'>SSID cannot be empty!</h4>";
		credsOk = false;
	}
	if (pwd == "")
	{
		errorList += "<h4 class='rd'>Password cannot be empty!</h4>";
		credsOk = false;
	}
	
	if (credsOk == false)
	{
		$("#wifi_connect_credentials_errors").html(errorList);
	}
	else
	{
		$("#wifi_connect_credentials_errors").html("");
		connectWifi();    
	}
}

/**
 * Shows the WiFi password if the box is checked.
 */
function showPassword()
{
	var x = document.getElementById("connect_pass");
	if (x.type === "password")
	{
		x.type = "text";
	}
	else
	{
		x.type = "password";
	}
}

/**
 * Gets the connection information for displaying on the web page.
 */
function getConnectInfo()
{
	$.getJSON('/wifiConnectInfo.json', function(data)
	{
		$("#connected_ap_label").html("Connected to: ");
		$("#connected_ap").text(data["ap"]);
		
		$("#ip_address_label").html("IP Address: ");
		$("#wifi_connect_ip").text(data["ip"]);
		
		$("#netmask_label").html("Netmask: ");
		$("#wifi_connect_netmask").text(data["netmask"]);
		
		$("#gateway_label").html("Gateway: ");
		$("#wifi_connect_gw").text(data["gw"]);
		
		document.getElementById('disconnect_wifi').style.display = 'block';
	});
}

//disconnects wifi ones the disconnect button pressed
function disconnectWifi(){
	$.ajax({
		url:'/wifiDisconnect.json',
		dataType: 'json',
		method: 'DELETE',
		cache: false,
		data: {'timestamp': Date.now() }
	});
	setTimeout("location.reload(true);", 2000);
}

/**
 * Sets the interval for displaying local time.
 */
function startLocalTimeInterval()
{
	setInterval(getLocalTime, 10000);
}

/**
 * Gets the local time.
 * @note connect the ESP32 to the internet and the time will be updated.
 */
function getLocalTime()
{
	$.getJSON('/localTime.json', function(data) {
		$("#local_time").text(data["time"]);
	});
}

function getSSID(){
	$.getJSON('/apSSID.json', function(data) {
		$("#ap_ssid").text(data["ssid"]);
		
	});
}

function getwifiSSID(){
	$.getJSON('/wifiSSID.json', function(data)
	{
		$("#wifi_selection").find("option").remove().end().append(
                '<option id="option-none" value="">-- Select WiFi --</option>');
		var x = document.getElementById("wifi_selection");
    	var opt = document.createElement('option');
    	opt.value = data["wifi_1"];
    	opt.innerHTML = data["wifi_1"];
    	x.appendChild(opt);
		var opt = document.createElement('option');
    	opt.value = data["wifi_2"];
    	opt.innerHTML = data["wifi_2"];
    	x.appendChild(opt);
		var opt = document.createElement('option');
    	opt.value = data["wifi_3"];
    	opt.innerHTML = data["wifi_3"];
    	x.appendChild(opt);
		var opt = document.createElement('option');
    	opt.value = data["wifi_4"];
    	opt.innerHTML = data["wifi_4"];
    	x.appendChild(opt);
		var opt = document.createElement('option');
    	opt.value = data["wifi_5"];
    	opt.innerHTML = data["wifi_5"];
    	x.appendChild(opt);
	});
}



/**
 * Sets the interval for displaying ssids.
 */
function startwifiSSIDInterval()
{
	setInterval(getwifiSSID, 15000);
}
    










    


