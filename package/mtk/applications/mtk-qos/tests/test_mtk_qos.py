#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only

import json
import os
import subprocess
import tempfile
import unittest
from pathlib import Path

PKG_DIR = Path(__file__).resolve().parent.parent
SCRIPT = PKG_DIR / "files" / "mtk-qos"
RENDER_UC = PKG_DIR / "files" / "render-nft.uc"


class FakeSysEnv:
    def __init__(self, tmp: Path):
        self.root = tmp
        self.bin_dir = tmp / "bin"
        self.uci_dir = tmp / "etc_config"
        self.qdma_dir = tmp / "qdma"
        self.hnat_dir = tmp / "hnat"
        self.pce_dir = tmp / "pce"
        self.ppe_status = tmp / "mtk_flow_multitier"
        self.dt_compat = tmp / "dt_compatible"
        self.sys_net = tmp / "sys_net"
        self.sys_mod = tmp / "sys_mod"
        self.run_dir = tmp / "run"
        self.leases = tmp / "dhcp.leases"
        self.arp = tmp / "arp"
        self.logs = tmp / "logs"
        self.queue_reads = self.logs / "queue_reads.log"

        for d in [
            self.bin_dir,
            self.uci_dir,
            self.qdma_dir,
            self.hnat_dir,
            self.pce_dir,
            self.sys_net,
            self.sys_mod,
            self.run_dir,
            self.logs,
        ]:
            d.mkdir(parents=True, exist_ok=True)

        self.leases.write_text("1700000000 aa:bb:cc:dd:ee:02 192.168.59.66 nas *\n")
        self.arp.write_text(
            "IP address       HW type     Flags       HW address            Mask     Device\n"
            "192.168.59.88    0x1         0x2         aa:bb:cc:dd:ee:03     *        br-lan\n"
        )
        (self.sys_net / "br-lan").mkdir(exist_ok=True)
        (self.sys_net / "eth0").mkdir(exist_ok=True)
        (self.sys_net / "eth1").mkdir(exist_ok=True)
        (self.sys_net / "wwan0_1").mkdir(exist_ok=True)
        (self.sys_mod / "nft_connlimit").mkdir(exist_ok=True)
        (self.sys_mod / "mtk_wed").mkdir(exist_ok=True)

        for sch in (2, 3):
            (self.qdma_dir / f"qdma_sch{sch}").write_text(
                "EN\tScheduling\tMAX\tQueue#\n0\tWRR\t\t0\t\n"
            )
        for q in range(64):
            (self.qdma_dir / f"qdma_txq{q}").write_text(
                "scheduler: 0\npacket count: 0\npacket drop: 0\n"
            )
        for fname, content in {
            "hook_toggle": "enabled\n",
            "qos_toggle": "0\n",
            "sqm_toggle": "enabled=0 dpi=1 ul_eqos=1..15 ul_sqm=16..31 dl_eqos=33..47 dl_sqm=48..63 qos_toggle=0 ul=0 dl=0\n",
            "qos_dpi": "enabled=1 realtime_pkts=10 fair_pkts=20 video_pkts=5 p2p_pkts=2 eqos_flows=1\n",
            "qos_ack_prio": "1\n",
            "hnat_setting": "11 30\n",
            "all_entry": "state=BIND\nstate=BIND\nstate=UNBIND\n",
        }.items():
            (self.hnat_dir / fname).write_text(content)
        self.ppe_status.write_text("tier1_ppe_bind 2\ntier1_ppe_total 4096\n")
        (self.pce_dir / "cls_entry").write_text("")

        self.uci_kv = {
            "mtk_qos.global.enabled": "1",
            "mtk_qos.global.sqm_enabled": "1",
            "mtk_qos.global.eqos_enabled": "1",
            "mtk_qos.global.dpi_enabled": "1",
            "mtk_qos.global.pce_enabled": "1",
            "turboacc.config.fastpath_mh_eth_hnat_bind_rate": "5",
            "mtk_qos.global.wan_mode": "auto",
            "mtk_qos.global.interface": "auto",
            "mtk_qos.global.download_kbps": "300000",
            "mtk_qos.global.upload_kbps": "50000",
            "mtk_qos.global.scheduler": "wrr",
            "mtk_qos.global.overhead": "8",
            "mtk_qos.global.realtime_pct": "25",
            "mtk_qos.global.video_pct": "45",
            "mtk_qos.global.p2p_pct": "15",
            "mtk_qos.global.ack_prio": "1",
            "mtk_qos.global.cake_diffserv": "diffserv4",
            "mtk_qos.global.cake_ack_filter": "1",
        }
        self.routes = "default via 192.168.1.1 dev eth1 metric 10\n"
        self.install_shims()

    def sync_uci(self):
        (self.logs / "uci.json").write_text(json.dumps(self.uci_kv))
        (self.logs / "routes.txt").write_text(self.routes)

    def install_shims(self):
        uci_sh = f"""#!/usr/bin/env python3
import json, sys
kv = json.loads(open({str(self.logs / 'uci.json')!r}).read())
args = sys.argv[1:]
while args and args[0] in ('-q', '-c'):
    args = args[2:] if args[0] == '-c' else args[1:]
if len(args) >= 2 and args[0] == 'get':
    val = kv.get(args[1])
    if val is None:
        sys.exit(1)
    print(val)
elif len(args) >= 2 and args[0] == 'show':
    pref = args[1] + '.'
    secs = sorted({{k.split('.')[1] for k in kv if k.startswith(pref) and k.split('.')[1].startswith('dev')}})
    for s in secs:
        print(f"{{args[1]}}.{{s}}=device")
elif len(args) == 0:
    sys.exit(1)
"""
        ip_sh = f"""#!/bin/sh
printf '%s\\n' "$*" >> {str(self.logs / 'ip.log')!r}
case "$*" in
  *"route show default"*) cat {str(self.logs / 'routes.txt')!r} ;;
esac
"""
        rec_sh = lambda name: f"""#!/bin/sh
printf '%s\\n' "$*" >> {str(self.logs / (name + '.log'))!r}
"""
        awk_sh = f"""#!/bin/sh
for arg do
  case "$arg" in
    {str(self.qdma_dir)}/qdma_txq*) printf '%s\\n' "$arg" >> {str(self.queue_reads)!r} ;;
  esac
done
exec /usr/bin/awk "$@"
"""
        ucode_sh = f"""#!/usr/bin/env python3
import json, pathlib, re, sys
args = sys.argv[1:]
kv = json.loads(open({str(self.logs / 'uci.json')!r}).read())
opts = {{}}
i = 1
while i < len(args):
    if args[i].startswith('--') and i + 1 < len(args):
        opts[args[i][2:]] = args[i + 1]
        i += 2
    else:
        i += 1
wan_devs = [x.strip() for x in opts.get('wan-devs', 'eth1').split(',') if x.strip()]
lan_devs = [x.strip() for x in opts.get('lan-devs', 'br-lan').split(',') if x.strip()]
wan_expr = '{{ ' + ', '.join(f'"{{d}}"' for d in wan_devs) + ' }}' if len(wan_devs) > 1 else f'"{{wan_devs[0]}}"'
lan_expr = '{{ ' + ', '.join(f'"{{d}}"' for d in lan_devs) + ' }}' if len(lan_devs) > 1 else f'"{{lan_devs[0]}}"'
lines = [
    'table inet mtk_qos',
    'delete table inet mtk_qos',
    'table inet mtk_qos {{',
    '\\tchain forward {{',
    '\\t\\ttype filter hook forward priority mangle; policy accept;',
    f'\\t\\toifname {{wan_expr}} tcp flags syn / syn,rst tcp option maxseg size set rt mtu',
    f'\\t\\tiifname {{wan_expr}} tcp flags syn / syn,rst tcp option maxseg size set rt mtu',
]
slots = []
if kv.get('mtk_qos.global.enabled') == '1' and kv.get('mtk_qos.global.eqos_enabled', '1') != '0':
    leases = {{}}
    for line in pathlib.Path(opts['leases']).read_text().splitlines():
        p = line.split()
        if len(p) >= 3:
            leases[p[1].lower()] = p[2]
    dev_ids = sorted({{k.split('.')[1] for k in kv if k.startswith('mtk_qos.dev')}})
    for slot, did in enumerate(dev_ids, 1):
        if kv.get(f'mtk_qos.{{did}}.enabled', '1') == '0':
            continue
        ip = kv.get(f'mtk_qos.{{did}}.ip', '')
        mac = kv.get(f'mtk_qos.{{did}}.mac', '').lower()
        if not ip and mac in leases:
            ip = leases[mac]
        ul = int(kv.get(f'mtk_qos.{{did}}.upload_kbps', '0') or 0)
        dl = int(kv.get(f'mtk_qos.{{did}}.download_kbps', '0') or 0)
        mc = int(kv.get(f'mtk_qos.{{did}}.max_conn', '0') or 0)
        if not (ip or mac):
            continue
        comment = (kv.get(f'mtk_qos.{{did}}.comment', '') or '').replace('|', ' ')
        slots.append(f"{{slot}}|{{ul}}|{{dl}}|{{mc}}|{{ip}}|{{mac}}|{{comment}}")
        if mc > 0 and opts.get('with-connlimit') == '1' and ip:
            lines.append(f'\\t\\toifname {{wan_expr}} ip saddr {{ip}} ct state new ct count over {{mc}} counter drop')
        if ul > 0 and ip:
            lines.append(f'\\t\\tiifname {{lan_expr}} oifname {{wan_expr}} ip saddr {{ip}} meta mark set {{slot}}')
        if ul > 0 and mac:
            lines.append(f'\\t\\tiifname {{lan_expr}} oifname {{wan_expr}} ether saddr {{mac}} meta mark set {{slot}}')
        if dl > 0 and ip:
            lines.append(f'\\t\\tiifname {{wan_expr}} oifname {{lan_expr}} ip daddr {{ip}} meta mark set {{32 + slot}}')
lines.extend(['\\t}}', '}}'])
pathlib.Path(opts['out-nft']).write_text('\\n'.join(lines) + '\\n')
pathlib.Path(opts['out-slots']).write_text(('\\n'.join(slots) + '\\n') if slots else '')
"""
        scripts = {
            "uci": uci_sh,
            "ip": ip_sh,
            "tc": rec_sh("tc"),
            "nft": rec_sh("nft"),
            "sysctl": rec_sh("sysctl"),
            "modprobe": rec_sh("modprobe"),
            "ubus": "#!/bin/sh\necho '{}'\n",
            "logger": "#!/bin/sh\nexit 0\n",
            "ucode": ucode_sh,
            "awk": awk_sh,
        }
        for name, body in scripts.items():
            p = self.bin_dir / name
            p.write_text(body)
            p.chmod(0o755)

    def run(self, cmd="start"):
        self.sync_uci()
        env = os.environ.copy()
        env["PATH"] = f"{self.bin_dir}:/usr/bin:/bin:/sbin:/usr/sbin"
        env["MTK_QOS_UCI_BIN"] = str(self.bin_dir / "uci")
        env["MTK_QOS_TC_BIN"] = str(self.bin_dir / "tc")
        env["MTK_QOS_NFT_BIN"] = str(self.bin_dir / "nft")
        env["MTK_QOS_UBUS_BIN"] = str(self.bin_dir / "ubus")
        env["MTK_QOS_IP_BIN"] = str(self.bin_dir / "ip")
        env["MTK_QOS_SYSCTL_BIN"] = str(self.bin_dir / "sysctl")
        env["MTK_QOS_MODPROBE_BIN"] = str(self.bin_dir / "modprobe")
        env["MTK_QOS_UCODE_BIN"] = str(self.bin_dir / "ucode")
        env["MTK_QOS_RENDER_UC"] = str(RENDER_UC)
        env["MTK_QOS_UCI_DIR"] = str(self.uci_dir)
        env["MTK_QOS_LEASES_FILE"] = str(self.leases)
        env["MTK_QOS_ARP_FILE"] = str(self.arp)
        env["MTK_QOS_QDMA_DIR"] = str(self.qdma_dir)
        env["MTK_QOS_HNAT_DIR"] = str(self.hnat_dir)
        env["MTK_QOS_PCE_DIR"] = str(self.pce_dir)
        env["MTK_QOS_PPE_STATUS"] = str(self.ppe_status)
        env["MTK_QOS_DT_COMPAT"] = str(self.dt_compat)
        env["MTK_QOS_SYS_NET"] = str(self.sys_net)
        env["MTK_QOS_SYS_MOD"] = str(self.sys_mod)
        env["MTK_QOS_RUN_DIR"] = str(self.run_dir)
        return subprocess.run(
            ["/bin/sh", str(SCRIPT), cmd],
            env=env,
            capture_output=True,
            text=True,
        )


class MtkQosTests(unittest.TestCase):
    def test_1_wired_wan_simultaneous_4coproc(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.routes = "default via 100.64.0.1 dev eth1 metric 10\n"
            env.uci_kv.update({
                "mtk_qos.dev1.enabled": "1",
                "mtk_qos.dev1.ip": "192.168.59.50",
                "mtk_qos.dev1.upload_kbps": "10000",
                "mtk_qos.dev1.download_kbps": "40000",
                "mtk_qos.dev1.max_conn": "200",
                "mtk_qos.dev1.comment": "host_note",
            })
            res = env.run("start")
            self.assertEqual(res.returncode, 0, res.stderr)
            self.assertEqual((env.qdma_dir / "qdma_txq1").read_text().strip(), "2 0 0 1 10000 4 4")
            self.assertEqual((env.qdma_dir / "qdma_txq2").read_text().strip(), "0 0 0 0 0 4 4")
            self.assertEqual((env.qdma_dir / "qdma_txq33").read_text().strip(), "3 0 0 1 40000 4 4")
            j_wired = json.loads(env.run("json").stdout)
            self.assertEqual(j_wired["eqos_slots"][0]["mac"], "")
            self.assertEqual(j_wired["eqos_slots"][0]["comment"], "host_note")
            self.assertEqual((env.hnat_dir / "hnat_setting").read_text().strip(), "11 5")
            self.assertEqual((env.hnat_dir / "sqm_toggle").read_text().strip(), "1")
            self.assertEqual((env.hnat_dir / "qos_toggle").read_text().strip(), "1")
            cls = (env.pce_dir / "cls_entry").read_text().strip()
            self.assertIn("sport 4 4", cls)
            self.assertIn("fport 4", cls)
            tc_log = (env.logs / "tc.log").read_text() if (env.logs / "tc.log").exists() else ""
            self.assertNotIn("cake", tc_log)

    def test_2_5g_wwan_zero_cpu_no_cake(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.routes = "default via 10.18.64.110 dev wwan0_1 metric 12\n"
            env.uci_kv.update({
                "mtk_qos.dev1.enabled": "1",
                "mtk_qos.dev1.mac": "AA:BB:CC:DD:EE:02",
                "mtk_qos.dev1.upload_kbps": "5000",
                "mtk_qos.dev1.download_kbps": "25000",
                "mtk_qos.dev1.max_conn": "150",
            })
            res = env.run("start")
            self.assertEqual(res.returncode, 0, res.stderr)
            nft_txt = (env.run_dir / "mtk_qos.nft").read_text()
            self.assertIn("ip daddr 192.168.59.66 meta mark set 33", nft_txt)
            self.assertIn("meta mark set 1", nft_txt)
            self.assertIn("ct count over 150", nft_txt)
            tc_log = (env.logs / "tc.log").read_text() if (env.logs / "tc.log").exists() else ""
            self.assertNotIn("cake", tc_log)
            j = json.loads(env.run("json").stdout)
            self.assertEqual(j["mode"], "coprocessor_4x")
            self.assertEqual(j["wan_kind"], "cellular_5g")
            self.assertEqual(j["wan_dev"], "wwan0_1")

    def test_3_nft_lan_bypass_and_pppoe_lower_dev(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            (env.sys_net / "pppoe-wan").mkdir(exist_ok=True)
            (env.sys_net / "pppoe-wan" / "lower_eth1").write_text("up\n")
            env.routes = "default dev pppoe-wan metric 5\n"
            env.uci_kv.update({
                "mtk_qos.dev1.enabled": "1",
                "mtk_qos.dev1.ip": "192.168.59.77",
                "mtk_qos.dev1.upload_kbps": "8000",
                "mtk_qos.dev1.download_kbps": "50000",
            })
            res = env.run("start")
            self.assertEqual(res.returncode, 0, res.stderr)
            nft_txt = (env.run_dir / "mtk_qos.nft").read_text()
            self.assertIn("pppoe-wan", nft_txt)
            self.assertIn("eth1", nft_txt)
            self.assertIn("oifname {", nft_txt)
            self.assertIn('iifname "br-lan"', nft_txt)

    def test_4_hook_toggle_0_cake_fallback(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.uci_kv["mtk_qos.global.ack_prio"] = "0"
            (env.hnat_dir / "hook_toggle").write_text("disabled\n")
            env.routes = "default via 10.18.64.110 dev wwan0_1 metric 12\n"
            res = env.run("start")
            self.assertEqual(res.returncode, 0, res.stderr)
            tc_log = (env.logs / "tc.log").read_text()
            self.assertIn("qdisc replace dev wwan0_1 root cake", tc_log)
            self.assertIn("qdisc replace dev ifb4mtkqos root cake", tc_log)
            j = json.loads(env.run("json").stdout)
            self.assertEqual(j["mode"], "cake_fallback")
            self.assertEqual((env.hnat_dir / "qos_ack_prio").read_text().strip(), "1")

    def test_5_stop_full_cleanup_and_rollback(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.uci_kv["mtk_qos.global.ack_prio"] = "0"
            env.uci_kv.update({
                "mtk_qos.dev1.enabled": "1",
                "mtk_qos.dev1.ip": "192.168.59.50",
                "mtk_qos.dev1.upload_kbps": "10000",
                "mtk_qos.dev1.download_kbps": "40000",
            })
            self.assertEqual(env.run("start").returncode, 0)
            res = env.run("stop")
            self.assertEqual(res.returncode, 0, res.stderr)
            self.assertEqual((env.hnat_dir / "sqm_toggle").read_text().strip(), "0")
            self.assertEqual((env.hnat_dir / "qos_toggle").read_text().strip(), "0")
            self.assertEqual((env.hnat_dir / "hnat_setting").read_text().strip(), "11 5")
            self.assertEqual((env.hnat_dir / "qos_ack_prio").read_text().strip(), "1")
            self.assertEqual((env.qdma_dir / "qdma_sch2").read_text().strip(), "0 wrr 1000000")
            self.assertEqual((env.pce_dir / "cls_entry").read_text().strip(), "31")
            nft_log = (env.logs / "nft.log").read_text()
            self.assertIn("delete table inet mtk_qos", nft_log)
            self.assertFalse((env.run_dir / "state").exists())

    def test_6_mt7987_skips_pce_and_prerebuild_guard(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.dt_compat.write_bytes(b"hiveton,h5000m\x00mediatek,mt7987\x00")
            (env.pce_dir / "cls_entry").unlink()
            env.routes = "default via 10.18.64.110 dev wwan0_1 metric 12\n"
            res = env.run("start")
            self.assertEqual(res.returncode, 0, res.stderr)
            modprobe_log = (env.logs / "modprobe.log").read_text() if (env.logs / "modprobe.log").exists() else ""
            self.assertNotIn("pce", modprobe_log)
            j = json.loads(env.run("json").stdout)
            self.assertEqual(j["pce"]["supported"], 0)
            self.assertEqual(j["pce"]["loaded"], 0)
            self.assertEqual((env.hnat_dir / "qos_toggle").read_text().strip(), "1")

            (env.hnat_dir / "qos_dpi").unlink()
            res2 = env.run("start")
            self.assertEqual(res2.returncode, 0, res2.stderr)
            self.assertEqual((env.hnat_dir / "qos_toggle").read_text().strip(), "1 uplink")

    def test_status_uses_cached_tier1_occupancy(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.ppe_status.write_text("tier1_ppe_bind 17\ntier1_ppe_total 1024\n")
            (env.hnat_dir / "all_entry").write_text("state=BIND\n" * 200)

            res = env.run("json")
            self.assertEqual(res.returncode, 0, res.stderr)
            status = json.loads(res.stdout)
            self.assertEqual(status["ppe_bound"], 17)
            self.assertEqual(status["ppe_total"], 1024)

            env.ppe_status.write_text("tier1_ppe_bind 17\n")
            unknown = json.loads(env.run("json").stdout)
            self.assertIsNone(unknown["ppe_bound"])
            self.assertEqual(unknown["ppe_total"], 0)

    def test_json_reads_each_qdma_queue_file_once(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            (env.qdma_dir / "qdma_txq1").write_text(
                "scheduler: 0\npacket count: 41\npacket drop: 3\n"
            )

            res = env.run("json")
            self.assertEqual(res.returncode, 0, res.stderr)
            status = json.loads(res.stdout)
            self.assertEqual(status["queues"]["ul_eqos"], [41, 3])
            queue_reads = env.queue_reads.read_text().splitlines()
            self.assertEqual(len(queue_reads), 62)
            self.assertEqual(len(set(queue_reads)), 62)

    def test_mandatory_hardware_write_failures_are_reported(self):
        for missing in ("qdma_txq2", "hnat_qos_toggle"):
            with self.subTest(missing=missing), tempfile.TemporaryDirectory() as td:
                env = FakeSysEnv(Path(td))
                if missing == "qdma_txq2":
                    (env.qdma_dir / missing).unlink()
                else:
                    (env.hnat_dir / "qos_toggle").unlink()

                res = env.run("start")
                self.assertNotEqual(res.returncode, 0)
                self.assertIn("failed", res.stderr)
                self.assertEqual((env.run_dir / "last_apply").read_text().strip(), "failed")

    def test_failed_apply_is_visible_after_prior_success(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            self.assertEqual(env.run("start").returncode, 0)
            (env.qdma_dir / "qdma_txq2").unlink()

            failed = env.run("reload")
            self.assertNotEqual(failed.returncode, 0)
            status = json.loads(env.run("json").stdout)
            self.assertEqual(status["active"], 1)
            self.assertEqual(status["last_apply"], "failed")

    def test_ack_priority_setting_is_binary_and_optional(self):
        for value in ("0", "1"):
            with self.subTest(value=value), tempfile.TemporaryDirectory() as td:
                env = FakeSysEnv(Path(td))
                env.uci_kv["mtk_qos.global.ack_prio"] = value
                res = env.run("start")
                self.assertEqual(res.returncode, 0, res.stderr)
                self.assertEqual((env.hnat_dir / "qos_ack_prio").read_text().strip(), value)

        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            (env.hnat_dir / "qos_ack_prio").unlink()
            res = env.run("start")
            self.assertEqual(res.returncode, 0, res.stderr)

        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.uci_kv["mtk_qos.global.ack_prio"] = "2"
            res = env.run("start")
            self.assertNotEqual(res.returncode, 0)
            self.assertIn("invalid ack_prio", res.stderr)

    def test_fast_bind_range_and_stop_restore(self):
        # Legacy mtk_qos.global.fast_bind_pkt is ignored; turboacc value is used directly
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.uci_kv["mtk_qos.global.fast_bind_pkt"] = "31"
            env.uci_kv["turboacc.config.fastpath_mh_eth_hnat_bind_rate"] = "18"
            self.assertEqual(env.run("start").returncode, 0)
            self.assertEqual((env.hnat_dir / "hnat_setting").read_text().strip(), "11 18")
            st = json.loads(env.run("json").stdout)
            self.assertEqual(st["fast_bind_pkt"], 18)
            self.assertEqual(env.run("stop").returncode, 0)
            self.assertEqual((env.hnat_dir / "hnat_setting").read_text().strip(), "11 18")

        # Out-of-range or unset turboacc bind rate falls back cleanly to 5 on start and stop
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.uci_kv["turboacc.config.fastpath_mh_eth_hnat_bind_rate"] = "99"
            self.assertEqual(env.run("start").returncode, 0)
            self.assertEqual((env.hnat_dir / "hnat_setting").read_text().strip(), "11 5")
            st = json.loads(env.run("json").stdout)
            self.assertEqual(st["fast_bind_pkt"], 5)
            self.assertEqual(env.run("stop").returncode, 0)
            self.assertEqual((env.hnat_dir / "hnat_setting").read_text().strip(), "11 5")

    def test_sync_noop_lease_update_and_drift_heal(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.uci_kv.update({
                "mtk_qos.dev1.enabled": "1",
                "mtk_qos.dev1.mac": "AA:BB:CC:DD:EE:02",
                "mtk_qos.dev1.upload_kbps": "5000",
                "mtk_qos.dev1.download_kbps": "25000",
            })
            self.assertEqual(env.run("start").returncode, 0)
            # Simulate kernel formatted sqm_toggle readback as exposed by mtk_hnat
            (env.hnat_dir / "sqm_toggle").write_text(
                "enabled=1 dpi=1 ul_eqos=1..15 ul_sqm=16..31 dl_eqos=33..47 dl_sqm=48..63 qos_toggle=1 ul=1 dl=1\n"
            )
            (env.logs / "nft.log").unlink(missing_ok=True)
            (env.hnat_dir / "qos_toggle").write_text("SENTINEL_UNTOUCHED\n")
            (env.qdma_dir / "qdma_sch2").write_text("SENTINEL_SCH2\n")

            # 1. Identical config + WAN + leases + healthy live hardware -> complete no-op
            self.assertEqual(env.run("sync").returncode, 0)
            self.assertFalse((env.logs / "nft.log").exists())
            self.assertEqual((env.hnat_dir / "qos_toggle").read_text().strip(), "SENTINEL_UNTOUCHED")
            self.assertEqual((env.qdma_dir / "qdma_sch2").read_text().strip(), "SENTINEL_SCH2")

            # 2. DHCP lease IP changes -> only nftables table updates; QDMA/qos_toggle stay untouched
            env.leases.write_text("1700000100 aa:bb:cc:dd:ee:02 192.168.59.77 nas *\n")
            self.assertEqual(env.run("sync").returncode, 0)
            self.assertTrue((env.logs / "nft.log").exists())
            self.assertIn("192.168.59.77", (env.run_dir / "mtk_qos.nft").read_text())
            self.assertEqual((env.hnat_dir / "qos_toggle").read_text().strip(), "SENTINEL_UNTOUCHED")
            self.assertEqual((env.qdma_dir / "qdma_sch2").read_text().strip(), "SENTINEL_SCH2")

            # 3. Kernel sqm_toggle drifts (qos_toggle=0) -> full re-apply heals hardware
            (env.hnat_dir / "sqm_toggle").write_text(
                "enabled=1 dpi=1 ul_eqos=1..15 ul_sqm=16..31 dl_eqos=33..47 dl_sqm=48..63 qos_toggle=0 ul=0 dl=0\n"
            )
            self.assertEqual(env.run("sync").returncode, 0)
            self.assertEqual((env.hnat_dir / "qos_toggle").read_text().strip(), "1")
            self.assertEqual((env.qdma_dir / "qdma_sch2").read_text().strip(), "1 wrr 49734")

            # 4. WAN switch (eth1 -> wwan0_1) -> full re-apply updates wan_kind & nft
            env.routes = "default via 10.18.64.110 dev wwan0_1 metric 12\n"
            self.assertEqual(env.run("sync").returncode, 0)
            st = json.loads(env.run("json").stdout)
            self.assertEqual(st["wan_kind"], "cellular_5g")
            self.assertEqual(st["wan_dev"], "wwan0_1")

    def test_dhcp_hotplug_skips_when_no_device_rules(self):
        with tempfile.TemporaryDirectory() as td:
            env = FakeSysEnv(Path(td))
            env.sync_uci()
            called = env.logs / "qos_called.log"
            qos_shim = env.bin_dir / "fake-qos"
            qos_shim.write_text(f"#!/bin/sh\necho \"$*\" >> {str(called)!r}\n")
            qos_shim.chmod(0o755)
            hook = PKG_DIR / "files" / "95-mtk-qos.dhcp"
            run_env = os.environ.copy()
            run_env.update({
                "ACTION": "add",
                "MTK_QOS_UCI_BIN": str(env.bin_dir / "uci"),
                "MTK_QOS_UCI_DIR": str(env.uci_dir),
                "MTK_QOS_BIN": str(qos_shim),
            })
            r1 = subprocess.run(["/bin/sh", str(hook)], env=run_env, capture_output=True, text=True)
            self.assertEqual(r1.returncode, 0)
            self.assertFalse(called.exists())

            env.uci_kv["mtk_qos.dev1.enabled"] = "1"
            env.sync_uci()
            r2 = subprocess.run(["/bin/sh", str(hook)], env=run_env, capture_output=True, text=True)
            self.assertEqual(r2.returncode, 0)
            self.assertEqual(called.read_text().strip(), "sync")


if __name__ == "__main__":
    unittest.main(verbosity=2)
