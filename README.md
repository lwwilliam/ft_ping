# FT_PING

# ping
- used to test and verify a host is reachable on an Internet Protocol (IP) network. It works by sending packets from the source to the target host which, if it is accessible through the network, sends packets back. The source host then analyzes the returning packets to determine if the target host is accessible and calculates simple statistics of packets traversing the network.


# BASIC UNDERSTANDING
- The ping command sends a series of Internet Control Message Protocol (ICMP) Echo Request packets to the target host and waits for ICMP Echo Reply packets in return.

Source Host
- The source host sends an ICMP Echo Request packet to the target host.
- The source host waits for a response from the target host.

Target Host
- The target host receives the ICMP Echo Request packet.
- The target host processes the request and sends back an ICMP Echo Reply packet to the source host.
- The target host may also send an ICMP Time Exceeded message if the packet's TTL (Time to Live) value reaches zero before reaching its destination.

# PING packet breakdown
PING 1.1.1.1 (1.1.1.1) 56(84) bytes of data.
- 56: This is the size of the ICMP payload (data) in bytes.
- 84: This is the total size of the packet sent over the wire, in bytes.

# Payload data
- ICMP header: 8 bytes
- ICMP payload: 56 bytes (default for ping)
- IP header: 20 bytes (standard IPv4 header)
- So, total size = 20 (IP) + 8 (ICMP header) + 56 (payload) = 84 bytes.

56 = data bytes (payload)

84 = total bytes sent (IP header + ICMP header + payload)

# Checksum
- The checksum is a value used to verify the integrity of data transmitted over a network.
- The checksum is included in the packet header and is used by the receiving host to verify that the data has not been corrupted during transmission.

# ICMP
- ICMP (Internet Control Message Protocol) is a network layer protocol used by network devices to send error messages and operational information.
- ICMP is used by the ping command to send Echo Request and Echo Reply messages.