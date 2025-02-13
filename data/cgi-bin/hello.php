#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");
?>
<html>
<body>
<h1>Hello from PHP CGI!</h1>
<?php
echo "<p>This is a simple CGI script written in PHP.</p>";

// Print environment variables
echo "<h2>Environment Variables</h2>";
echo "<ul>";
foreach ($_SERVER as $key => $value) {
    echo "<li>$key: $value</li>";
}
echo "</ul>";
?>
</body>
</html>