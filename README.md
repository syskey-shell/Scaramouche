# Scaramouche
 A simple tool that checks if Tor onion sites are up or down.
# About Scaramouche
- It's a command line tool that checks if .onion addresses are reachable through your local Tor proxy.
- Flexible HTTP/HTTPS, custom ports, timeouts.
- Can Read from a file and check them all.
  
# Requirements
- Platform: •Linux  •macOS 
- Tor Running locally with SOCKS5 proxy on 127.0.0.1:9050(default)

# why it doesn't support windows?
- well because i built this with posix sockets (socket(), connect()),unix specific timing (clock_gettime()), and standard C libraries that either don't exist on Windows or behave completely differently.
- Windows uses Winsock with different initialization, has its own timing apis.

# Setting Up Tor

```bash
# Check if Tor is running
sudo systemctl status tor

# If not, start it
sudo systemctl start tor

# Enable auto start on boot
sudo systemctl enable tor
```

## Installation
build the binary

```bash
git clone https://github.com/syskey-shell/Scaramouche.git
cd Scaramouche
make
```
## Usage

```bash
# Check single onion
./scaramouche https://duckduckgogg42xjoc72x3sjasowoarfbgcmvfimaftt6twagswzczad.onion

# Check with custom timeout
./scaramouche -t 5 https://duckduckgogg42xjoc72x3sjasowoarfbgcmvfimaftt6twagswzczad.onion

# Batch check from file
./scaramouche OnionLinks.txt
```

# Flags 
| Flags | description                      | Default |
|-------|----------------------------------|---------|
| -t    | Connection timeout in seconds    |  10     | 
| -p    | Port to check                    |  80     |
| -o    | Output results to CSV file       |  none   |
| -q    | Quiet mode (no terminal output)  |  false  |
| -h    | Show help message                |  ---    |

# Timeout Behavior
| Timeout Value | Behavior                                |
|---------------|-----------------------------------------|
| -t 1          | Very aggressivec, may miss slow onions  | 
| -t 5          | Balanced                                |
| -t 10         |  Default, patient                       | 
| -t 30         | 	Very patient, good for slow onions     |  


# Example

<img width="829" height="286" alt="image" src="https://github.com/user-attachments/assets/b3f9e3e5-aa66-477e-b3ef-206f0ce109a1" />







# Limitations
- Single threaded, Checks one onion at a time
- HTTPS limited Only verifies TCP connection, not TLS
- No proxy auth Tor must run without socks5 authentication

# some notes
- Inside your file # are treated as comments.
- wanna ask something? hit me up on discord  `sysbackdoor`
  






