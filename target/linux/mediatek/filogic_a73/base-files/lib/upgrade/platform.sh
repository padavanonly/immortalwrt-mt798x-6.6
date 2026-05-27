REQUIRE_IMAGE_METADATA=1
RAMFS_COPY_BIN='fitblk blkid dmsetup'

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	mediatek,mt7988a-rfb)
		[ -e /dev/dm-0 ] && dmsetup remove_all
		[ -e /dev/fit0 ] && fitblk /dev/fit0
		[ -e /dev/fitrw ] && fitblk /dev/fitrw
		export_fitblk_bootdev
		case "$CI_METHOD" in
		emmc)
			mmc_do_upgrade "$1"
			;;
		default)
			default_do_upgrade "$1"
			;;
		ubi)
			CI_KERNPART="firmware"
			ubi_do_upgrade "$1"
			;;
		*)
			if grep \"rootfs_data\" /proc/mtd; then
				default_do_upgrade "$1"
			fi
			;;
		esac
		;;
	bananapi,bpi-r4|\
	bananapi,bpi-r4-poe|\
	bananapi,bpi-r4-pro|\
	mediatek,mt7988a-rfb|\
	tplink,tl-7dr7230-rev1.0-sp2|\
	tplink,tl-7dr7299-v1)
		fit_do_upgrade "$1"
		;;
	arcadyan,mozart|\
	smartrg,sdg-8733|\
	smartrg,sdg-8733a|\
	smartrg,sdg-8734)
		CI_KERNPART="kernel"
		CI_ROOTPART="rootfs"
		emmc_do_upgrade "$1"
		;;
	*)
		nand_do_upgrade "$1"
		;;
	esac
}

PART_NAME=firmware

platform_check_image() {
	local board=$(board_name)
	local magic="$(get_magic_long "$1")"

	[ "$#" -gt 1 ] && return 1

	case "$board" in
	mediatek,mt7988a-rfb|\
	bananapi,bpi-r4|\
	bananapi,bpi-r4-poe|\
	bananapi,bpi-r4-pro|\
	tplink,tl-7dr7230-rev1.0-sp2|\
	tplink,tl-7dr7299-v1)
		magic="$(dd if="$1" bs=1 skip=257 count=5 2>/dev/null)"

		[ "$magic" != "ustar" ] && {
			echo "Invalid image type."
			return 1
		}

		return 0
		;;

	*)
		nand_do_platform_check "$board" "$1"
		return $?
		;;
	esac

	return 0
}

platform_copy_config() {
	case "$(board_name)" in
	mediatek,mt7988a-rfb|\
	bananapi,bpi-r4|\
	bananapi,bpi-r4-poe|\
	bananapi,bpi-r4-pro)
		if [ "$CI_METHOD" = "emmc" ]; then
			emmc_copy_config
		fi
		;;
	arcadyan,mozart|\
	smartrg,sdg-8733|\
	smartrg,sdg-8733a|\
	smartrg,sdg-8734)
		emmc_copy_config
		;;
	esac
}

platform_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	*)
		;;
	esac
}
