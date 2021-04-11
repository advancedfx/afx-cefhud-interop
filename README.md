# Short instructions

This project is still under development, so it's not fully ready yet, esepcially the afx_interop version 7.

The master branch is probably not fully functionial atm (sorry about that), the official HLAE is compatbile with the v6 branch released a while ago:  
https://github.com/advancedfx/afx-cefhud-interop/tree/v6

There's already pre-built binaries for the v6 version here:
https://drive.google.com/drive/folders/1CQFGMYhmz4x9DxunmwhWMp37ow6YOBON  
First install / extract the Release-Base.7z and over that replace with the contents from Release.7z - the AfxHookSource.7z is not needed, since the officcial HLAE fully supports the v6 by now.

If you want to build afx_interop_v6 yourself please:

- Obtain Visual Studio 2019 (e.g. Community edition) and make sure at least the "Desktop Development with C++ component" is installed.
- Clone the v6 branch with git:
```
cd /c/some/directory
git clone --recursive https://github.com/advancedfx/afx-cefhud-interop.git
cd afx-cefhud-interop
git checkout v6
```

Now open the Start -> Visual Studios 2019 -> Developer Command Prompt for VS 2019

```
c:
cd c:\some\directory\afx-cefhud-interop
mkdir build
cd build
cmake -G "Visual Studio 16" -A x64 ..
```

Then, open the newly created c:\some\directory\afx-cefhud-interop\build\cef.sln in Visual Studio 2019 and select **Release** and **x64** in the confugration, right click afx-cefhud-interop in Solution tree and select Build.

For instructions how to use the binary see the comments at the top of the example.html here:  
https://github.com/advancedfx/afx-cefhud-interop/blob/main/afx-cefhud-interop/example.html

----

The [Chromium Embedded Framework](https://bitbucket.org/chromiumembedded/cef/) (CEF) is a simple framework for embedding Chromium-based browsers in other applications. This repository hosts a sample project called "cef-project" that can be used as the starting point for third-party applications built using CEF.

# Quick Links

* Project Page - https://bitbucket.org/chromiumembedded/cef-project
* Tutorial - https://bitbucket.org/chromiumembedded/cef/wiki/Tutorial
* Support Forum - http://www.magpcss.org/ceforum/
