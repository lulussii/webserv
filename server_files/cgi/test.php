<?php
// ---------------------------------
// Lire le body POST
// ---------------------------------
$body = file_get_contents("php://input");

// ---------------------------------
// Lire les variables CGI envoyées
// ---------------------------------
$request_method   = $_SERVER['REQUEST_METHOD'] ?? '';
$script_filename  = $_SERVER['SCRIPT_FILENAME'] ?? '';
$query_string     = $_SERVER['QUERY_STRING'] ?? '';
$content_type     = $_SERVER['CONTENT_TYPE'] ?? '';
$content_length   = $_SERVER['CONTENT_LENGTH'] ?? '';
$gateway_interface = $_SERVER['GATEWAY_INTERFACE'] ?? '';
$server_protocol  = $_SERVER['SERVER_PROTOCOL'] ?? '';
$server_name      = $_SERVER['SERVER_NAME'] ?? '';
$server_port      = $_SERVER['SERVER_PORT'] ?? '';
$redirect_status  = $_SERVER['REDIRECT_STATUS'] ?? '';

// ---------------------------------
// Envoyer les headers CGI obligatoires
// ---------------------------------
header("Content-Type: text/plain");

// ---------------------------------
// Afficher toutes les infos pour tester
// ---------------------------------
echo "=== TEST CGI COMPLET ===\n";
echo "Received body: " . $body . "\n";
echo "REQUEST_METHOD: " . $request_method . "\n";
echo "SCRIPT_FILENAME: " . $script_filename . "\n";
echo "QUERY_STRING: " . $query_string . "\n";
echo "CONTENT_TYPE: " . $content_type . "\n";
echo "CONTENT_LENGTH: " . $content_length . "\n";
echo "GATEWAY_INTERFACE: " . $gateway_interface . "\n";
echo "SERVER_PROTOCOL: " . $server_protocol . "\n";
echo "SERVER_NAME: " . $server_name . "\n";
echo "SERVER_PORT: " . $server_port . "\n";
echo "REDIRECT_STATUS: " . $redirect_status . "\n";
echo "========================\n";
?>
