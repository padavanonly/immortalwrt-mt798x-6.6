#!/bin/sh
# Copyright (C) 2023 Siriling <siriling@qq.com>

# Script directory
SCRIPT_DIR="${MODEM_SHARE_DIR:-/usr/share/modem}"
. "${SCRIPT_DIR}/modem_debug.sh"

# Send AT command
# $1: AT serial port
# $2: AT command
at "$1" "$2"