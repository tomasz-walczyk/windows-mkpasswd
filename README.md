Very simple implementation of mkpasswd for Windows.
___
Application encrypts passwords using random salt and SHA512 algorithm, passwords are read from stdin and written in encrypted form to stdout.
You need to install [Microsoft Visual C++ Redistributable for Visual Studio 2015 ](https://www.microsoft.com/pl-pl/download/details.aspx?id=48145)
first if you want to use a prebuilt application.
___
#### Build instructions:
**You may need to start PowerShell console as an administrator if your installation directory requires administrator privileges for writing.**
```powershell
git clone https://twalczyk@bitbucket.org/twalczyk/mkpasswd-win.git
Set-Location (New-Item mkpasswd-win/build -ItemType Directory)
cmake -G "Visual Studio 14 2015 Win64" ..
cmake --build . --config Release --target INSTALL
```
___
*Copyright (C) 2019 Tomasz Walczyk*

*This software may be modified and distributed under the terms*
*of the MIT license. See the [LICENSE](LICENSE) file for details.*

