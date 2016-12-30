# DshowTestFilters
#### Example DirectShow Filters Project

This project creates a DLL containing example DirectShow filters of various sophistication and usefulness.

* "Example Filter" is defined in [`CExFilter.cpp`](DshowAudioTransform/CExFilter.cpp). It is a close adaptation of the example code in [this MSDN article](https://msdn.microsoft.com/en-us/library/dd391015(v=VS.85).aspx) deriving from CTransformFilter. It has no real function other than to serve as an example.

* "Audio Transfom Filter" is defined in [`CATFilter.cpp`](DshowAudioTransform/CATFilter.cpp). It is derived from CTransInPlaceFilter and designed to work with PCM audio streams. Current function is hard coded to reducing the amplitude of the input waveform by 50%.

* "Wave Source Filter" is defined in [`CWaveSource.cpp`](DshowAudioTransform/CWaveSource.cpp). [This article](https://msdn.microsoft.com/en-us/library/windows/desktop/dd757807(v=vs.85).aspx) helped provide some guidance on its creation. It is derived from CSource, with a capture pin derived from CSourceStream. It generates a tone as a proof of concept.

## Windows 10 DirectShow Setup

DirectShow is an older technology, and requires some setup to use. The setup process I have followed is documented below.

1. Install the Windows 7 SDK ([Download Here](https://www.microsoft.com/en-us/download/details.aspx?id=8279))
    1. Manually uninstall VC++ 2010 Runtime (x86 and x64) from Control Panel->Programs and Features.
    2. Download and run the Windows 7 SDK Web Installer.
    3. Ensure "Samples" and "Visual C++ 2010 Runtime" options are included in the selection.
2. Build the DirectShow Base Class Library
    1. Run the C++ project solution, **"C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\directshow\baseclasses\baseclasses.sln"**, that was installed with the SDK.
    2. Allow Visual Studio to update project references.
    3. Build in both Debug and Release modes.
    4. Verify **"C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\directshow\baseclasses\Debug\strmbasd.lib"** and **"C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\directshow\baseclasses\Release\strmbase.lib"** exist.
3. Set Up Project References
    1. Create or Open C++ Project.
    2. Navigate to project properties->Configuration Properties->VC++ Directories.
    3. Add **"C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\directshow\baseclasses"** to Include Directories for both Debug and Release.
    4. Add **"C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\directshow\baseclasses\Debug"** and **"C:\Program Files\Microsoft SDKs\Windows\v7.1\Samples\multimedia\directshow\baseclasses\Release"** to Library Directories for both Debug and Release (not strictly necessary to do both, but it won't hurt).
    5. Under Linker->Input->Additional Dependencies, add strmbasd.lib, msvcrtd.lib, and winmm.lib on Debug, and strmbase.lib, msvcrt.lib, and winmm.lib on Release

## Running the Filters

The DLL may be registered for system use through the following command, ran from the directory containing the DLL:

```cmd
regsvr32.exe .\DshowAudioTransform.dll
```

Similarly, it may be unregistered through the command:

```cmd
regsvr32.exe -u .\DshowAudioTransform.dll
```

After registration, the filters should be available for use through GraphEdit and other DirectShow interfacing applications.

In addition, it is possible to debug the filter by setting the Local Windows Debugger command to GraphEdit, as has been done for this project. Simply implement the filter in a graph, and set a breakpoint.

## Credits

* [MSDN: Building DirectShow Filters](https://msdn.microsoft.com/en-us/library/windows/desktop/dd318238(v=vs.85).aspx) contains key information about writing DirectShow Filters.
* [This StackOverflow answer](http://stackoverflow.com/a/3483072) explains how exactly to set up the base classes for development.
* [MSDN: Introduction to DirectShow](https://msdn.microsoft.com/en-us/library/windows/desktop/dd390354(v=vs.85).aspx) provides some much-needed basic knowledge of filter development.
