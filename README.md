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
