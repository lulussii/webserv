echo "=== TEST GET ==="
curl -i http://localhost:8080/ > results/GET/getclassic.txt
curl -i http://localhost:8080/nofile.html > results/GET/get404.txt
curl -i http://localhost:8080/html/nofile.html > esults/get4042.txt
curl -i http://localhost:8080/html/secret.html > results/GET/get403.txt
curl -i http://localhost:8080/html/secret > results/GET/get403Directory.txt
curl -i -X PUT http://localhost:8080/test/ > results/GET/get405.txt
curl -i http://localhost:8080/html/ > results/GET/getautoindex.txt
curl -v http://localhost:8080/html/image.png > results/GET/getImage.txt



echo "=== TEST POST ==="
curl -i -X POST -H "Content-Type: text/plain" --data "Bonjour" http://localhost:8080/upload > results/POST/postclassic.txt
curl -i -X POST -H "Content-Type: text/plain" --data "Bonjour" http://localhost:8080 > results/POST/postclassic2.txt
curl -i -X POST -H "Content-Type: text/plain" --data "$(head -c 20000 /dev/zero | tr '\0' 'A')" http://localhost:8080/upload > results/POST/post413.txt
curl -i -X POST -F "file=@/Users/mathildelaussel/webserv/server_files/html/image.png" http://localhost:8080/upload > results/POST/postimage.txt
curl -i -X POST \
-H "Content-Type: text/plain" \
--data-binary @/Users/mathildelaussel/webserv/server_files/test_post.txt \
http://localhost:8080/upload > results/POST/postfile.txt

curl -i -X POST \
-F "file=@/Users/mathildelaussel/webserv/server_files/test_post.txt" \
http://localhost:8080/upload > results/POST/postBoundary.txt

curl -i -X POST \
-H "Content-Type: text/plain" \
--data "test" \
http://localhost:8080/html/index.html > results/POST/post405.txt

curl -i -X POST \
-H "Transfer-Encoding: chunked" \
-H "Content-Type: text/plain" \
-d $'5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n' \
http://localhost:8080/upload > results/POST/postChunked.txt

curl -i -X POST \
-H "Transfer-Encoding: chunked" \
-H "Content-Type: text/plain" \
-d $'ZZZ\r\nHello\r\n6\r\n World\r\n0\r\n\r\n' \
http://localhost:8080/upload > results/POST/post400.txt

echo "=== TEST DELETE ==="
curl -i -X DELETE http://localhost:8080/file.txt > results/DELETE/delete.txt
curl -i -X DELETE http://localhost:8080/nofile.txt > results/DELETE/delete404.txt
curl -i -X DELETE http://localhost:8080/file.txt > results/DELETE/delete403.txt


echo "=== TEST CGI ==="
curl -i http://localhost:8080/cgi/test.php > results/CGI/cgiGet.txt
curl -i -X POST -H "Content-Type: application/x-www-form-urlencoded" --data "param1=val1" http://localhost:8080/cgi/test.php > results/CGI/cgiPost.txt
curl -i http://localhost:8080/cgi/nofile.php > results/CGI/cgi404.txt
curl -i "http://localhost:8080/cgi/test.php?param1=value1&param2=value2" > results/CGI/cgiQuery.txt
