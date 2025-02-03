#!/usr/bin/env python3

import sys
import random

# Read the POST data from stdin
post_data = sys.stdin.read()

# Basic parsing of form data
data = {}
for item in post_data.split("&"):
    key, value = item.split("=")
    data[key] = value

name = data.get("name", "")
message = data.get("message", "")

# Function to scramble words
def scramble_word(word):
    if len(word) <= 1:
        return word
    word_list = list(word)
    random.shuffle(word_list)
    return ''.join(word_list)

# Scramble each word in the message
scrambled_message = " ".join([scramble_word(word) for word in message.split()])

# Print the scrambled message
print(scrambled_message)
