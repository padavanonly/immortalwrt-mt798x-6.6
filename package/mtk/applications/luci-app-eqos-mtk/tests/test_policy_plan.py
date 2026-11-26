#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
import argparse
import json
import os
import shutil
import subprocess
import tempfile
from pathlib import Path


def main():
    package = Path(__file__).resolve().parent.parent
    default_planner = package / "root/usr/libexec/eqos/policy-plan.uc"
    default_ucode = os.environ.get("UCODE") or shutil.which("ucode")

    parser = argparse.ArgumentParser(
        description="Exercise the inactive EQoS policy planner with real ucode; no services or network tools are called."
    )
    parser.add_argument("--planner", type=Path, default=default_planner,
                        help="planner path; defaults relative to package/tests")
    parser.add_argument("--ucode", type=Path,
                        default=Path(default_ucode) if default_ucode else None,
                        help="ucode executable; defaults to $UCODE or PATH")
    parser.add_argument("--results", type=Path,
                        help="new directory for case inputs/results; defaults to a temporary directory")
    args = parser.parse_args()
    if args.ucode is None:
        parser.error("set --ucode, $UCODE, or PATH to a real ucode executable")

    planner = args.planner.resolve()
    ucode = args.ucode.resolve()
    if not planner.is_file():
        raise SystemExit(f"planner not found: {planner}")
    if not ucode.is_file():
        raise SystemExit(f"ucode executable not found: {ucode}")

    results_temp = None
    if args.results:
        results = args.results.resolve()
        if results.exists():
            raise SystemExit(f"refusing to overwrite result directory: {results}")
        results.mkdir(parents=True)
    else:
        results_temp = tempfile.TemporaryDirectory(prefix="eqos-policy-plan-")
        results = Path(results_temp.name)
    (results / "inputs").mkdir()
    (results / "stdout").mkdir()
    (results / "stderr").mkdir()

    env = os.environ.copy()
    passed = 0

    def policy():
        return {
            "enabled": True,
            "ipv6_enabled": True,
            "shaper": {
                "download_mbit": 100,
                "upload_mbit": 20,
                "smartqos": True,
            },
            "wan_balance": {
                "enabled": False,
                "interfaces": [],
                "special_marks": [],
            },
        }

    def document(static=None, leases=None, registry=None, now=1000, grace=100):
        return {
            "schema": "eqos-planner-input/v1",
            "now_epoch": now,
            "tombstone_grace_seconds": grace,
            "snapshot": {
                "policy": policy(),
                "static_clients": [] if static is None else static,
                "active_leases": [] if leases is None else leases,
            },
            "prior_registry": registry,
        }

    def static(ip=None, mac=None, upload=True, download=True,
               upload_kbit=2048, download_kbit=10240, interfacename=None):
        row = {
            "upload": upload,
            "download": download,
            "upload_kbit": upload_kbit,
            "download_kbit": download_kbit,
        }
        if ip is not None:
            row["ipv4"] = ip
        if mac is not None:
            row["mac"] = mac
        if interfacename is not None:
            row["interfacename"] = interfacename
        return row

    def lease(mac, ip, expiry=5000):
        return {"mac": mac, "ipv4": ip, "expires_epoch": expiry}

    def entry(identity, slot, aliases=None, static_reserved=False,
              static_configured=False, lease_expiry=None, tombstone=None):
        return {
            "identity": identity,
            "slot": slot,
            "aliases": [] if aliases is None else aliases,
            "static_reserved": static_reserved,
            "static_configured": static_configured,
            "lease_expires_epoch": lease_expiry,
            "tombstone_until_epoch": tombstone,
        }

    def registry(entries, generation=1):
        return {
            "schema": "eqos-slot-registry/v1",
            "generation": generation,
            "assignments": entries,
        }

    def run(name, value):
        input_bytes = (json.dumps(value, sort_keys=True, indent=2) + "\n").encode()
        input_path = results / "inputs" / f"{name}.json"
        input_path.write_bytes(input_bytes)
        proc = subprocess.run(
            [str(ucode), str(planner), str(input_path)],
            env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False
        )
        (results / "stdout" / f"{name}.bin").write_bytes(proc.stdout)
        (results / "stderr" / f"{name}.bin").write_bytes(proc.stderr)
        (results / f"{name}.exit").write_text(f"{proc.returncode}\n")
        return proc

    def ready(name, value):
        nonlocal passed
        proc = run(name, value)
        assert proc.returncode == 0, (name, proc.returncode, proc.stderr.decode(errors="replace"))
        assert proc.stderr == b"", (name, proc.stderr.decode(errors="replace"))
        result = json.loads(proc.stdout)
        assert result.get("status") == "ready", (name, result)
        assert "plan" in result and "next_registry" in result, (name, result)
        passed += 1
        return result

    def reject(name, value, code):
        nonlocal passed
        proc = run(name, value)
        assert proc.returncode == 2, (name, proc.returncode, proc.stderr.decode(errors="replace"))
        assert proc.stderr == b"", (name, proc.stderr.decode(errors="replace"))
        result = json.loads(proc.stdout)
        assert result.get("status") == "error", (name, result)
        assert result.get("code") == code, (name, result)
        assert "plan" not in result and "next_registry" not in result, (name, result)
        passed += 1
        return result

    # Empty bootstrap has a complete deterministic plan and registry.
    empty = ready("empty-bootstrap", document())
    assert empty["next_registry"]["generation"] == 1
    assert empty["plan"]["static_clients"] == []
    assert empty["plan"]["lease_clients"] == []

    # A static IPv4-only row joins exactly one same-IP lease; static policy wins.
    joined = ready("static-ip-lease-join", document(
        static=[static(ip="192.0.2.10", upload=False, download=True,
                       upload_kbit=3072, download_kbit=8192)],
        leases=[lease("02:00:00:00:00:0a", "192.0.2.10")],
    ))
    assert joined["plan"]["static_clients"] == [{
        "identity": "mac:02:00:00:00:00:0a", "slot": 1,
        "upload": False, "download": True,
        "upload_kbit": 3072, "download_kbit": 8192,
        "mac": "02:00:00:00:00:0a", "ipv4": "192.0.2.10",
    }]
    assert joined["plan"]["lease_clients"][0]["slot"] == 1
    assert joined["plan"]["lease_clients"][0]["upload"] is False
    assert joined["next_registry"]["assignments"][0]["static_reserved"] is True
    assert joined["next_registry"]["assignments"][0]["aliases"] == ["ipv4:192.0.2.10"]

    # Multiple IPv4 leases for one MAC use one stable slot.
    multi_ip = ready("multiple-ip-one-mac", document(
        leases=[lease("02:00:00:00:00:22", "192.0.2.22"),
                lease("02:00:00:00:00:22", "192.0.2.23")],
    ))
    assert len(multi_ip["next_registry"]["assignments"]) == 1
    assert [x["slot"] for x in multi_ip["plan"]["lease_clients"]] == [1, 1]

    # dnsmasq expiry zero means a permanent active lease. It survives a
    # registry round-trip and remains a no-op when the same snapshot repeats.
    permanent_mac = "02:00:00:00:00:26"
    permanent_input = document(
        leases=[lease(permanent_mac, "192.0.2.26", expiry=0)],
    )
    permanent = ready("permanent-lease-single", permanent_input)
    permanent_entry = permanent["next_registry"]["assignments"][0]
    assert permanent_entry["identity"] == f"mac:{permanent_mac}"
    assert permanent_entry["lease_expires_epoch"] == 0
    assert permanent["plan"]["lease_clients"][0]["ipv4"] == "192.0.2.26"
    permanent_repeat_input = document(
        leases=[lease(permanent_mac, "192.0.2.26", expiry=0)],
        registry=permanent["next_registry"],
    )
    permanent_repeat = ready("permanent-lease-noop", permanent_repeat_input)
    assert permanent_repeat["registry_changed"] is False
    assert permanent_repeat["next_registry"] == permanent["next_registry"]

    # A permanent address makes the per-MAC registry expiry permanent even
    # when another address has a finite lease; row order cannot change it.
    mixed_mac = "02:00:00:00:00:27"
    permanent_then_finite = ready("permanent-zero-then-finite", document(
        leases=[lease(mixed_mac, "192.0.2.27", expiry=0),
                lease(mixed_mac, "192.0.2.28", expiry=5000)],
    ))
    finite_then_permanent = ready("permanent-finite-then-zero", document(
        leases=[lease(mixed_mac, "192.0.2.28", expiry=5000),
                lease(mixed_mac, "192.0.2.27", expiry=0)],
    ))
    assert permanent_then_finite == finite_then_permanent
    assert permanent_then_finite["next_registry"]["assignments"][0]["lease_expires_epoch"] == 0
    assert [row["ipv4"] for row in permanent_then_finite["plan"]["lease_clients"]] == [
        "192.0.2.27", "192.0.2.28"]

    # Removing a permanent lease creates a normal grace tombstone; re-entry
    # during that grace retains the slot and restores active expiry zero.
    permanent_removed = ready("permanent-lease-removal-tombstone", document(
        registry=permanent["next_registry"], now=1000, grace=120,
    ))
    permanent_tombstone = permanent_removed["next_registry"]["assignments"][0]
    assert permanent_tombstone["slot"] == permanent_entry["slot"]
    assert permanent_tombstone["lease_expires_epoch"] is None
    assert permanent_tombstone["tombstone_until_epoch"] == 1120
    permanent_readded = ready("permanent-lease-readd-within-grace", document(
        leases=[lease(permanent_mac, "192.0.2.26", expiry=0)],
        registry=permanent_removed["next_registry"], now=1050, grace=120,
    ))
    permanent_readded_entry = permanent_readded["next_registry"]["assignments"][0]
    assert permanent_readded_entry["slot"] == permanent_entry["slot"]
    assert permanent_readded_entry["lease_expires_epoch"] == 0
    assert permanent_readded_entry["tombstone_until_epoch"] is None

    # Dynamic lease addresses are current aliases, not permanent reservations.
    old_mac = "02:00:00:00:00:24"
    new_mac = "02:00:00:00:00:25"
    recycled = ready("dynamic-lease-ip-reused", document(
        leases=[lease(old_mac, "192.0.2.26"), lease(new_mac, "192.0.2.24")],
        registry=registry([entry(f"mac:{old_mac}", 8,
                                 aliases=["ipv4:192.0.2.24"],
                                 lease_expiry=5000)], generation=3),
    ))
    recycled_slots = {row["identity"]: row["slot"]
                      for row in recycled["next_registry"]["assignments"]}
    assert recycled_slots[f"mac:{old_mac}"] == 8
    assert recycled_slots[f"mac:{new_mac}"] == 1
    assert all(not row["aliases"]
               for row in recycled["next_registry"]["assignments"])

    # Sorting makes output independent of snapshot row order at bootstrap.
    order_a = document(
        static=[static(ip="192.0.2.31"), static(mac="02:00:00:00:00:30")],
        leases=[lease("02:00:00:00:00:32", "192.0.2.32"),
                lease("02:00:00:00:00:33", "192.0.2.33")],
    )
    order_b = document(
        static=list(reversed(order_a["snapshot"]["static_clients"])),
        leases=list(reversed(order_a["snapshot"]["active_leases"])),
    )
    sorted_a = ready("input-order-a", order_a)
    sorted_b = ready("input-order-b", order_b)
    assert sorted_a == sorted_b

    # Repeating the same snapshot with the returned registry is a no-op.
    stable_input = document(leases=[lease("02:00:00:00:00:41", "192.0.2.41")])
    first = ready("stable-first", stable_input)
    stable_input["prior_registry"] = first["next_registry"]
    second = ready("stable-second", stable_input)
    assert second["registry_changed"] is False
    assert second["next_registry"] == first["next_registry"]
    assert second["plan"] == first["plan"]
    assert second["decisions"] == []

    # Existing slot assignments survive event-order changes; a new static
    # identity receives another free slot instead of forcing a live lease move.
    previous = registry([
        entry("mac:02:00:00:00:00:50", 7, lease_expiry=5000),
    ], generation=8)
    stable_slots = ready("preserve-existing-slots", document(
        static=[static(ip="192.0.2.51", mac="02:00:00:00:00:51")],
        leases=[lease("02:00:00:00:00:50", "192.0.2.50")],
        registry=previous,
    ))
    by_identity = {row["identity"]: row["slot"]
                   for row in stable_slots["next_registry"]["assignments"]}
    assert by_identity["mac:02:00:00:00:00:50"] == 7
    assert by_identity["mac:02:00:00:00:00:51"] == 1

    # A same-identity dynamic assignment becomes static without moving.
    promote = ready("promote-lease-to-static", document(
        static=[static(ip="192.0.2.60", mac="02:00:00:00:00:60")],
        leases=[lease("02:00:00:00:00:60", "192.0.2.60")],
        registry=registry([entry("mac:02:00:00:00:00:60", 9,
                                 lease_expiry=5000)], generation=4),
    ))
    assert promote["next_registry"]["assignments"][0]["slot"] == 9
    assert promote["next_registry"]["assignments"][0]["static_reserved"] is True
    assert promote["next_registry"]["generation"] == 5

    # Removed static configuration keeps a visible reservation pending a
    # future reviewed release mechanism; this planner never frees it.
    removed_static = ready("removed-static-reservation", document(
        registry=registry([entry("mac:02:00:00:00:00:70", 12,
                                 static_reserved=True, static_configured=True)], generation=2),
    ))
    retained = removed_static["next_registry"]["assignments"][0]
    assert retained["slot"] == 12 and retained["static_reserved"] is True
    assert retained["static_configured"] is False
    assert retained["tombstone_until_epoch"] is None

    # Lease removal creates one explicit tombstone; re-add within grace keeps it.
    removed_lease = ready("lease-removal-tombstone", document(
        registry=registry([entry("mac:02:00:00:00:00:80", 14,
                                 lease_expiry=5000)], generation=3),
        now=1000, grace=120,
    ))
    tombstone = removed_lease["next_registry"]["assignments"][0]
    assert tombstone["slot"] == 14
    assert tombstone["lease_expires_epoch"] is None
    assert tombstone["tombstone_until_epoch"] == 1120
    readded = ready("lease-readd-within-grace", document(
        leases=[lease("02:00:00:00:00:80", "192.0.2.80")],
        registry=removed_lease["next_registry"], now=1050, grace=120,
    ))
    assert readded["next_registry"]["assignments"][0]["slot"] == 14
    assert readded["next_registry"]["assignments"][0]["tombstone_until_epoch"] is None

    # At the exact tombstone deadline the old dynamic slot is reclaimable.
    expired = ready("lease-tombstone-expired", document(
        leases=[lease("02:00:00:00:00:81", "192.0.2.81")],
        registry=registry([entry("mac:02:00:00:00:00:80", 14,
                                 tombstone=1100)], generation=5),
        now=1100, grace=120,
    ))
    assert expired["next_registry"]["assignments"][0]["identity"] == "mac:02:00:00:00:00:81"
    assert expired["next_registry"]["assignments"][0]["slot"] == 1

    # PPP interfaces can have no via gateway or destination subnet; route IDs remain stable.
    marked = document()
    marked["snapshot"]["policy"]["wan_balance"] = {
        "enabled": True,
        "interfaces": [{"index": 1, "ifname": "pppoe-wan", "gateway": "",
                         "subnet": "", "mark": "21", "table": "210"}],
        "special_marks": [{"protocol": "tcp", "destination_port": 443, "mark": "21"}],
    }
    passthrough = ready("preserve-route-identifiers", marked)
    assert passthrough["plan"]["wan_balance"]["interfaces"][0]["mark"] == "21"
    assert passthrough["plan"]["wan_balance"]["interfaces"][0]["table"] == "210"
    assert passthrough["plan"]["wan_balance"]["interfaces"][0]["subnet"] == ""
    stale_special_mark = json.loads(json.dumps(marked))
    stale_special_mark["snapshot"]["policy"]["wan_balance"]["special_marks"][0]["mark"] = "20"
    reject("reject-special-mark-without-active-wan", stale_special_mark, "invalid_input")

    # Identity conflicts reject the entire plan and produce no partial state.
    reject("duplicate-ip-different-macs", document(
        leases=[lease("02:00:00:00:00:90", "192.0.2.90"),
                lease("02:00:00:00:00:91", "192.0.2.90")],
    ), "identity_conflict")
    reject("alias-mac-changed", document(
        leases=[lease("02:00:00:00:00:a2", "192.0.2.92")],
        registry=registry([entry("mac:02:00:00:00:00:a1", 5,
                                 aliases=["ipv4:192.0.2.92"],
                                 static_reserved=True)], generation=1),
    ), "alias_mac_changed")
    reject("duplicate-static-policy", document(
        static=[static(ip="192.0.2.93", mac="02:00:00:00:00:93", upload=True),
                static(ip="192.0.2.94", mac="02:00:00:00:00:93", upload=False)],
    ), "identity_conflict")
    reject("duplicate-registry-slot", document(
        registry=registry([
            entry("mac:02:00:00:00:00:b1", 4, lease_expiry=5000),
            entry("mac:02:00:00:00:00:b2", 4, lease_expiry=5000),
        ]),
    ), "invalid_registry")
    reject("multiple-macs-in-registry-assignment", document(
        registry=registry([entry(
            "mac:02:00:00:00:00:b3", 4,
            aliases=["mac:02:00:00:00:00:b4"],
            static_reserved=True, static_configured=True,
        )]),
    ), "invalid_registry")

    # The allocator either returns 31 unique slots or an explicit capacity error.
    def numbered_leases(count, expiry=5000):
        return [lease(f"02:00:00:00:01:{number:02x}",
                      f"198.51.100.{number}", expiry=expiry)
                for number in range(1, count + 1)]

    def numbered_static(count):
        return [static(ip=f"203.0.113.{number}",
                       mac=f"02:00:00:00:02:{number:02x}")
                for number in range(1, count + 1)]

    full = ready("capacity-31", document(leases=numbered_leases(31)))
    assert len(full["next_registry"]["assignments"]) == 31
    assert len({row["slot"] for row in full["next_registry"]["assignments"]}) == 31
    capacity = reject("capacity-32", document(leases=numbered_leases(32)), "slot_capacity")
    assert capacity["limit"] == 31
    assert capacity["required"] == 32
    assert capacity["available"] == 31
    reject("expired-positive-lease", document(
        leases=[lease("02:00:00:00:00:a3", "192.0.2.163", expiry=1000)],
        now=1000,
    ), "invalid_input")

    # Disabling Smart QoS still allows an exact lease to enrich a static IP,
    # but ordinary lease-only clients do not consume classifier slots.
    no_smartqos_static = document(
        static=[static(ip="192.0.2.140")],
        leases=[lease("02:00:00:00:00:8c", "192.0.2.140")],
    )
    no_smartqos_static["snapshot"]["policy"]["shaper"]["smartqos"] = False
    enriched_static = ready("smartqos-off-static-enrichment", no_smartqos_static)
    assert enriched_static["plan"]["static_clients"][0]["mac"] == "02:00:00:00:00:8c"
    assert enriched_static["plan"]["lease_clients"] == []
    no_smartqos_leases = document(leases=numbered_leases(32))
    no_smartqos_leases["snapshot"]["policy"]["shaper"]["smartqos"] = False
    ignored_dynamic = ready("smartqos-off-32-leases", no_smartqos_leases)
    assert ignored_dynamic["plan"]["lease_clients"] == []
    assert ignored_dynamic["next_registry"]["assignments"] == []
    no_smartqos_permanent = document(leases=numbered_leases(32, expiry=0))
    no_smartqos_permanent["snapshot"]["policy"]["shaper"]["smartqos"] = False
    ignored_permanent = ready("smartqos-off-32-permanent-leases", no_smartqos_permanent)
    assert ignored_permanent["plan"]["lease_clients"] == []
    assert ignored_permanent["next_registry"]["assignments"] == []
    no_smartqos_permanent_static = document(
        static=[static(ip="192.0.2.141")],
        leases=[lease("02:00:00:00:00:8d", "192.0.2.141", expiry=0)],
    )
    no_smartqos_permanent_static["snapshot"]["policy"]["shaper"]["smartqos"] = False
    permanent_enrichment = ready("smartqos-off-permanent-static-enrichment",
                                 no_smartqos_permanent_static)
    assert permanent_enrichment["plan"]["static_clients"][0]["mac"] == "02:00:00:00:00:8d"
    assert permanent_enrichment["plan"]["lease_clients"] == []
    assert permanent_enrichment["next_registry"]["assignments"][0]["static_reserved"] is True

    # Overall disable emits an empty disabled plan and preserves the registry
    # without allocating for a large live lease snapshot.
    disabled = document(
        static=numbered_static(32),
        leases=numbered_leases(32),
        registry=registry([
            entry("mac:02:00:00:00:00:c1", 6, static_reserved=True,
                  static_configured=True),
            entry("mac:02:00:00:00:00:c2", 7, lease_expiry=5000),
        ], generation=9),
    )
    disabled["snapshot"]["policy"]["enabled"] = False
    disabled_result = ready("overall-disabled-32-leases", disabled)
    assert disabled_result["plan"]["enabled"] is False
    assert disabled_result["plan"]["static_clients"] == []
    assert disabled_result["plan"]["lease_clients"] == []
    assert disabled_result["registry_changed"] is False
    assert disabled_result["next_registry"]["generation"] == 9
    assert len(disabled_result["next_registry"]["assignments"]) == 2
    disabled_permanent = document(
        static=numbered_static(32),
        leases=numbered_leases(32, expiry=0),
        registry=disabled["prior_registry"],
    )
    disabled_permanent["snapshot"]["policy"]["enabled"] = False
    disabled_permanent_result = ready("overall-disabled-32-permanent-leases", disabled_permanent)
    assert disabled_permanent_result["plan"]["enabled"] is False
    assert disabled_permanent_result["plan"]["lease_clients"] == []
    assert disabled_permanent_result["registry_changed"] is False
    assert disabled_permanent_result["next_registry"] == disabled["prior_registry"]

    missing_clock = document()
    del missing_clock["now_epoch"]
    reject("clock-required", missing_clock, "invalid_input")
    print(f"PASS: {passed} actual-uCode policy-planner cases; planner={planner}; ucode={ucode}; results={results}")


if __name__ == "__main__":
    main()
