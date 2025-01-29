# Mylorawan-AES-128
Deployment of AES-128 loraWan using NS-3

## Lorawan Cryptography ##
LoRaWAN (Long Range Wide Area Network) is a protocol designed for low-power, wide-area networks (LPWANs) that enables long-range communication for IoT devices. Security is a critical aspect of LoRaWAN, and it uses AES-128 (Advanced Encryption Standard with a 128-bit key) to ensure data confidentiality, integrity, and authenticity.

 **AES-128 in LoRaWAN**
AES-128 is a symmetric encryption algorithm, meaning the same key is used for both encryption and decryption. It is widely regarded as secure and is used in many other protocols and applications. In LoRaWAN, AES-128 is used for:
- Encrypting application payloads.
- Generating MICs for message integrity.
- Deriving session keys during the activation process.
of cryptographic keys are essential to maintaining the security of a LoRaWAN network.

### Features ###

The LoraWan module used in NS-3 developed by David Magrin of [Signet Lab]( https://github.com/signetlabdei/lorawan) can be found at the following link:

[NS-3 module for simulation of LoraWan network]( https://apps.nsnam.org/app/lorawan/)



copy the file mylorawan-energy.cc in the contrib folder and compile it using:

./waf  --run  mylorawan-energy 

The mylorawan-energy.cc scenario sends a packet of 12-byte every 05 seconds with an integrated energy harvesting module, the end devices are randomly distributed around the gateway.

