<?php
// test.php
$body = file_get_contents("php://input"); // lit le POST
header("Content-Type: text/plain");
echo "Received body: " . $body;
?>
