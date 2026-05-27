. /lib/functions/system.sh

preinit_set_mac_address() {
	case $(board_name) in
	*)
		;;
	esac
}

boot_hook_add preinit_main preinit_set_mac_address
