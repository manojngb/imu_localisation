import socket
import sys
import time

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# Bind the socket to the port
server_address = ('localhost', 23458)
print('starting up on ', server_address)
sock.bind(server_address)
# Listen for incoming connections
sock.listen(1)

streaming = False

while True:
    # Wait for a connection
    print('waiting for a connection')
    connection, client_address = sock.accept()
    try:
        print('connection from ', client_address)

        # Receive the data in small chunks and retransmit it
        while True:
            time.sleep(0.100)
            connection.sendall(b'0,0,1,173\n')
            time.sleep(0.100)
            connection.sendall(b'0,0,1,174\n')
    except:
        print('client disconnected')