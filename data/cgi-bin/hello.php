#!/usr/bin/env php
<?php
// Set Content-Type header
header("Content-Type: text/html");

// Start HTML
echo "<html>";
echo "<head><title>CGI PHP</title></head>";
echo "<body>";

// Print some information about the request
echo "<h1>CGI Script</h1>";
echo "<p>This is a simple CGI script written in PHP.</p>";

// Print environment variables
echo "<h2>Environment Variables</h2>";
echo "<ul>";
foreach ($_SERVER as $key => $value) {
    // If the value is an array, convert it to a string (for example, using print_r)
    if (is_array($value)) {
        $value = print_r($value, true);
    }
    echo "<li>$key: $value</li>";
}
echo "</ul>";

// End HTML
echo "</body>";
echo "</html>";
?>
