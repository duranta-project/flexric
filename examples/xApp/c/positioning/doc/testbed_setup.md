# Tutorial: Positioning Based on Channel Charting with OAI and FlexRIC

## Requirements
  The implementation was tested with an O-RAN testbed using two Category-A O-RUs from VVDN configured on $100Mhz$ maximum bandwitdth, using $30kHz$ subcarrier spacing in TDD mode.
  Each RU is equipped with four antennas.

## Integration with OAI
  The implementation is integrated with the E2 agent in OAI in the branch [srs_e2](https://gitlab.eurecom.fr/oai/openairinterface5g/-/tree/srs_e2?ref_type=heads)

  You will have to follow both the [FH 7.2 tutorial](https://gitlab.eurecom.fr/oai/openairinterface5g/-/blob/develop/doc/ORAN_FHI7.2_Tutorial.md#vvdn-lpru) to install the FH library and configure the RU.
  As well as the [E2AP tutorial](https://gitlab.eurecom.fr/oai/openairinterface5g/-/blob/develop/openair2/E2AP/README.md) to configure the E2 agent in OAI.
  You can either use the FlexRIC git submodule inside OAI to install the service models, or clone FlexRIC and install them independently of OAI.

   ```bash
   cd openairinterface5g/
   git checkout srs_e2
   cd cmake_targets
   ./build_oai -I  # if you never installed OAI, use this command only once 
   ./build_oai --gNB -t oran_fhlib_5g --ninja --build-e2 --cmake-opt -Dxran_LOCATION=/path/to/phy/fhi_lib/lib -C
  ```
## Building

   You can follow the same instructions found in the FlexRIC [README](https://gitlab.eurecom.fr/mosaic5g/flexric/-/blob/dev/README.md?ref_type=heads) but you need to specify the path where libtorch is installed in your `cmake` command:

   ```
   cmake -DCMAKE_PREFIX_PATH=/path/to/libtorch .. -GNinja
   sudo ninja install
   ```

## Running

   Start the gNB:

   ```
   cd openairinterface5g/cmake_targets/ran_build/build
   sudo ./nr-softmodem -O /path/to/conf_file/gnb.conf --sa --reorder-thread-disable 1 --thread-pool 2,4,6,8,10,12

   ```
   Start the RIC:

   ```
   cd flexric/build/examples/ric
   ./nearRT-RIC
   ```
   Start the CC xApp:

   ```
   cd flexric/build/examples/xApp/c/positioning
   XAPP_DURATION=-1 ./xapp_srs_channel
   ```
