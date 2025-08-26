CMM_SECURITY_PATH := $(PARENT_DIR)/common/security

obj_common += \
	$(CMM_SECURITY_PATH)/crypt_md5.o\
	$(CMM_SECURITY_PATH)/crypt_sha2.o\
	$(CMM_SECURITY_PATH)/crypt_hmac.o\
	$(CMM_SECURITY_PATH)/crypt_aes.o\
	$(CMM_SECURITY_PATH)/crypt_arc4.o\
	$(CMM_SECURITY_PATH)/crypt_biginteger.o\
	$(CMM_SECURITY_PATH)/cmm_wep.o\
	$(CMM_SECURITY_PATH)/cmm_tkip.o\
	$(CMM_SECURITY_PATH)/cmm_aes.o\
	$(CMM_SECURITY_PATH)/cmm_wpa.o\
	$(CMM_SECURITY_PATH)/cmm_sec.o\
	$(CMM_SECURITY_PATH)/mlo_sec.o

ifeq ($(CONFIG_WAPI_SUPPORT),y)
obj_common += \
	$(CMM_SECURITY_PATH)/wapi.o\
	$(CMM_SECURITY_PATH)/wapi_crypt.o\
	$(CMM_SECURITY_PATH)/wapi_sms4.o
endif

ifeq ($(CONFIG_WPA3_SUPPORT),y)
obj_common += \
	$(CMM_SECURITY_PATH)/crypt_bignum.o
endif

ifeq ($(CONFIG_WSC_INCLUDED),y)
obj_common += \
	$(CMM_SECURITY_PATH)/crypt_dh.o
endif

