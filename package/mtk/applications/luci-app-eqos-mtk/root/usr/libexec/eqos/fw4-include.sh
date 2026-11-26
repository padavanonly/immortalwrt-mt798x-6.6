#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only

exec ${EQOS_UCODE_BIN:-/usr/bin/ucode} ${EQOS_UCODE_ARGS} "${EQOS_LIBEXEC_DIR:-/usr/libexec/eqos}/reconcile.uc" --trigger fw4
