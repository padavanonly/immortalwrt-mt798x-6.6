cription:
- help to build wlan_hwifi driver in colgin

Steps:
1) copy wlan_hwifi folder to colgin sdk path
 a. openwrt/feeds/mtk/kernel/wlan_driver/wlan_hwifi

2) create a symboling link to path for package
 a. cd openwrt/package/feeds/mtk
 b. ln -sf $(your sdk path)/openwrt/feeds/mtk/kernel/wlan_driver/wlan_hwifi wlan_hwifi

3) create a symboling link to path for src
 a. cd $(your sdk path)/mtk/package/kernel/wlan_driver/jedi
 b. ln -sf $(your hwifi src) wlan_hwifi

4) enable colgin compiler flag for wlan_hwifi
 a. file: target/linux/mt6890/evb6890v1_64_cpe/target.config
 b. append: CONFIG_PACKAGE_kmod-wlan_hwifi=y
 c. update configure: cp target/linux/mt6890/evb6890v1_64_cpe/target.config .config ; dockerq "make -j32 defconfig"

5) compiler wlan_hwifi only
 a. dockerq 'make package/feeds/mtk/wlan_hwifi/compile -j1 V=s DISABLE_MTK_BUILD_PRECHECK=yes DISABLE_KCONFIG_CHECK=yes'

6) full compiler and generate image
 a. dockerq "make -O -j32 V=s DISABLE_MTK_BUILD_PRECHECK=yes DISABLE_KCONFIG_CHECK=yes 2>&1 | tee build.log"
