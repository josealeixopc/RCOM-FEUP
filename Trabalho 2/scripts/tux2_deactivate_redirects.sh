echo 0 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
echo 0 > /proc/sys/net/ipv4/conf/all/accept_redirects
route del -net 172.16.40.0 gw 172.16.41.253 netmask 255.255.255.0 dev eth0