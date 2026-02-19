#!/usr/bin/php-cgi
<?php
// Le script doit dire quel type de fichier il renvoie
echo "Content-Type: text/html\r\n\r\n";

echo "<html><body>";
echo "<h1>VICTOIRE ! Le CGI fonctionne !</h1>";
echo "<p>Si tu lis ça, c'est que ton serveur a execute PHP.</p>";
echo "</body></html>";
?>