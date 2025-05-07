#!/bin/bash

set -e

patch_ndk_on_apple_m1() {
    os=$(uname -s)
    arch=$(uname -m)

    if [ $os == "Darwin" -a $arch == "arm64" ]; then
        # Fix for building with NDK on Apple Silicon
        # https://stackoverflow.com/questions/69541831/unknown-host-cpu-architecture-arm64-android-ndk-siliconm1-apple-macbook-pro
        if [ ! -f $UNITY_NDK/ndk-build.old ]; then
            cp $UNITY_NDK/ndk-build $UNITY_NDK/ndk-build.old
        fi
        echo '#!/bin/sh'                                         > $UNITY_NDK/ndk-build
        echo 'DIR="$(cd "$(dirname "$0")" && pwd)"'             >> $UNITY_NDK/ndk-build
        echo 'arch -x86_64 /bin/bash $DIR/build/ndk-build "$@"' >> $UNITY_NDK/ndk-build
    fi
}

main() {

    patch_ndk_on_apple_m1

    # set UNITY_NDK environment variable to NDK installation used by Unity
    # i. e. /Applications/Unity/Hub/Editor/2022.1.24f1/PlaybackEngines/AndroidPlayer/NDK
    echo "Using UNITY_NDK: $UNITY_NDK"

    # https://developer.android.com/ndk/guides/ndk-build
    export NDK_PROJECT_PATH=$(cd ../ && pwd)
    echo "Using NDK_PROJECT_PATH: $NDK_PROJECT_PATH"
    $UNITY_NDK/ndk-build
}

main
