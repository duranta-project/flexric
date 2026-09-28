# SPDX-License-Identifier: MIT
# Add /opt/asn1c/bin to PATH for this script session
export PATH="/opt/asn1c/bin:${PATH}"

# Remove existing asn1c package
sudo apt remove -y asn1c

# Update package list
sudo apt update

# Install necessary dependencies
sudo apt install -y git autoconf automake libtool make gcc g++ m4

# Clone the specific version of asn1c
sudo git clone https://github.com/mouse07410/asn1c /tmp/asn1c

# Navigate to the cloned repository
cd /tmp/asn1c

# Checkout the specific commit
sudo git checkout 940dd5fa9f3917913fd487b13dfddfacd0ded06e

# Generate configuration files  
sudo autoreconf -iv

# Configure with custom prefix
sudo ./configure --prefix=/opt/asn1c

# Compile and install
sudo make -j$(nproc)
sudo make install

# Clean up
sudo rm -rf /tmp/asn1c

echo "asn1c installation completed successfully."

