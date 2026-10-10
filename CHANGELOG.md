<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# RELEASE NOTES:

## [v3.0.0](https://github.com/duranta-project/flexric/releases/tag/v3.0.0) -> October 2026.

Release v3.0.0 for FlexRIC. Changes since release v2.0.0:

### Major:

- New Service Model: E2SM-CCC v6.00
- E2SM-RC extensions:
   * REPORT Style 1 ("RRC Message" and "UE ID") and support for multiple REPORT styles
   * REPORT Style 5 ("On Demand Report") with aperiodic RIC Indication sent on demand
   * Neighbour Relation Table IE
   * RAN Parameter Definition encoding/decoding, including nested (recursive) definitions
   * Channel quality parameters retrieved when available
- E2SM-KPM extensions:
   * REPORT Style 1 ("E2 Node Measurement") - Cell-level measurements, distribution bins (distBinX/Y/Z)
   * Hash table of measurements defined by 3GPP TS 28.552
   * CU-UP support (and fixes for CU-CP/CU-UP in E2AP v1/v3)
- New xApps:
   * RF channel reconfiguration xApp (`xapp_rf_reconfiguration`)
   * Handover xApp driving RC Control Style 3 (`xapp_rc_handover`)
   * RC monitor xApp (`xapp_rc_moni`)
   * Zero-touch Energy Saving xApp based on cell utilization (`xapp_es_with_cell_util`)
- Monitoring: SQLite backend for KPM (all measurements, distributions) and RC data, plus Grafana dashboards for bare-metal and Docker
- F1AP ASN.1 definitions added
- RRC ASN.1 definitions added
- Symbols of E2AP v1/v2/v3 are suffixed and hidden (`-fvisibility=hidden`) to avoid clashes when linking with OAI RAN
- Graceful exit (SIGTERM) for the nearRT-RIC and E2 agents
- Licensing and documentation
   * Re-license the project from OAI Public License v1.1 to CSSL v1.0
   * Re-license documentation under CC-BY-4.0 and orchestration/CI assets under MIT
   * Add NOTICE, LICENSES/, SECURITY.md, AGENTS.md
   * Update main README.md: installation guide, `ns-O-RAN-flexric` (`ns3-oran`) integration by Orange Innovation Egypt and Orange Polska, O-RAN SC nearRT-RIC notes, Keysight PlugFest Fall 2023 notes
- Repository moved to GitHub under Duranta Project

### Minor:

- E2 agent emulators support KPM and RC (including aperiodic subscriptions and multiple RC REPORT styles)
- Removal of aperiodic subscriptions, including multiple aperiodic delete subscriptions
- Handling of the RIC CONTROL FAILURE message
- Build: minimum CMake version 3.16, gcc version check, security build options (incl. `FORTIFY_SOURCE`), and `XAPP_MULTILANGUAGE` off by default
- Docker: Ubuntu and CentOS Stream 10 (replaces RHEL 9 and Rocky 9) images, E2 agent emulators and xApps containers, plus a Docker test environment
- Runtime options: nearRT-RIC IP address, DB directory and name, and `XAPP_DURATION` (infinite by default)
- Clang/gcc warnings cleaned up (integer widths, aliasing, missing returns, static functions)

### Bug fixes:

- nearRT-RIC no longer aborts when an xApp connects before any E2 node is registered
- E2AP: warn instead of aborting on unexpected initiating criticality
- E2AP v3 fix when using CU and DU
- Fix `fd > 0` failure at `consume_fd` in the RIC (externally reported)
- E2SM-RC: correct RAN Function Name, fix encoding/comparison/copy/free of nested RAN Parameter Definitions, and drop invalid size asserts
- E2SM-KPM: fix RAN Function Name memory leak, PDSCH MCS measurement, PHR expected value, and measurement units of RRU.PrbTotDl/Ul and DRB.PdcpSduVolumeDL/UL (aligned with TS 28.552)
- Database: support channel quality measurements obtained via E2SM-RC, and E2SM-KPM measurements
- Larger RIC SUBSCRIPTION REQUEST buffer and correct `call_process_id` allocation
- Bimap fix, wrong AMF region ID assert removed, and memory leak fixed in `xapp_rc_moni`
- Fixes in the TC xApp message type check, slice monitor script and JSON indication output
- All declared pending event types are accepted in the library

## [v2.0.0](https://github.com/duranta-project/flexric/releases/tag/v2.0.0) -> December 2023.

Release v2.0.0 for FlexRIC. Some changes from release 1.0.0

### Major:

- E2AP multi version supported i.e., v1/v2/v3
- Complete KPM v2.01/v2.03 and v3.00 support
- Support for 7 measurements in OAI RAN
- Complete RC v1.03 support (for periodic and aperiodic events)
- Major API redesign
- Multi vendor interoperability (e.g., RICtest (Keysight), OSC RIC, TIM RIC)
- Scalability improved with a new thread pool in the nearRT-RIC

### Minor:

- Code tested with ASan, TSan, cppcheck, scan-build, gcov
- Ctest added
- Automatic xApp subscription delete send when the xApp abruptly disconnects
- gcc and clang compilers tested
- Docker build utilized to ensure deployability in Ubuntu 20 and 22
- Diverse bug fixes

## [v1.0.0](https://github.com/duranta-project/flexric/releases/tag/v1.0.0) -> November 2022.

This new release contains many new features as well as extended documentation.
The matrix containing the features implemented divided by system components can be found in the README.md file at the root of the source tree.
From this release on, we apply a semantic versioning release process following Major.Minor.Patch numbering.

### Features:

- O-RAN KPM v.2.02 service model (experimental).
- Custom NG/GTP service model.
- Custom Slice Control (SC) service model.
- Custom Traffic Control (TC) service model.
- Utility to generate the code of a generic service model.
- Multi-RAT (4G/5G) and multi-vendor (OAI, SRS) supports.
- Replaced NNG protocol with E42 protocol as an extension of E2 to support RIC-xApp northbound interface.
- Multi-language xApp development environment bound to the SDK interface via SWIG. Currently supported languages are C/C++ and Python.
- Agent-RIC-xApp simulation environment with E2 and E42 interface.
- FlexRIC configuration file

### Documentation:

- xApp tutorial with steps to create new xApps.
- Service Model development tutorial.
- Updated README.md.
- FlexRIC presentation.

# INITIAL FLEXRIC TAG:

## [v0.1](https://github.com/duranta-project/flexric/releases/tag/v0.1) -> January 2022.

FlexRIC version used for CoNext '21, without TC

