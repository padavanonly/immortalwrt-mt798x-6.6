#!/bin/sh
rec=$(sendat 1 AT+CMGL=0)
index=0
echo "$rec" | while IFS= read -r line; do
    echo "$line" | grep -q '^+CMGL:'
    if [ $? -eq 0 ]; then
        index=$(echo "$line" | awk -F '[ ,]' '{print $2}')
        length=$(echo "$line" | awk -F '[ ,]' '{print $5}')
        read -r pdu_line_1 || break
        read -r pdu_line_2 || break
        pdu=$(echo "$pdu_line_2")
        echo "第${index}条短信" >> /tmp/smstruns.at
        echo " " >> /tmp/smsc2.at
        pdurb=$(echo "${pdu}" | pdu_decoder)
        echo "${pdurb}" >> /tmp/smstruns.at
        echo " " >> /tmp/smsc2.at
        echo "------------------------------------------------------" >> /tmp/smstruns.at
        sed -e '/^Textlen=/d' -e 's/^From:/发件人:/' -e 's/^Date\/Time:/发件时间:/' /tmp/smstruns.at > /tmp/smstrunt.at
    fi
done
cat /tmp/smstrunt.at
rm -f /tmp/smstruns.at
rm -f /tmp/smstrunt.at
