This page contains documentation for enabling positioning with FlexRIC and OAI.

# General

# Tutorials
You can either run the xApp in real-time with OAI or run with the E2 agent emulator. Both xApps include a GUI based on implot that displays the SRS channel estimates and a 2D representation of the testbed.
`xapp_srs_channel.cpp` is a specialized localization xApp that leverages the custom service model E2SM-SRS, the UE position are the result of ML-inference on a pre-trained CC model using `libtorch`.

`xapp_srs_emulator.cpp` also leverages the custom service model E2SM-SRS, plays back an SRS CIR dataset and provides the CC predictions.
Below are the common prerequisites of the xApps.

## Prerequisites
  We use LibTorch to implement the ML-inference. Thus, LibTorch should be installed in your system. You can refer to the [official website](https://pytorch.org/get-started/locally/). LibTorch is distributed with precompiled binaries, so no need to compile it.

  You would also need the the torchscript of the pre-trained CC model. For that, you would need to train your own model.

  To run the GUI, glfw3 and OpenGL should be installed. On Ubuntu you can install the package `libglfw3-dev` and `libopengl-dev`.


The following tutorials provide a step-by-step guide to running the localization xApps:

- [Testbed Setup](./doc/testbed_setup.md)
- [E2 Agent Emulator](./doc/emulator.md)
