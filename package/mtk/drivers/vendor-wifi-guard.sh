#!/bin/sh
# Pre-delete guard for mt_hwifi/mt_wifi7/mt_wifi/warp/conninfra package Make targets.
set -eu
test "$#" -ge 8 -a "$#" -le 9 || { echo 'vendor guard: expected 8 or 9 arguments' >&2; exit 2; }
top=$1
source_dir=$2
kernel_build=$3
package_build=$4
package_name=$5
use_source_dir=$6
use_git_tree=$7
use_git_src_checkout=$8
build_variant=${9:-}

case "$package_name" in
    mt_hwifi|mt_wifi7|mt_wifi|warp|conninfra) ;;
    *) echo 'vendor guard: unexpected package' >&2; exit 2 ;;
esac
test -z "$use_source_dir$use_git_tree$use_git_src_checkout" || {
    echo 'vendor guard: source override is forbidden' >&2
    exit 2
}
test ! -L "$package_build" || {
    echo 'vendor guard: package build directory is a symlink' >&2
    exit 2
}

top_phys=$(realpath -m -- "$top")
top_lex=$(realpath -m -s -- "$top")
src_phys=$(realpath -m -- "$source_dir")
kernel_phys=$(realpath -m -- "$kernel_build")
kernel_lex=$(realpath -m -s -- "$kernel_build")
build_phys=$(realpath -m -- "$package_build")
if [ -n "$build_variant" ]; then
    case "$build_variant" in
        -*|_*|*[!a-z0-9_-]*)
            echo 'vendor guard: invalid build_variant' >&2
            exit 2
            ;;
    esac
    case "$package_name" in
        mt_hwifi|mt_wifi7|conninfra)
            test ! -L "$kernel_build/${package_name}-${build_variant}" || {
                echo 'vendor guard: variant build directory is a symlink' >&2
                exit 2
            }
            expected_lex=$(realpath -m -s -- "$kernel_build/${package_name}-${build_variant}/$package_name")
            ;;
        mt_wifi|warp)
            expected_lex=$(realpath -m -s -- "$kernel_build/${package_name}-${build_variant}")
            ;;
    esac
else
    expected_lex=$(realpath -m -s -- "$kernel_build/$package_name")
fi
build_lex=$(realpath -m -s -- "$package_build")

# A parent symlink can redirect an otherwise expected package path to a
# different build tree. Reject symlinks below TOPDIR/build_dir before rm.
build_root_lex=$top_lex/build_dir
case "$kernel_lex/" in
    "$build_root_lex/"*) kernel_rel=${kernel_lex#"$build_root_lex"/} ;;
    *) echo 'vendor guard: kernel build directory is not lexically under workspace build_dir' >&2; exit 2 ;;
esac
test -n "$kernel_rel" || {
    echo 'vendor guard: kernel build directory is workspace build_dir itself' >&2
    exit 2
}
check_path=$build_root_lex
test ! -L "$check_path" || {
    echo 'vendor guard: kernel build directory path contains a symlink' >&2
    exit 2
}
while test -n "$kernel_rel"; do
    component=${kernel_rel%%/*}
    if test "$kernel_rel" = "$component"; then
        kernel_rel=
    else
        kernel_rel=${kernel_rel#*/}
    fi
    check_path=$check_path/$component
    test ! -L "$check_path" || {
        echo 'vendor guard: kernel build directory path contains a symlink' >&2
        exit 2
    }
done

test "$build_lex" = "$expected_lex" || {
    echo 'vendor guard: unexpected PKG_BUILD_DIR' >&2
    exit 2
}
case "$kernel_phys/" in
    "$top_phys/build_dir/"*) ;;
    *) echo 'vendor guard: kernel build directory escapes workspace build_dir' >&2; exit 2 ;;
esac
case "$build_phys/" in
    "$kernel_phys/"*) ;;
    *) echo 'vendor guard: package build directory escapes kernel build directory' >&2; exit 2 ;;
esac
case "$build_phys/" in
    "$src_phys/"*) echo 'vendor guard: build directory aliases source' >&2; exit 2 ;;
esac
case "$src_phys/" in
    "$build_phys/"*) echo 'vendor guard: source is inside build directory' >&2; exit 2 ;;
esac
