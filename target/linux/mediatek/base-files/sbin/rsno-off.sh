#!/bin/sh
# phy0 interface
uci delete wireless.default_radio0_mld.encryption
uci delete wireless.default_radio0_mld.encryption_rsno
uci delete wireless.default_radio0_mld.ieee80211w

# phy1 interface
uci delete wireless.default_radio1_mld.encryption
uci delete wireless.default_radio1_mld.encryption_rsno
uci delete wireless.default_radio1_mld.ieee80211w

# phy2 interface
uci delete wireless.default_radio2_mld.ieee80211w
uci delete wireless.default_radio2_mld.encryption

# MLD interface
uci delete wireless.ap_mld_1.encryption_rsno_2
uci set wireless.ap_mld_1.encryption=sae-ext-mixed
uci set wireless.ap_mld_1.group_cipher=CCMP
uci set wireless.ap_mld_1.ieee80211w=2

#Save and reload
uci commit wireless
wifi
