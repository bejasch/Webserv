#!/usr/bin/env php
<?php
// Read the POST data
$post_data = file_get_contents("php://stdin");

// Basic parsing of form data
$data = [];
parse_str($post_data, $data);

$name = isset($data['name']) ? $data['name'] : '';
$message = isset($data['message']) ? $data['message'] : '';

// Function to capitalize every letter of a word
function capitalize_word($word) {
    return strtoupper($word); // Capitalize every letter of the word
}

// Capitalize each word in the message
$capitalized_message = implode(" ", array_map("capitalize_word", explode(" ", $message)));

// Output the capitalized message
echo $capitalized_message;
?>
