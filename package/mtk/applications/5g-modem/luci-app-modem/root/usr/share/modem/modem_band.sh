#!/bin/sh
# Copyright (C) 2023 Siriling <siriling@qq.com>

# Script directory
SCRIPT_DIR="${MODEM_SHARE_DIR:-/usr/share/modem}"
source "${SCRIPT_DIR}/modem_debug.sh"

# Initialize modem bands
init_modem_band()
{
	#2G
	DCS_1800="-"
	E-GSM_900="-"
	P-GSM_900="-"
	GSM_450="-"
	GSM_480="-"
	GSM_750="-"
	GSM_850="-"
	R-GSM_900="-"
	PCS_1900="-"

	#3G

	#4G
	B1="-"
	B2="-"
	B3="-"
	B4="-"
	B5="-"
	B6="-"
	B7="-"
	B8="-"
	B9="-"
	B10="-"
	B11="-"
	B12="-"
	B13="-"
	B14="-"
	B17="-"
	B18="-"
	B19="-"
	B20="-"
	B21="-"
	B25="-"
	B26="-"
	B28="-"
	B29="-"
	B30="-"
	B32="-"
	B34="-"
	B38="-"
	B39="-"
	B40="-"
	B41="-"
	B42="-"
	B66="-"
	B71="-"

	#5G
	N1="-"
	N2="-"
	N3="-"
	N5="-"
	N7="-"
	N8="-"
	N12="-"
	N20="-"
	N25="-"
	N28="-"
	N38="-"
	N40="-"
	N41="-"
	N48="-"
	N66="-"
	N71="-"
	N77="-"
	N78="-"
	N79="-"
}

# Get modem data info
# $1: AT serial port
# $2: Manufacturer
# $3: Platform
# $4: Connection definition
modem_band()
{
	# Initialize modem bands
    debug "Initializing modem band"
    init_modem_band
    debug "Modem band initialization completed"
}

modem_band "$1" "$2" "$3" "$4"