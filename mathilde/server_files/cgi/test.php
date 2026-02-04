<?php
// lit le body POST
$body = file_get_contents("php://input");

// headers CGI
header("Content-Type: text/plain");

// output
echo "Received body: " . $body . "\n";
echo "QUERY_STRING: " . $_SERVER['QUERY_STRING'] . "\n";
echo "REQUEST_METHOD: " . $_SERVER['REQUEST_METHOD'] . "\n";
?>
