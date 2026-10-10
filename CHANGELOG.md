<!-- SPDX-License-Identifier: CC-BY-4.0 -->

# RELEASE NOTES:

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

