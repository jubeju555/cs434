# Testing Guide for HTTP Proxy Server

## Quick Start Testing

### 1. Start the Proxy Server

```bash
cd ~/cs434/lab1
python3 ProxyServer.py 127.0.0.1
```

You should see:

```
Starting proxy server on 127.0.0.1:8888
Proxy server is listening on port 8888...
Ready to serve...
```

### 2. Test with Command Line

**Open a new terminal** and test:

```bash
# Simple HTTP request using curl
curl -x http://127.0.0.1:8888 http://example.com

# Or using wget
wget -e use_proxy=yes -e http_proxy=127.0.0.1:8888 http://example.com -O test_output.html
```

### 3. Test with Browser

**Option A: Direct URL (Easiest)**

- Open browser
- Type in address bar: `http://localhost:8888/example.com`
- Should display the website

**Option B: Configure Browser Proxy**

- Configure proxy settings (see below)
- Visit: `http://example.com`

---

## Browser Proxy Configuration

### Firefox (Recommended for Testing)

1. Open Firefox
2. Settings → General → Network Settings → Settings
3. Select **Manual proxy configuration**
4. HTTP Proxy: `127.0.0.1`
5. Port: `8888`
6. ✓ Check "Also use this proxy for HTTPS"
7. Click OK

### Chrome/Edge (Windows)

1. Settings → System → Open proxy settings
2. LAN Settings
3. ✓ Use a proxy server
4. Address: `127.0.0.1`, Port: `8888`
5. OK

### Chrome (macOS)

1. System Settings → Network
2. Select your network → Advanced
3. Proxies tab
4. ✓ Web Proxy (HTTP)
5. Server: `127.0.0.1`, Port: `8888`
6. OK

### Safari (macOS)

1. Safari → Settings → Advanced → Proxies
2. ✓ Web Proxy (HTTP)
3. Server: `127.0.0.1`, Port: `8888`
4. OK

---

## Test URLs (HTTP Only!)

✅ **These will work:**

- `http://example.com`
- `http://info.cern.ch`
- `http://neverssl.com`
- `http://httpforever.com`

❌ **These won't work (HTTPS not supported):**

- `https://google.com`
- `https://github.com`
- Any URL starting with `https://`

---

## What to Check

### Test 1: First Request (Cache Miss)

1. Start proxy server
2. Request `http://example.com`
3. **Check terminal output:**
   ```
   CACHE MISS! Fetching from web server...
   Connecting to host: example.com
   Connected to example.com:80
   Received XXXX bytes from web server
   Successfully cached and forwarded response to client
   ```
4. **Check cache folder:**
   ```bash
   ls -la cache/
   # Should see: example.com
   ```

### Test 2: Second Request (Cache Hit)

1. Request `http://example.com` again (refresh page)
2. **Check terminal output:**
   ```
   CACHE HIT! File found in cache
   Successfully sent cached file to client
   ```
3. **Notice:** Much faster! No "Connecting to host" message

### Test 3: Different Website

1. Request `http://info.cern.ch`
2. Should fetch from web and cache
3. **Check cache folder:**
   ```bash
   ls cache/
   # Should see: example.com  info.cern.ch
   ```

---

## Debugging Tests

### Test: Port Already in Use

```bash
python3 ProxyServer.py 127.0.0.1
```

**If you see:** `OSError: [Errno 98] Address already in use`

**Solution:**

```bash
# Find the process using port 8888
lsof -i :8888
# Or
netstat -tulpn | grep 8888

# Kill the process
kill -9 <PID>

# Or kill all python processes
pkill -9 python3
```

### Test: Connection Refused

**Problem:** Client can't connect to proxy

**Check:**

1. Is proxy server running? (check terminal)
2. Is it listening on the right port?
3. Firewall blocking connections?

```bash
# Test if port is open
nc -zv 127.0.0.1 8888
# Or
telnet 127.0.0.1 8888
```

### Test: 404 Not Found

**Problem:** Proxy returns 404

**Common causes:**

1. Using HTTPS instead of HTTP
2. Web server is down
3. Hostname resolution issue

**Debug:**

```bash
# Test if you can reach the website directly
curl http://example.com

# Test DNS resolution
nslookup example.com
```

---

## Advanced Testing

### Test with Netcat (Manual HTTP Request)

```bash
# In terminal 1: Start proxy
python3 ProxyServer.py 127.0.0.1

# In terminal 2: Send raw HTTP request
nc 127.0.0.1 8888
```

Then type:

```
GET http://example.com HTTP/1.0

```

(Press Enter twice after the GET line)

### Test Cache Files Manually

```bash
# View cached file
cat cache/example.com

# Should see HTML content starting with:
# HTTP/1.1 200 OK
# <!doctype html>
# <html>
# ...
```

### Test with Multiple Requests

```bash
# Terminal 1: Proxy server running

# Terminal 2: Send multiple requests
for i in {1..5}; do
    echo "Request $i"
    curl -x http://127.0.0.1:8888 http://example.com -o /dev/null -s -w "Time: %{time_total}s\n"
done
```

**Expected:** First request slow, rest fast (cached)

---

## Screenshot Checklist for Submission

### 1. Proxy Server Running

- [ ] Screenshot of terminal showing "Ready to serve..."
- [ ] Should show your proxy listening on port 8888

### 2. First Request (Cache Miss)

- [ ] Screenshot showing "CACHE MISS!"
- [ ] Shows connection to web server
- [ ] Shows bytes received and cached

### 3. Browser Displaying Page

- [ ] Screenshot of browser showing webpage
- [ ] URL bar should show `http://example.com` or `http://localhost:8888/example.com`

### 4. Cache Folder

- [ ] Screenshot of `ls cache/` showing cached files
- [ ] Or screenshot of file browser showing cache contents

### 5. Second Request (Cache Hit)

- [ ] Screenshot showing "CACHE HIT!"
- [ ] Shows file loaded from cache

### 6. Optional: Wireshark Capture

- [ ] Capture showing HTTP GET request through proxy
- [ ] Capture showing proxy-to-server connection

---

## Common Student Mistakes

1. **Using HTTPS URLs** - Only HTTP works!
2. **Forgetting to encode/decode** - Python 3 sockets use bytes
3. **Not closing sockets** - Can cause "address in use" errors
4. **Wrong cache path** - Use `cache/` prefix for all cached files
5. **Not handling exceptions** - Server crashes on bad requests

---

## Performance Testing

### Compare Cache vs No Cache

```bash
# First request (from web server)
time curl -x http://127.0.0.1:8888 http://example.com -o /dev/null -s

# Second request (from cache)
time curl -x http://127.0.0.1:8888 http://example.com -o /dev/null -s
```

**Expected:** Second request should be significantly faster!

---

## Troubleshooting Checklist

- [ ] Python 3.6+ installed?
- [ ] Proxy server actually running?
- [ ] Using HTTP (not HTTPS)?
- [ ] Port 8888 available?
- [ ] Firewall not blocking?
- [ ] Browser proxy configured correctly?
- [ ] `cache/` directory exists?
- [ ] File permissions correct? (`chmod 755 cache/`)

---

## Stop Testing

### Disable Browser Proxy

**Important:** Remember to disable proxy settings after testing!

1. Go back to proxy settings
2. Select "No proxy" or "System proxy"
3. Save changes

Otherwise, normal browsing won't work when proxy is off!

### Stop Proxy Server

In terminal running proxy:

```
Ctrl + C
```

Or force kill:

```bash
pkill -9 python3
```

---

## Ready to Submit?

✅ All fill-in sections completed
✅ Server starts without errors
✅ Can fetch pages from web server
✅ Pages are cached in `cache/` folder
✅ Second requests load from cache
✅ Have all required screenshots
✅ Code is commented and clean

**Good luck! 🚀**
