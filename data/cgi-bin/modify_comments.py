#!/usr/bin/env python3

import sys
import urllib.parse

# Read the POST data from stdin
post_data = sys.stdin.read()
form_data = urllib.parse.parse_qs(post_data)

# Get name and message from form data
name = form_data.get('name', [''])[0]
message = form_data.get('message', [''])[0]

# Modify the message (example modification)
modified_message = message.upper() + " 😂 JOKIFIED!"

# Print the modified message
print(modified_message)