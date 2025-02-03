#!/usr/bin/env python3

import sys
import urllib.parse
from googletrans import Translator

# Read the POST data from stdin
post_data = sys.stdin.read()
form_data = urllib.parse.parse_qs(post_data)

# Get name and message from form data
name = form_data.get('name', [''])[0]
message = form_data.get('message', [''])[0]

# Translate the message
translator = Translator()
translated_message = translator.translate(message, dest='fr').text

# Print the translated message
print(translated_message)