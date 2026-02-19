import socket
import time

host = "localhost"
port = 8080

# La requête en morceaux
request_headers = (
    "POST / HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Transfer-Encoding: chunked\r\n"
    "\r\n"
)

chunk1 = "5\r\nHello\r\n"
chunk2 = "6\r\n World\r\n"
end = "0\r\n\r\n"

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    
    # 1. On envoie les headers
    print("Envoi headers...")
    s.send(request_headers.encode())
    time.sleep(1) # Pause pour voir si ton serveur panique
    
    # 2. Premier morceau
    print("Envoi chunk 1...")
    s.send(chunk1.encode())
    time.sleep(1)
    
    # 3. Deuxième morceau
    print("Envoi chunk 2...")
    s.send(chunk2.encode())
    time.sleep(1)
    
    # 4. FIN
    print("Envoi fin...")
    s.send(end.encode())
    
    # Lecture réponse
    response = s.recv(4096)
    print("\nRéponse du serveur :")
    print(response.decode())
    
    s.close()

except Exception as e:
    print(f"Erreur : {e}")