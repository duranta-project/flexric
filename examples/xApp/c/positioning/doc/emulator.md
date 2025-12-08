# Tutorial: Channel Charting xApp with the FlexRIC E2 agent emulator

## Prerequisites
  The xApp loads samples from a real trajectory dataset obtained during experiments using Firecell GEO5G testbed at EURECOM. The dataset can be obtained from this repo:[5G SRS datasets](https://gitlab.eurecom.fr/ahadi/5g-srs-datasets)

  The dataset is stored as HDF5 so you need the HDF5 library on your system. On Ubuntu you can install the package `libhdf5-dev` (serial version)

## Building

   You can follow the same instructions found in the FlexRIC [README](https://gitlab.eurecom.fr/mosaic5g/flexric/-/blob/dev/README.md?ref_type=heads) but you need to specify the path where libtorch is installed in your `cmake` command:

   ```
   cmake -DCMAKE_PREFIX_PATH=/path/to/libtorch .. -GNinja
   ninja
   sudo ninja install
   ```

## Running

   Start the RIC:

   ```
   cd flexric/build/examples/ric
   ./nearRT-RIC
   ```
   Start the E2 agent emulator:
   ```
   cd flexric/build/examples/emulator/agent/
   ./emu_agent_gnb
   ```
   Start the CC xApp:
   ```
   cd flexric/build/examples/xApp/c/positioning
   ./xapp_srs_emulator // or ./xapp_srs_torch_minimal
   ```
