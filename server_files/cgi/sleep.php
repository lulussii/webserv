<?php
// On indique qu'on renvoie du HTML
header("Content-Type: text/html");

// Le script s'endort volontairement pendant 10 secondes
sleep(10);

// On affiche la page
echo "<html><body><h1>Bravo !</h1><p>Le zombie PHP s'est reveille au bout de 10 secondes.</p></body></html>";
?>