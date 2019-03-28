Very simple implementation of mkpasswd for Windows.
___
Application encrypts passwords using random salt and SHA512 algorithm. Passwords are read from stdin and are written in encrypted form to stdout.
___
#### Build instructions:
**You may need to start PowerShell console as an administrator if your installation/packaging directory requires administrator privileges for writing.**
```powershell
git clone https://github.com/tomasz-walczyk/windows-mkpasswd.git
Set-Location (New-Item windows-mkpasswd/build -ItemType Directory)
cmake -G "Visual Studio 15 2017 Win64" ..
cmake --build . --config Release --target Package
```
___
*Copyright (C) 2019 Tomasz Walczyk*

*This software may be modified and distributed under the terms*
*of the MIT license. See the [LICENSE](LICENSE) file for details.*
