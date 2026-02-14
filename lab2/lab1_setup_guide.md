# COSC434/534 Lab 1 Setup and Walkthrough Guide (No Execution)

This guide gives you a clean, step-by-step plan to set up and run the lab. It does **not** perform the lab for you.

## Environment Setup (3 VMs)

- Create 3 Ubuntu SEED VMs: **User**, **DNS Server**, **Attacker**.
- Network:
  - VirtualBox: "NAT Network"
  - VMware: "NAT"
- Assign IPs (example from lab):
  - DNS Server: `10.0.2.16`
  - Attacker: `10.0.2.17`
  - User: `10.0.2.18`
- Verify all VMs can ping each other by IP.

## Task 1: Configure User Machine to Use Local DNS

On **User VM**:

1. Edit `/etc/resolvconf/resolv.conf.d/head` and add:
   ```
   nameserver 10.0.2.16
   ```
2. Apply the change:
   ```
   sudo resolvconf -u
   ```
3. Confirm `/etc/resolv.conf` lists `10.0.2.16` first.

Test:

- Run `dig www.google.com`
- Evidence should show the DNS **SERVER** as `10.0.2.16`.

## Task 2: Configure Local DNS Server (BIND 9)

On **DNS Server VM**:

1. Edit `/etc/bind/named.conf.options`:
   - Add dump-file:
     ```
     dump-file "/var/cache/bind/dump.db";
     ```
   - Disable DNSSEC:
     ```
     # dnssec-validation auto;
     dnssec-enable no;
     ```
2. Restart BIND:
   ```
   sudo service bind9 restart
   ```

Test from **User VM**:

- `ping www.google.com` and `ping www.facebook.com`
- Use Wireshark to capture DNS queries and note when cache is used.

## Task 3: Host a Zone in Local DNS Server

On **DNS Server VM**:

1. Add to `/etc/bind/named.conf`:
   ```
   zone "example.com" {
     type master;
     file "/etc/bind/example.com.db";
   };
   zone "0.168.192.in-addr.arpa" {
     type master;
     file "/etc/bind/192.168.0.db";
   };
   ```
2. Create `/etc/bind/example.com.db` with the forward records from the lab.
3. Create `/etc/bind/192.168.0.db` with the reverse records from the lab.
4. Restart BIND:
   ```
   sudo service bind9 restart
   ```

Test from **User VM**:

- `dig www.example.com`
- Explain why the response is local and authoritative.

## Task 4: Modify Hosts File

On **User VM**:

1. Add entry to `/etc/hosts`:
   ```
   <your_ip> www.bank32.com
   ```
2. Compare `ping www.bank32.com` before and after.
3. Note: `dig` ignores `/etc/hosts`.

## Task 5: Direct Spoofing Response to User

On **Attacker VM**:

- Use `netwox 105` with filter targeting the User VM.

On **User VM**:

- Run `dig www.example.net` while attacker is running.
- Compare output before and after spoofing.

## Task 6: DNS Cache Poisoning (Server)

On **DNS Server VM**:

- Flush cache:
  ```
  sudo rndc flush
  ```

On **Attacker VM**:

- Run `netwox 105` with filter for DNS Server traffic.
- Use `-s raw` and TTL (example: 600 seconds).

Verify:

- Dump cache:
  ```
  sudo rndc dumpdb -cache
  sudo cat /var/cache/bind/dump.db
  ```
- From **User VM**, run `dig www.example.net` and confirm cached spoof.

## Task 7: Poison Authority Section (Scapy)

On **Attacker VM**:

- Write a Scapy script that forges Answer + Authority + Additional sections.

On **DNS Server VM**:

- Flush cache, then trigger query from **User VM**.

Verify:

- Cache entry shows malicious NS.
- Wireshark shows DNS queries going to attacker-controlled NS.

## Submission Checklist

- Screenshots (Wireshark, dig output, cache dumps).
- Notes explaining each observation.
- Code snippets with explanations.
