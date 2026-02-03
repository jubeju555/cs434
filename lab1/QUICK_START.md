# Lab 1 Quick Reference

## File Structure

```
cs434/lab1/
├── ProxyServer.py          # Your main code (complete the Fill in sections)
├── LAB_INSTRUCTIONS.md     # Detailed instructions
├── TESTING.md              # How to test everything
├── QUICK_START.md          # This file
└── cache/                  # Where cached pages are stored
```

## Complete the Lab in 4 Steps

### Step 1: Read Instructions (5 min)

```bash
cd ~/cs434/lab1
cat LAB_INSTRUCTIONS.md
```

### Step 2: Complete Code (30-45 min)

Open `ProxyServer.py` and fill in **9 sections**:

1. Bind and listen server socket
2. Receive client message
3. Send cached file to client
4. Create proxy socket
5. Connect to web server
6. Read web server response
7. Save to cache and forward to client
8. Send 404 error
9. Close server socket

### Step 3: Test It (15 min)

```bash
# Start server
python3 ProxyServer.py 127.0.0.1

# In another terminal or browser:
http://localhost:8888/example.com
```

### Step 4: Take Screenshots (10 min)

- Server running in terminal
- Cache miss message
- Cache hit message (second request)
- Browser showing webpage
- Files in cache/ folder

---

## The 9 Code Sections Explained

### 1. Server Socket Setup

```python
# You need to:
tcpSerSock.bind((server_ip, server_port))
tcpSerSock.listen(5)
```

### 2. Receive Client Message

```python
message = tcpCliSock.recv(4096)
```

### 3. Send Cached File

```python
for line in outputdata:
    tcpCliSock.send(line.encode())
```

### 4. Create Proxy Socket

```python
c = socket(AF_INET, SOCK_STREAM)
```

### 5. Connect to Web Server

```python
c.connect((hostn, 80))
```

### 6. Read Response

```python
while True:
    data = c.recv(4096)
    if not data:
        break
    buffer += data
```

### 7. Save and Forward

```python
tmpFile.write(buffer)
tcpCliSock.sendall(buffer)
```

### 8. Send 404

```python
tcpCliSock.send("HTTP/1.0 404 Not Found\r\n\r\n".encode())
```

### 9. Close Server

```python
tcpSerSock.close()
```

---

## Testing Commands

```bash
# Start proxy
python3 ProxyServer.py 127.0.0.1

# Test with curl (in another terminal)
curl -x http://127.0.0.1:8888 http://example.com

# Or browser address bar:
http://localhost:8888/example.com

# Check cache
ls -la cache/
cat cache/example.com
```

---

## Common Issues & Fixes

| Problem            | Solution                        |
| ------------------ | ------------------------------- |
| Port in use        | `pkill -9 python3` then restart |
| Connection refused | Make sure server is running     |
| 404 error          | Use HTTP not HTTPS              |
| Nothing in cache   | Check `cache/` folder exists    |
| Decode error       | Use `.encode()` and `.decode()` |

---

## Test URLs (HTTP Only)

✅ http://example.com
✅ http://info.cern.ch  
✅ http://neverssl.com
❌ https://google.com (HTTPS not supported)

---

## Expected Output

### First Request (Cache Miss):

```
Ready to serve...
Received a connection from: ('127.0.0.1', 54321)
Request line: GET http://example.com HTTP/1.0
==================================================
CACHE MISS! Fetching from web server...
==================================================
Connecting to host: example.com
Connected to example.com:80
Received 1256 bytes from web server
Saving to cache: cache/example.com
Successfully cached and forwarded response to client
```

### Second Request (Cache Hit):

```
Ready to serve...
Received a connection from: ('127.0.0.1', 54322)
==================================================
CACHE HIT! File found in cache
==================================================
Successfully sent cached file to client
```

---

## Submission Checklist

- [ ] `ProxyServer.py` completed (all 9 sections)
- [ ] Screenshot: Terminal showing server running
- [ ] Screenshot: Cache miss (first request)
- [ ] Screenshot: Cache hit (second request)
- [ ] Screenshot: Browser displaying webpage
- [ ] Screenshot: Files in cache/ folder

---

## Time Estimate

- Reading instructions: 10 min
- Coding: 30-45 min
- Testing: 15-20 min
- Screenshots: 10 min
- **Total: ~1-1.5 hours**

---

## Need Help?

1. Check `LAB_INSTRUCTIONS.md` for detailed explanations
2. Check `TESTING.md` for debugging tips
3. Print debug info: `print(f"Variable: {variable}")`
4. Test one section at a time

**Ready? Open ProxyServer.py and start coding!** 🚀
