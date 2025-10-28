#!/bin/sh

cfg80211_tool() {
	[ -f "/usr/sbin/mwctl" ] && {
		/usr/sbin/mwctl $@
		return
	}

	[ -f "/usr/sbin/iwpriv" ] && {
		/usr/sbin/iwpriv $@
		return
	}
}

set_testmode_en() {
	[ "$1" = "0" ] && {
		cfg80211_tool ra0 set testmode_en=0
	} || {
		cfg80211_tool ra0 set testmode_en=1
	}

	for idx in 0 1 2 3; do
		option="INDEX${idx}_profile_path"
		profile_path="$(grep -i "$option" /etc/wireless/l1profile.dat | awk -F'=' '{print $2}' | tr -d ' \n')"
		[ -n "$profile_path" -a -f "$profile_path" ] && {
			[ "$1" = "0" ] && {
				sed -i 's/TestModeEn=1/TestModeEn=0/g' $profile_path
			} || {
				sed -i 's/TestModeEn=0/TestModeEn=1/g' $profile_path
			}
		}
	done
}


if [ $1 == "1" ]; then
	echo "TestModeEn=1, Test mode"
	set_testmode_en 1
	wifi down
	wifi up
elif [ $1 == "0" ]; then
	echo "TestModeEn=0, Normal mode"
	set_testmode_en 0
	wifi down
	wifi up
fi

