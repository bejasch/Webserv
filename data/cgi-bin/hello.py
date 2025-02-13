#!/usr/bin/env python3

import cgi
import os
import time

# print("Start")
# time.sleep(30)  # Pause for 2 seconds
# print("End")

#!/usr/bin/env python3
print("HTTP/1.1 200 OK")
print("Content-Type: text/html")
print()  # Empty line to separate headers from body
print("<html><body><h1>Hello from Python CGI!</h1></body></html>")

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
