# afx-cefhud-interop

## Old v6 (compatible with official HLAE)

https://github.com/advancedfx/afx-cefhud-interop/tree/v6

There's pre-built binaries for the v6 version here:
https://drive.google.com/drive/folders/1CQFGMYhmz4x9DxunmwhWMp37ow6YOBON  
First install / extract the Release-Base.7z and over that replace with the contents from Release.7z - the AfxHookSource.7z is not needed, since the officcial HLAE fully supports the v6 by now.

## New v7 (under development)

- Based on CEF 03/08/2019 - 3.3626.1895.g7001d56 / Chromium 72.0.3626.121
- Pre-releases available here: https://github.com/advancedfx/afx-cefhud-interop/releases

### Build instructions

- Obtain Visual Studio 2019 (e.g. Community edition) and make sure at least the "Desktop Development with C++ component" is installed.
- Clone the afx-interop-v7 branch with git:
```
cd /c/some/directory
git clone --recursive https://github.com/advancedfx/afx-cefhud-interop.git
cd afx-cefhud-interop
git checkout main
```

Now open the Start -> Visual Studios 2019 -> Developer Command Prompt for VS 2019

```
c:
cd c:\some\directory\afx-cefhud-interop
mkdir build
cd build
cmake -G "Visual Studio 16" -A x64 "-DUSE_SANDBOX=Off" ..
```

Then, open the newly created c:\some\directory\afx-cefhud-interop\build\cef.sln in Visual Studio 2019 and select **Release** and **x64** in the confugration, right click afx-cefhud-interop in Solution tree and select Build.

For instructions how to use the binary see the comments at the top of the example.html here:  
https://github.com/advancedfx/afx-cefhud-interop/blob/main/afx-cefhud-interop/examples/default/index.html#L4

----

The [Chromium Embedded Framework](https://bitbucket.org/chromiumembedded/cef/) (CEF) is a simple framework for embedding Chromium-based browsers in other applications. This repository hosts a sample project called "cef-project" that can be used as the starting point for third-party applications built using CEF.

## Quick Links

* Project Page - https://bitbucket.org/chromiumembedded/cef-project
* Tutorial - https://bitbucket.org/chromiumembedded/cef/wiki/Tutorial
* Support Forum - http://www.magpcss.org/ceforum/
