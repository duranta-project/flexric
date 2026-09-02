# SPDX-License-Identifier: LicenseRef-CSSL-1.0

"""
xapp_spectrum.py — Python port of the E2SM-DAPP spectrum xApp.

Mirrors the behaviour of examples/xApp/c/spectrum/xapp_spectrum.c:
  - Two independent subscriptions per E2 node:
      Subscription A (Style 1): E3 data reports  → DAPPFrmt1Callback
      Subscription B (Style 2): subscription map → DAPPFrmt2Callback
  - A periodic control loop that cycles through predefined PRB blacklists
    and sends them to every connected E2 node.
  - Clean shutdown on SIGINT / SIGTERM.

Usage:
    python3 xapp_spectrum.py
"""

import xapp_sdk as ric
import time
import signal
import threading

##########################################################################
# Constants
##########################################################################

DAPP_SM_ID            = 255
E3_RAN_FUNC_ID_SPECTRUM = 1
DAPP_ID               = 1

CTRL_PERIOD_SEC  = 5
MAX_RUNTIME_SEC  = 600

# Predefined PRB blacklist variants (mirrors ctrl_variants[] in the C xApp)
CTRL_VARIANTS = [
    [75, 80, 85, 90],
    [100, 101, 102, 103, 104],
    [75, 76, 77, 103, 104, 105],
    [80, 90, 100],
    [],                          # empty = unblock all
]

##########################################################################
# Indication callbacks
##########################################################################

class DAPPFrmt1Callback(ric.dapp_e3_data_cb):
    """Handles Format 1 indications: E3 data reports (spectrum PRBs)."""

    def __init__(self):
        ric.dapp_e3_data_cb.__init__(self)

    def handle(self, ind):
        age_us = (time.time_ns() - ind.timestamp_ns) // 1000
        print(
            f"[DAPP][FMT1] Indication: ran_function_id={ind.ran_function_id}, "
            f"dapp_id={ind.dapp_id}, age={age_us} us"
        )
        print(
            f"[DAPP][FMT1]   node_type={ind.node_type}, "
            f"plmn_id={ind.plmn_0:02x}{ind.plmn_1:02x}{ind.plmn_2:02x}, "
            f"node_nb_id={ind.node_nb_id}"
        )
        if ind.node_cu_du_id_present:
            print(f"[DAPP][FMT1]   node_cu_du_id={ind.node_cu_du_id}")

        prbs = list(ind.prbs)
        print(f"[DAPP][FMT1] Spectrum E3 payload: prb_count={len(prbs)}")
        print(f"[DAPP][FMT1] PRBs: {prbs}")


class DAPPFrmt2Callback(ric.dapp_sub_map_cb):
    """Handles Format 2 indications: dApp E3 subscription map."""

    def __init__(self):
        ric.dapp_sub_map_cb.__init__(self)

    def handle(self, ind):
        age_us = (time.time_ns() - ind.timestamp_ns) // 1000
        print(f"[DAPP][FMT2] Indication (subscription map): age={age_us} us")
        print(
            f"[DAPP][FMT2]   node_type={ind.node_type}, "
            f"plmn_id={ind.plmn_0:02x}{ind.plmn_1:02x}{ind.plmn_2:02x}, "
            f"node_nb_id={ind.node_nb_id}"
        )
        if ind.node_cu_du_id_present:
            print(f"[DAPP][FMT2]   node_cu_du_id={ind.node_cu_du_id}")

        subs = list(ind.dapp_subs)
        if not subs:
            print("[DAPP][FMT2] No dApps currently registered on this E2 node")
            return

        print(f"[DAPP][FMT2] dApp E3 subscriptions: {len(subs)} dApp(s)")
        for item in subs:
            funcs = list(item.e3_ran_functions)
            print(f"[DAPP][FMT2]   dapp_id={item.dapp_id}, "
                  f"e3_ran_functions({len(funcs)}): {funcs}")


##########################################################################
# Control helper
##########################################################################

def send_spectrum_control(conn, variant_idx):
    """Send a PRB blacklist control to every connected E2 node."""
    prbs = CTRL_VARIANTS[variant_idx % len(CTRL_VARIANTS)]

    # Build a SWIG uint16_t vector
    prb_vec = ric.DAPP_PRBVector()
    for p in prbs:
        prb_vec.push_back(p)

    print(
        f"[DAPP RC] Sending control variant {variant_idx % len(CTRL_VARIANTS)} "
        f"— {len(prbs)} blocked PRBs: {prbs}"
    )

    for node in conn:
        ric.control_dapp_spectrum_sm(node.id, E3_RAN_FUNC_ID_SPECTRUM, DAPP_ID, prb_vec)


##########################################################################
# Signal handling — mirrors the C xApp's signal_thread pattern
#
# init_xapp_api() (called by ric.init()) installs a C-level SIGINT handler
# that calls try_stop_xapp_api() + exit(), bypassing Python cleanup.
# We prevent that by:
#   1. Blocking SIGINT/SIGTERM in the main thread *before* ric.init().
#   2. Running a dedicated Python thread that waits for the signal via
#      signal.sigwait() and sets g_terminate — exactly like the C xApp's
#      signal_thread / sigwait() pattern.
#
# With the signal blocked everywhere, the C sig_handler is never invoked;
# our thread catches the signal, sets g_terminate, and the main loop
# exits cleanly to run the unsubscribe / cleanup code.
##########################################################################

g_terminate = False

def _signal_thread():
    global g_terminate
    signal.sigwait({signal.SIGINT, signal.SIGTERM})
    g_terminate = True

# Block SIGINT/SIGTERM before ric.init() so the C sig_handler never fires.
signal.pthread_sigmask(signal.SIG_BLOCK, {signal.SIGINT, signal.SIGTERM})

sig_thr = threading.Thread(target=_signal_thread, daemon=True)
sig_thr.start()

##########################################################################
# Main
##########################################################################

ric.init()

conn = ric.conn_e2_nodes()
assert len(conn) > 0, "No E2 nodes connected"
print(f"[DAPP RC] Connected E2 nodes = {len(conn)}")

# ----- Print DAPP RAN function definition (mirrors print_dapp_sm_info in C xApp) -----
for node in conn:
    ric.print_dapp_ran_func_info(node.id)

# ----- Subscribe: Style 1 — E3 data (one handle per node) -----
cb_e3_data  = DAPPFrmt1Callback()
cb_sub_map  = DAPPFrmt2Callback()

hndl_e3_data = []
hndl_sub_map = []

for node in conn:
    h1 = ric.report_dapp_e3_data_sm(node.id, cb_e3_data)
    hndl_e3_data.append(h1)
    print(f"[DAPP RC] Installed E3-data subscription (style 1), handle={h1}")

# ----- Subscribe: Style 2 — subscription map -----
for node in conn:
    h2 = ric.report_dapp_sub_map_sm(node.id, cb_sub_map)
    hndl_sub_map.append(h2)
    print(f"[DAPP RC] Installed sub-map subscription (style 2), handle={h2}")

# ----- Periodic control loop -----
elapsed = 0
variant = 0

while not g_terminate and elapsed < MAX_RUNTIME_SEC:
    send_spectrum_control(conn, variant)
    variant += 1

    for _ in range(CTRL_PERIOD_SEC):
        if g_terminate:
            break
        time.sleep(1)
        elapsed += 1

if g_terminate:
    print("[DAPP RC] Termination signal received, shutting down...")
else:
    print("[DAPP RC] Max runtime reached, shutting down...")

# ----- Unsubscribe -----
for h in hndl_e3_data:
    ric.rm_report_dapp_e3_data_sm(h)

for h in hndl_sub_map:
    ric.rm_report_dapp_sub_map_sm(h)

time.sleep(1)

while ric.try_stop() == False:
    time.sleep(0.001)

print("[DAPP RC] Test xApp run SUCCESSFULLY")
