#!/bin/bash

lipo libcrypto.a -thin x86_64 -output libcrypto_x86_64
lipo libssl.a -thin x86_64 -output libssl_x86_64

lipo libcrypto.a -thin arm64 -output libcrypto_arm64
lipo libssl.a -thin arm64 -output libssl_arm64
