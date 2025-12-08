This page contains documentation for enabling positioning with FlexRIC and OAI.

# General
The custom service model E2SM-SRS exposes the FAPI SRS.indication message from the E2 agent to the Near-RT RIC, the SRS.indication includes the UL-SRS frequency domain channel estimates.
We provide examples of xApps that leverage this service model for ML-based localization.
# Tutorials
You can either run xApps in real-time with OAI or run with the E2 agent emulator.
`xapp_cc_testbed.cpp` is a specialized localization xApp that leverages the custom service model E2SM-SRS, the UE position are the result of ML-inference on a pre-trained CC model using `libtorch`, it also includes a GUI based on implot that displays the SRS channel estimates and a 2D representation of the testbed.

`xapp_srs_emulator.cpp` also leverages the custom service model E2SM-SRS and displays the GUI, plays back an SRS CIR dataset and provides the CC predictions.
Below are the common prerequisites of the xApps.

`xapp_srs_torch_minimal.cpp` is a minimal xApp that supports Libtorch, it can be used as a base for implementing the ML inference, it currently only runs with the E2 agent emulator. The xApp unpacks the SRS CIR, processes the CIR, the inference and model should be implemented. This xApp doesn't include the GUI.
## Prerequisites
  We use LibTorch to implement the ML-inference. Thus, LibTorch should be installed in your system. You can refer to the [official website](https://pytorch.org/get-started/locally/). LibTorch is distributed with precompiled binaries, so no need to compile it.

  You would also need the the torchscript of the pre-trained CC model. For that, you would need to train your own model.

  To run the GUI, glfw3 and OpenGL should be installed. On Ubuntu you can install the package `libglfw3-dev` and `libopengl-dev`.


The following tutorials provide a step-by-step guide to running the localization xApps:

- [Testbed Setup](./doc/testbed_setup.md)
- [E2 Agent Emulator](./doc/emulator.md)
