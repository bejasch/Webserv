!/usr/bin/env python3

import os
import cgi

# Get the form data from the environment variables
form_data = cgi.FieldStorage()

# Get 'name' and 'comment' from the form data
name = form_data.getvalue("name", "")
comment = form_data.getvalue("comment", "")

# Apply something "funny" to the comment (e.g., reverse the comment and add a silly message)
funny_comment = comment[::-1]  # Reverse the comment
funny_comment += " 😂😂😂 This is your hilarious comment!"