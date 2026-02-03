# Lab 1: HTTP Web Proxy Server - Simple Instructions

## What You're Building

A web proxy server that sits between your browser and websites. It catches and stores (caches) web pages so future requests load faster.

```
Browser → Proxy Server → Web Server
        ←              ←
```

## How It Works

1. **Browser** requests a page through the proxy
2. **Proxy** checks if it has the page cached
   - **If YES**: Return cached page (fast!)
   - **If NO**: Request from web server, save to cache, return to browser

---

## Setup Steps

### Step 1: Check Python Version

```bash
python3 --version
```

You need Python 3.6 or higher.

### Step 2: Navigate to Lab Folder

```bash
cd ~/cs434/lab1
```

### Step 3: Understand the Files

- `ProxyServer.py` - Your proxy server code (you'll complete this)
- `cache/` - Folder where cached web pages are stored
- `LAB_INSTRUCTIONS.md` - This file
- `TESTING.md` - How to test your proxy

---

## Coding Tasks

Your job: Fill in the missing code sections marked with:

```python
# Fill in start.
# Fill in end.
```

### Section 1: Create Server Socket (Lines ~10-12)

**What to do:** Bind the socket to your IP and port 8888, then listen

**Hint:** You need:

- `tcpSerSock.bind()`
- `tcpSerSock.listen()`

### Section 2: Receive Client Message (Line ~20)

**What to do:** Receive data from the client

**Hint:** Use `tcpCliSock.recv()`

### Section 3: Send Cached File (Lines ~40-42)

**What to do:** Loop through cached file lines and send each one

**Hint:** Use a for loop with `tcpCliSock.send()`

### Section 4: Create Proxy Socket (Lines ~48-49)

**What to do:** Create a new socket for proxy-to-webserver connection

**Hint:** `socket(AF_INET, SOCK_STREAM)`

### Section 5: Connect to Web Server (Lines ~53-54)

**What to do:** Connect to the web server on port 80

**Hint:** `c.connect((hostn, 80))`

### Section 6: Read Web Server Response (Lines ~60-61)

**What to do:** Read the response from the web server

**Hint:** Use `fileobj.read()` or `fileobj.readlines()`

### Section 7: Save and Forward Response (Lines ~66-67)

**What to do:** Write to cache file AND send to client

**Hint:** Loop and use both `tmpFile.write()` and `tcpCliSock.send()`

### Section 8: Send 404 Error (Lines ~72-73)

**What to do:** Send "file not found" message

**Format:** `"HTTP/1.0 404 Not Found\r\n\r\n"`

### Section 9: Close Server Socket (Lines ~76-77)

**What to do:** Close the server socket

**Hint:** `tcpSerSock.close()`

---

## Testing Your Proxy

### Method 1: Command Line Test

```bash
# Start your proxy server
python3 ProxyServer.py 127.0.0.1

# In another terminal or browser address bar:
http://localhost:8888/www.example.com
```

### Method 2: Configure Your Browser

**Chrome/Edge (Windows/Mac):**

1. Go to Settings → System → Proxy Settings
2. Enable manual proxy
3. HTTP Proxy: `127.0.0.1`
4. Port: `8888`

**Firefox:**

1. Settings → Network Settings → Settings
2. Manual proxy configuration
3. HTTP Proxy: `127.0.0.1`, Port: `8888`
4. Check "Use this proxy for all protocols"

Then visit: `http://www.example.com` (NOT https://)

---

## Testing Checklist

- [ ] Server starts without errors
- [ ] First request fetches from web server
- [ ] File appears in `cache/` folder
- [ ] Second request loads from cache (faster!)
- [ ] Browser displays web page correctly
- [ ] Take screenshots for submission

---

## Common Issues

**"Address already in use"**

- Another program is using port 8888
- Solution: `pkill -9 python3` or use a different port

**"Connection refused"**

- Make sure server is running before making requests
- Check firewall settings

**"404 Not Found"**

- Use HTTP (not HTTPS) URLs for testing
- Example: `http://example.com` ✓ | `https://google.com` ✗

**Nothing in cache folder**

- Check file permissions: `chmod 755 cache/`
- Make sure cache path is correct in code

---

## Submission Requirements

1. ✅ Complete `ProxyServer.py` with all sections filled in
2. ✅ Screenshot: Server running in terminal
3. ✅ Screenshot: Browser showing webpage through proxy
4. ✅ Screenshot: Cached files in `cache/` folder
5. ✅ Screenshot: Terminal output showing "Read from cache"

---

## Debugging Tips

**Print everything while developing:**

```python
print(f"Received message: {message}")
print(f"Filename: {filename}")
print(f"Cache check: {fileExist}")
```

**Test step-by-step:**

1. First, just get the server to accept connections
2. Then, add file caching logic
3. Finally, add web server forwarding

**Check your cache folder:**

```bash
ls -la cache/
cat cache/www.example.com  # View cached file
```

---

## Quick Reference: Socket Methods

```python
# Server side
socket.bind((ip, port))        # Bind to address
socket.listen(5)               # Listen for connections
socket.accept()                # Accept client connection

# Client side
socket.connect((host, port))   # Connect to server
socket.send(data)              # Send data
socket.recv(1024)              # Receive up to 1024 bytes
socket.close()                 # Close connection
```

---

## Next Steps

1. Read through `ProxyServer.py` skeleton code
2. Identify all "Fill in" sections (there are 9 total)
3. Complete one section at a time
4. Test after each section
5. Use `print()` statements liberally for debugging
6. Compare first request (from web) vs second request (from cache)

**Let's code together! Start with Section 1 when ready.**
