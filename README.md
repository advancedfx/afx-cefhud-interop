# afx-cefhud-interop

## FAQ

### Can I use this for browsing the internet?

You should strongly avoid loading any untrusted content with afx-cefhud-interop at the moment, this would put your PC at a high risk:
- CEF sandbox is disabled for technical reasons, so no sandbox protection
- even on v7 the CEF version is from 2019, so lots of unpatched security holes

## New v7 (under development)

- Based on CEF 07/27/2019 - 75.1.14+gc81164e+chromium-75.0.3770.100 / Chromium 75.0.3770.100
- Pre-releases available here: https://github.com/advancedfx/afx-cefhud-interop/releases
- Currently compatible with latest HLAE version

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
https://github.com/advancedfx/afx-cefhud-interop/blob/main/afx-cefhud-interop/assets/examples/default/index.html#L4
