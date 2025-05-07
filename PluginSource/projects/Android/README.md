
# Android Build Instructions

### Setup Environment Variables
```sh
# install so file to unity project
export NDK_LIBS_OUT="~/workspaecs/NativeRenderingPlugin/UnityProject/Assets/Plugins/Android/libs"
```


### Build
```sh
# windows
$NDK21/ndk-build.cmd


# linux/mac
$NDK21/ndk-build
```

`NDK21` is the path to your NDK installation. For example, `~/Library/Android/sdk/ndk/21.0.6113669` on macOS.


Finally output:
```sh
.
└── libs
    ├── arm64-v8a
    │   ├── libc++_shared.so     
    │   └── libRenderingPlugin.so
    ├── armeabi-v7a
    │   ├── libc++_shared.so     
    │   └── libRenderingPlugin.so
    ├── x86
    │   ├── libc++_shared.so     
    │   └── libRenderingPlugin.so
    └── x86_64
        ├── libc++_shared.so     
        └── libRenderingPlugin.so
```