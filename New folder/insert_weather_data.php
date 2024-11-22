<?php
$servername = "localhost"; // Server name
$username = "root";        // MySQL username
$password = "";            // MySQL password
$dbname = "weather_station"; // Database name

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Get data from URL parameters
$temperature = $_GET['temperature'];
$humidity = $_GET['humidity'];
$ldrValue = $_GET['ldrValue'];
$rainStatus = $_GET['rainStatus'];
$windSpeed = $_GET['windSpeed'];

// Insert data into the table
$sql = "INSERT INTO weather_data (temperature, humidity, ldr_value, rain_status, wind_speed)
        VALUES ('$temperature', '$humidity', '$ldrValue', '$rainStatus', '$windSpeed')";

if ($conn->query($sql) === TRUE) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

// Close connection
$conn->close();
?>
