#!/bin/sh
rec2=$(sendat $1 $2)
rec=$(sendat 1 at+cmgl=4)
index=0
echo "$rec" | while IFS= read -r line; do
    echo "$line" | grep -q '^+CMGL:'
    if [ $? -eq 0 ]; then
        index=$(echo "$line" | awk -F '[ ,]' '{print $2}')
        length=$(echo "$line" | awk -F '[ ,]' '{print $5}')
        read -r pdu_line_1 || break
        read -r pdu_line_2 || break
        pdu=$(echo "$pdu_line_2")
        echo "第${index}条短信" >> /tmp/smsc2.at
        echo " " >> /tmp/smsc2.at
        #echo "PDU数据：" >> /tmp/smsc.at
        #echo "${pdu}" >> /tmp/smsc.at
        #echo "PDU解析后的内容：" >> /tmp/smsc.at
        pdurb=$(echo "${pdu}" | pdu_decoder)
        echo "${pdurb}" >> /tmp/smsc2.at
        echo " " >> /tmp/smsc2.at
        echo "------------------------------------------------------" >> /tmp/smsc2.at
        sed -e '/^Textlen=/d' -e 's/^From:/发件人:/' -e 's/^Date\/Time:/发件时间:/' /tmp/smsc2.at > /tmp/smsc.at

        
    
    fi
done

