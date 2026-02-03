#!/usr/bin/env python3
"""
HTTP Web Proxy Server with Caching
Lab 1 - Computer Networks

Complete the sections marked with:
    # Fill in start.
    # Fill in end.
"""

from socket import *
import sys
import os

# Check command line arguments
if len(sys.argv) <= 1:
    print('Usage: "python ProxyServer.py server_ip"')
    print('[server_ip: It is the IP Address Of Proxy Server]')
    print('Example: python ProxyServer.py 127.0.0.1')
    sys.exit(2)

# Get server IP from command line
server_ip = sys.argv[1]
server_port = 8888
base_dir = os.path.dirname(os.path.abspath(__file__))
cache_dir = os.path.join(base_dir, "cache")
os.makedirs(cache_dir, exist_ok=True)

print(f"Starting proxy server on {server_ip}:{server_port}")

# Create a server socket, bind it to a port and start listening
tcpSerSock = socket(AF_INET, SOCK_STREAM)
tcpSerSock.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)

# Fill in start.
# TODO: Bind the socket to (server_ip, server_port)
tcpSerSock.bind((server_ip, server_port))
# HINT: tcpSerSock.bind((server_ip, server_port))
# TODO: Start listening with a backlog of 5
tcpSerSock.listen(5)
# HINT: tcpSerSock.listen(5)

# Fill in end.

print(f"Proxy server is listening on port {server_port}...")

while True:
    # Start receiving data from the client
    print('\n' + '='*50)
    print('Ready to serve...')
    
    tcpCliSock, addr = tcpSerSock.accept()
    print(f'Received a connection from: {addr}')
    
    # Fill in start.
    # TODO: Receive message from client (use recv with buffer size 4096)
    # HINT: tcpCliSock.recv(4096)
    message = tcpCliSock.recv(4096)
    # Fill in end.
    
    # Decode the message from bytes to string
    try:
        message = message.decode()
    except:
        print("Error decoding message")
        tcpCliSock.close()
        continue
    
    print(f"Request:\n{message}")
    
    # Extract the filename from the given message
    try:
        # Parse the HTTP GET request
        # Format: GET http://www.example.com/index.html HTTP/1.1
        request_line = message.split('\n')[0]
        print(f"Request line: {request_line}")
        
        url = request_line.split()[1]
        if url.startswith("/"):
            url = url[1:]
        print(f"URL: {url}")
        
        # Extract filename (remove leading http:// if present)
        if "//" in url:
            filename = url.partition("//")[2]
        else:
            filename = url
        print(f"Filename: {filename}")
        
        # Remove 'www.' for cleaner filenames
        fileExist = "false"
        filetouse = filename.replace("www.", "", 1)
        filetouse = filetouse.replace("/", "_")
        print(f"Cache file: {filetouse}")
        
    except IndexError:
        print("Error parsing request")
        tcpCliSock.close()
        continue
    
    try:
        # Check whether the file exists in the cache
        print(f"Checking cache for: {os.path.join(cache_dir, filetouse)}")
        f = open(os.path.join(cache_dir, filetouse), "r")
        outputdata = f.readlines()
        fileExist = "true"
        f.close()
        
        print('='*50)
        print('CACHE HIT! File found in cache')
        print('='*50)
        
        # ProxyServer finds a cache hit and generates a response message
        tcpCliSock.send("HTTP/1.0 200 OK\r\n".encode())
        tcpCliSock.send("Content-Type:text/html\r\n\r\n".encode())
        
        # Fill in start.
        # TODO: Send each line of the cached file to the client
        # HINT: Loop through outputdata (which is a list of lines)
        # HINT: Use for loop: for line in outputdata:
        # HINT: Send each line: tcpCliSock.send(line.encode())
        
        for line in outputdata:
            tcpCliSock.send(line.encode())
            
        # Fill in end.
        
        print('Successfully sent cached file to client')
    
    # Error handling for file not found in cache
    except IOError:
        if fileExist == "false":
            print('='*50)
            print('CACHE MISS! Fetching from web server...')
            print('='*50)
            
            # Fill in start.
            # TODO: Create a socket for proxy-to-webserver connection
            # HINT: c = socket(AF_INET, SOCK_STREAM)
            c = socket(AF_INET, SOCK_STREAM)
            # Fill in end.
            # Note: This is a client socket; no bind/listen needed.
            # Extract hostname (remove any path)
            hostn = filename.split('/')[0]
            hostn = hostn.replace("www.", "", 1)
            print(f"Connecting to host: {hostn}")
            
            try:
                # Fill in start.
                # TODO: Connect to the web server on port 80
                # HINT: c.connect((hostn, 80))
                
                c.connect((hostn, 80))
                # Fill in end.
                print(f"Connected to {hostn}:80")
                
                # Create HTTP GET request
                request = f"GET http://{filename} HTTP/1.0\r\nHost: {hostn}\r\n\r\n"
                print(f"Sending request:\n{request}")
                
                # Send request to web server
                c.sendall(request.encode())
                
                # Fill in start.
                # TODO: Read the response from the web server
             
                # HINT: Use a while loop: while True:
                # HINT: Receive data: data = c.recv(4096)
                # HINT: If no more data (data is empty), break
                # HINT: Otherwise, append to buffer: buffer += data
                buffer = b""
                while True:
                    data = c.recv(4096)
                    if not data:
                        break
                    buffer += data
                
                # Fill in end.
                
                print(f"Received {len(buffer)} bytes from web server")
                
                # Create a new file in the cache for the requested file
                print(f"Saving to cache: {os.path.join(cache_dir, filetouse)}")
                tmpFile = open(os.path.join(cache_dir, filetouse), "wb")
                
                # Fill in start.
                # TODO: Write buffer to cache file
                tmpFile.write(buffer)
                # HINT: tmpFile.write(buffer)
                # TODO: Send buffer to client socket
                # HINT: tcpCliSock.send(buffer)
                tcpCliSock.send(buffer)
                
                # Fill in end.
                
                tmpFile.close()
                c.close()
                
                print('Successfully cached and forwarded response to client')
                
            except Exception as e:
                print(f"Error connecting to web server: {e}")
                print("Sending 404 to client")
                
                # Fill in start.
                # TODO: Send HTTP 404 response to client
                # HINT: tcpCliSock.send("HTTP/1.0 404 Not Found\r\n\r\n".encode())
                tcpCliSock.send("HTTP/1.0 404 Not Found\r\n\r\n".encode())
                # Fill in end.
        
        else:
            # This shouldn't happen, but handle it anyway
            print("Unexpected error - file exists but couldn't read")
            tcpCliSock.send("HTTP/1.0 500 Internal Server Error\r\n\r\n".encode())
    
    # Close the client socket
    print('Closing client connection')
    tcpCliSock.close()

# Fill in start.
# TODO: Close the server socket (this code won't normally be reached due to while True loop)
# HINT: tcpSerSock.close()
tcpSerSock.close()
# Fill in end.
