#!/bin/sh
command -v ipset >/dev/null 2>&1 || exit 0
[ -f "$2" ] || exit 0

ipset -N "$1" hash:net 2>/dev/null

echo "create $1 hash:net family inet hashsize 1024 maxelem 65536" > /tmp/mwan3.ipset
sed -e "s/^/add $1 /" "$2" >> /tmp/mwan3.ipset
ipset -! flush "$1" 2>/dev/null
ipset -! restore < /tmp/mwan3.ipset 2>/dev/null
rm -f /tmp/mwan3.ipset

