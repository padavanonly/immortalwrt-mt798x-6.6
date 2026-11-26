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

    DEFAULT_RENDERER = Path(__file__).resolve().parent.parent / "root/usr/libexec/eqos/nft-render.uc"
    DEFAULT_UCODE = os.environ.get("UCODE") or shutil.which("ucode")
    parser = argparse.ArgumentParser(
        description="Exercise an EQoS nft renderer with real ucode; does not invoke nft."
    )
    parser.add_argument("--renderer", type=Path, default=DEFAULT_RENDERER,
                        help="renderer path; defaults relative to package/tests")
    parser.add_argument("--ucode", type=Path,
                        default=Path(DEFAULT_UCODE) if DEFAULT_UCODE else None,
                        help="ucode executable; defaults to $UCODE or PATH")
    parser.add_argument("--results", type=Path,
                        help="new directory for raw case inputs, stdout, stderr, and exit codes; defaults to a temporary directory")
    args = parser.parse_args()
    if args.ucode is None:
        parser.error("set --ucode, $UCODE, or PATH to a real ucode executable")
    CANDIDATE = args.renderer.resolve()
    UCODE = args.ucode.resolve()
    RESULTS_TEMP = None
    if args.results:
        RESULTS = args.results.resolve()
        if RESULTS.exists():
            raise SystemExit(f"refusing to overwrite existing result directory: {RESULTS}")
        RESULTS.mkdir(parents=True)
    else:
        RESULTS_TEMP = tempfile.TemporaryDirectory(prefix="eqos-nft-render-")
        RESULTS = Path(RESULTS_TEMP.name)


    def document(mode="install", static=None, leases=None, bypass=None, **extra):
        value = {
            "schema": "eqos-nft/v1",
            "mode": mode,
            "static_clients": [] if static is None else static,
            "lease_clients": [] if leases is None else leases,
            "bypass_v4": [] if bypass is None else bypass,
        }
        value.update(extra)
        return value


    def client(slot=1, ip=None, mac=None, upload=True, download=True, **extra):
        value = {"slot": slot, "upload": upload, "download": download}
        if ip is not None:
            value["ipv4"] = ip
        if mac is not None:
            value["mac"] = mac
        value.update(extra)
        return value


    if not CANDIDATE.is_file():
        raise SystemExit(f"renderer not found: {CANDIDATE}")
    if not UCODE.is_file():
        raise SystemExit(f"ucode executable not found: {UCODE}")
    (RESULTS / "inputs").mkdir(parents=True)
    (RESULTS / "stdout").mkdir()
    (RESULTS / "stderr").mkdir()

    # Preserve UCODE_PATH, LD_LIBRARY_PATH, and all other caller environment.
    env = os.environ.copy()
    passed = [0]


    def run(name, value=None, raw=None):
        if raw is None:
            raw = (json.dumps(value, sort_keys=True, indent=2) + "\n").encode()
        input_path = RESULTS / "inputs" / f"{name}.json"
        input_path.write_bytes(raw)
        proc = subprocess.run(
            [str(UCODE), str(CANDIDATE), str(input_path)],
            env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False
        )
        (RESULTS / "stdout" / f"{name}.bin").write_bytes(proc.stdout)
        (RESULTS / "stderr" / f"{name}.txt").write_bytes(proc.stderr)
        (RESULTS / f"{name}.exit").write_text(f"{proc.returncode}\n")
        return proc


    def ok(name, value=None, raw=None):
        proc = run(name, value, raw)
        assert proc.returncode == 0, (name, proc.returncode, proc.stderr.decode(errors="replace"))
        assert proc.stderr == b"", (name, proc.stderr.decode(errors="replace"))
        output = proc.stdout.decode()
        forbid_output(output)
        passed[0] += 1
        return output


    def reject(name, value=None, raw=None, contains=None):
        proc = run(name, value, raw)
        err = proc.stderr.decode(errors="replace")
        assert proc.returncode != 0, (name, "unexpected success")
        assert proc.stdout == b"", (name, f"partial stdout: {proc.stdout[:300]!r}")
        assert "eqos-nft-render:" in err, (name, f"not a renderer validation error: {err}")
        if contains:
            assert contains in err, (name, f"missing {contains!r} in {err!r}")
        passed[0] += 1
        return err


    def has_lines(output, *expected):
        lines = output.splitlines()
        for line in expected:
            assert line in lines, f"missing output line: {line}"


    def forbid_output(output):
        lower = output.lower()
        for forbidden in (
            "hook ", "priority ", "flush ruleset", "flush table",
            "iptables", "ip6tables", "ebtables", "/etc/firewall.user",
            "tc ", "ct mark", "wan"
        ):
            assert forbidden not in lower, f"unexpected output token: {forbidden}"
        for line in output.splitlines():
            if line.startswith("#"):
                continue
            assert line.startswith(("add table ", "add chain ", "add map ",
                                    "add element ", "add rule ", "flush map ")), line


    # Self-contained equivalents of the author install and lease examples.
    example_static = [client(slot=3, ip="192.0.2.3", mac="02:00:00:00:00:03",
                              upload=False, download=True)]
    example_leases_data = [
        client(slot=3, ip="192.0.2.3", mac="02:00:00:00:00:03",
               upload=True, download=True),
        client(slot=4, ip="192.0.2.4", mac="02:00:00:00:00:04",
               upload=True, download=False),
    ]
    example_bypass = [{"ipv4": "192.0.2.99", "upload": True, "download": False}]
    example_install = ok("author-example-install", document(
        static=example_static, leases=example_leases_data, bypass=example_bypass
    ))
    example_leases = ok("author-example-leases", document(
        mode="leases", static=example_static, leases=example_leases_data,
        bypass=example_bypass
    ))
    assert "meta mark set 0x99 return" in example_install
    assert "flush map inet eqos_candidate lease_v4_upload" in example_leases
    assert "add rule " not in example_leases
    forbid_output(example_install)
    forbid_output(example_leases)

    # Single-family identities, four directions, and the 1..31 / paired 33..63 bounds.
    ip_only = ok("positive-ip-only", document(
        static=[client(slot=1, ip="192.0.2.10", upload=True, download=False)]
    ))
    has_lines(ip_only,
        "add rule inet eqos_candidate ipv4_upload ip saddr 192.0.2.10 ip dscp set 1 return",
        "add rule inet eqos_candidate ipv4_download ip daddr 192.0.2.10 return")
    mac_only = ok("positive-mac-only", document(
        static=[client(slot=31, mac="02:00:00:00:00:1F", upload=False, download=True)]
    ))
    has_lines(mac_only,
        "add rule inet eqos_candidate ipv6_upload meta nfproto ipv6 ether saddr 02:00:00:00:00:1f return",
        "add rule bridge eqos_candidate ipv6_download ether type ip6 ether daddr 02:00:00:00:00:1f meta mark set 63 return")
    all_four = ok("positive-all-four-directions", document(
        static=[client(slot=7, ip="192.0.2.44", mac="02:00:00:00:00:44")]
    ))
    has_lines(all_four,
        "add rule inet eqos_candidate ipv4_upload ip saddr 192.0.2.44 ip dscp set 7 return",
        "add rule inet eqos_candidate ipv4_download ip daddr 192.0.2.44 ip dscp set 39 return",
        "add rule inet eqos_candidate ipv6_upload meta nfproto ipv6 ether saddr 02:00:00:00:00:44 meta mark set 7 return",
        "add rule bridge eqos_candidate ipv6_download ether type ip6 ether daddr 02:00:00:00:00:44 meta mark set 39 return")
    slot31_maps = ok("positive-map-slot31", document(
        leases=[client(slot=31, ip="255.255.255.255", mac="02:00:00:00:00:ff")]
    ))
    has_lines(slot31_maps,
        "add chain inet eqos_candidate dscp_slot_31",
        "add rule inet eqos_candidate dscp_slot_31 ip dscp set 31",
        "add chain inet eqos_candidate dscp_slot_63",
        "add rule inet eqos_candidate dscp_slot_63 ip dscp set 63",
        "add map inet eqos_candidate lease_v4_upload { type ipv4_addr : verdict; }",
        "add map inet eqos_candidate lease_v4_download { type ipv4_addr : verdict; }",
        "add map inet eqos_candidate lease_v6_upload { type ether_addr : mark; }",
        "add map bridge eqos_candidate lease_v6_download { type ether_addr : mark; }",
        "add element inet eqos_candidate lease_v4_upload { 255.255.255.255 : jump dscp_slot_31 }",
        "add element inet eqos_candidate lease_v4_download { 255.255.255.255 : jump dscp_slot_63 }",
        "add element inet eqos_candidate lease_v6_upload { 02:00:00:00:00:ff : 31 }",
        "add element bridge eqos_candidate lease_v6_download { 02:00:00:00:00:ff : 63 }")
    forbid_output(slot31_maps)

    # 0x99 bypass; only selected directions emit the reserved mark.
    bypass = ok("positive-bypass-0x99", document(
        bypass=[{"ipv4": "203.0.113.99", "upload": True, "download": False}]
    ))
    has_lines(bypass,
        "add rule inet eqos_candidate ipv4_upload ip saddr 203.0.113.99 meta mark set 0x99 return")
    assert not any(
        "ipv4_download ip daddr 203.0.113.99 meta mark set 0x99" in line
        for line in bypass.splitlines()
    )

    # Exact duplicates are emitted once; different identities may not share an IP/slot.
    dup_client = client(slot=2, ip="198.51.100.2")
    dup_lease = client(slot=3, ip="198.51.100.3")
    dedup = ok("dedupe-clients-and-bypass", document(
        static=[dup_client, dup_client],
        leases=[dup_lease, dup_lease],
        bypass=[{"ipv4": "203.0.113.8", "upload": True, "download": False}] * 2
    ))
    assert dedup.splitlines().count(
        "add rule inet eqos_candidate ipv4_upload ip saddr 198.51.100.2 ip dscp set 2 return"
    ) == 1
    assert dedup.splitlines().count(
        "add element inet eqos_candidate lease_v4_upload { 198.51.100.3 : jump dscp_slot_3 }"
    ) == 1
    assert dedup.splitlines().count(
        "add rule inet eqos_candidate ipv4_upload ip saddr 203.0.113.8 meta mark set 0x99 return"
    ) == 1

    # Static disabled directions return before the dynamic lookup for the same identity.
    disabled_static = client(slot=1, ip="192.0.2.60", mac="02:00:00:00:00:60",
                             upload=False, download=True)
    enabled_lease = client(slot=1, ip="192.0.2.60", mac="02:00:00:00:00:60",
                           upload=True, download=False)
    guard = ok("static-disabled-direction-guard", document(
        static=[disabled_static], leases=[enabled_lease]
    ))
    has_lines(guard,
        "add rule inet eqos_candidate ipv4_upload ip saddr 192.0.2.60 return",
        "add rule inet eqos_candidate ipv4_download ip daddr 192.0.2.60 ip dscp set 33 return",
        "add rule inet eqos_candidate ipv6_upload meta nfproto ipv6 ether saddr 02:00:00:00:00:60 return",
        "add rule bridge eqos_candidate ipv6_download ether type ip6 ether daddr 02:00:00:00:00:60 meta mark set 33 return",
        "add element inet eqos_candidate lease_v4_upload { 192.0.2.60 : jump dscp_slot_1 }",
        "add element inet eqos_candidate lease_v6_upload { 02:00:00:00:00:60 : 1 }")
    assert guard.index("ip saddr 192.0.2.60 return") < guard.index(
        "ip saddr vmap @lease_v4_upload"
    )
    assert guard.index("ether saddr 02:00:00:00:00:60 return") < guard.index(
        "meta mark set ether saddr map @lease_v6_upload"
    )
    assert "add element inet eqos_candidate lease_v4_download { 192.0.2.60" not in guard
    assert "add element bridge eqos_candidate lease_v6_download { 02:00:00:00:00:60" not in guard

    # IPv4-only static identity can share a slot with its MAC+IPv4 lease without
    # adding the learned MAC to the static rules. Dynamic bridge identity remains.
    partial_ip = "192.0.2.8"
    partial_mac = "02:00:00:00:00:08"
    partial_static = client(slot=8, ip=partial_ip, upload=False, download=True)
    partial_lease = client(slot=8, ip=partial_ip, mac=partial_mac)
    partial = ok("positive-partial-static-lease-identity", document(
        static=[partial_static], leases=[partial_lease]
    ))
    has_lines(partial,
        f"add rule inet eqos_candidate ipv4_upload ip saddr {partial_ip} return",
        f"add rule inet eqos_candidate ipv4_download ip daddr {partial_ip} ip dscp set 40 return",
        f"add element inet eqos_candidate lease_v4_upload {{ {partial_ip} : jump dscp_slot_8 }}",
        f"add element inet eqos_candidate lease_v4_download {{ {partial_ip} : jump dscp_slot_40 }}",
        f"add element inet eqos_candidate lease_v6_upload {{ {partial_mac} : 8 }}",
        f"add element bridge eqos_candidate lease_v6_download {{ {partial_mac} : 40 }}")
    assert not any(line.startswith("add rule ") and partial_mac in line
                   for line in partial.splitlines())

    # Multiple active IPv4 addresses can share one MAC's slot.
    multi_ip = ok("positive-same-mac-multiple-ipv4", document(leases=[
        client(slot=8, ip="192.0.2.8", mac=partial_mac),
        client(slot=8, ip="192.0.2.9", mac=partial_mac),
    ]))
    has_lines(multi_ip,
        "add element inet eqos_candidate lease_v4_upload { 192.0.2.8 : jump dscp_slot_8, 192.0.2.9 : jump dscp_slot_8 }",
        "add element inet eqos_candidate lease_v4_download { 192.0.2.8 : jump dscp_slot_40, 192.0.2.9 : jump dscp_slot_40 }",
        f"add element inet eqos_candidate lease_v6_upload {{ {partial_mac} : 8 }}",
        f"add element bridge eqos_candidate lease_v6_download {{ {partial_mac} : 40 }}")

    # Per-slot identity validation is independent of record order.
    order_ip_client = client(slot=8, ip=partial_ip)
    partial_mac_client = client(slot=8, ip=partial_ip, mac=partial_mac)
    order_a = ok("positive-partial-identity-order-a", document(
        mode="leases", static=[order_ip_client, partial_mac_client]
    ))
    order_b = ok("positive-partial-identity-order-b", document(
        mode="leases", static=[partial_mac_client, order_ip_client]
    ))
    assert order_a == order_b

    # Lease-only update contains no static rule/table/chain operation and ignores static input.
    lease_set = client(slot=4, ip="198.51.100.40", mac="02:00:00:00:00:40")
    lease_a = ok("lease-only-static-a", document(
        mode="leases", static=[client(slot=1, ip="192.0.2.71")], leases=[lease_set]
    ))
    lease_b = ok("lease-only-static-b", document(
        mode="leases", static=[client(slot=1, ip="192.0.2.72")], leases=[lease_set]
    ))
    assert lease_a == lease_b
    assert "192.0.2.71" not in lease_a and "192.0.2.72" not in lease_a
    assert "add rule " not in lease_a and "add chain " not in lease_a and "add table " not in lease_a
    assert all(line.startswith("#") or line.startswith(("flush map ", "add element "))
               for line in lease_a.splitlines())

    # Changing only the lease set leaves static rule lines byte-for-byte identical.
    static_fixed = client(slot=5, ip="192.0.2.80", mac="02:00:00:00:00:80")
    install_a = ok("install-lease-revision-a", document(
        static=[static_fixed], leases=[client(slot=6, ip="198.51.100.81")]
    ))
    install_b = ok("install-lease-revision-b", document(
        static=[static_fixed], leases=[client(slot=6, ip="198.51.100.82")]
    ))
    static_a = [line for line in install_a.splitlines()
                if "192.0.2.80" in line or "02:00:00:00:00:80" in line]
    static_b = [line for line in install_b.splitlines()
                if "192.0.2.80" in line or "02:00:00:00:00:80" in line]
    assert static_a == static_b and len(static_a) == 4

    # Rejected inputs must never produce partial stdout.
    for slot in (0, 32, 1.5, "1", True):
        reject(f"reject-slot-{str(slot).replace('.', '_')}",
               document(static=[client(slot=slot, ip="192.0.2.1")]),
               contains="expected integer 1..31")
    reject("reject-upload-type",
           document(static=[client(ip="192.0.2.1", upload="true")]),
           contains="JSON booleans")
    reject("reject-ip-overflow",
           document(static=[client(ip="256.0.0.1")]),
           contains="canonical dotted-decimal IPv4")
    reject("reject-ip-leading-zero",
           document(static=[client(ip="192.0.2.01")]),
           contains="canonical dotted-decimal IPv4")
    reject("reject-ip-nul-suffix",
           document(static=[client(ip="192.0.2.1\u0000x")]),
           contains="canonical dotted-decimal IPv4")
    reject("reject-mac-nul-suffix",
           document(static=[client(mac="02:00:00:00:00:01\u0000x")]),
           contains="six colon-separated")
    reject("reject-bad-mac",
           document(static=[client(mac="02:00:00:00:00:gg")]),
           contains="six colon-separated")
    reject("reject-no-identity",
           document(static=[client()]),
           contains="at least one of mac or ipv4")
    reject("reject-unknown-wan-key",
           document(wan={"nth": 1}),
           contains="unsupported key 'wan'")
    reject("reject-unknown-conntrack-key",
           document(ctmark=1),
           contains="unsupported key 'ctmark'")
    reject("reject-malformed-json", raw=b'{"schema": ')
    reject("reject-missing-required-key",
           {"schema": "eqos-nft/v1", "mode": "install",
            "static_clients": [], "lease_clients": []},
           contains="missing required key 'bypass_v4'")
    reject("reject-static-flags-conflict",
           document(static=[
               client(slot=8, ip="192.0.2.8", upload=True, download=False),
               client(slot=8, ip="192.0.2.8", upload=False, download=False),
           ]),
           contains="static IPv4 directions")
    reject("reject-lease-flags-conflict",
           document(leases=[
               client(slot=8, ip="192.0.2.8", upload=True, download=False),
               client(slot=8, ip="192.0.2.8", upload=False, download=False),
           ]),
           contains="lease IPv4 directions")
    reject("reject-slot-disconnected-ipv4",
           document(static=[
               client(slot=8, ip="192.0.2.8", mac="02:00:00:00:00:08"),
               client(slot=8, ip="192.0.2.9"),
           ]),
           contains="disconnected IPv4")
    reject("reject-slot-mac-only-disconnected-ipv4",
           document(static=[
               client(slot=8, mac="02:00:00:00:00:08"),
               client(slot=8, ip="192.0.2.8"),
           ]),
           contains="disconnected IPv4")
    reject("reject-slot-unrelated-macless-ipv4",
           document(static=[
               client(slot=8, ip="192.0.2.8"),
               client(slot=8, ip="192.0.2.9"),
           ]),
           contains="multiple IPv4 addresses without a MAC")
    reject("reject-slot-conflicting-mac",
           document(static=[
               client(slot=8, ip="192.0.2.8", mac="02:00:00:00:00:08"),
               client(slot=8, ip="192.0.2.8", mac="02:00:00:00:00:09"),
           ]),
           contains="slot identity MAC")
    reject("reject-mac-two-slots",
           document(static=[
               client(slot=8, ip="192.0.2.8", mac="02:00:00:00:00:08"),
               client(slot=9, ip="192.0.2.9", mac="02:00:00:00:00:08"),
           ]),
           contains="MAC to slot")
    reject("reject-static-lease-ip-slot-conflict",
           document(static=[client(slot=8, ip="192.0.2.8")],
                    leases=[client(slot=9, ip="192.0.2.8")]),
           contains="IPv4 to slot")

    print(f"PASS: {passed[0]} actual-uCode candidate cases")
    print(f"renderer={CANDIDATE}")
    print(f"ucode={UCODE}")
    print(f"results={RESULTS}")

if __name__ == "__main__":
    main()
