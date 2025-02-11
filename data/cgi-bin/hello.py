#!/usr/bin/env python3

import cgi
import os

    
# Set Content-Type header
print("Content-Type: text/html\n")

# Start HTML
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")

# Print some information about the request
print("<h1>CGI Test Script</h1>")
print("<p>This is a simple CGI script written in Python.</p>")

# Print environment variables
print("<h2>Environment Variables</h2>")
print("<ul>")
for key, value in os.environ.items():
    print(f"<li>{key}: {value}</li>")
print("</ul>")

# End HTML
print("</body>")
print("</html>")

# erg = 1 / 0 # Uncomment to test error handling
