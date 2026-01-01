# Development Environment Setup Guide.
Follow these steps to set up the environment for STM32 development on Windows 11.

## 1. Install Package Manager (Scoop).
We use **Scoop** for streamlined tool management. Open PowerShell and run:

```powershell
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser;
irm get.scoop.sh | iex
```

## 2. Adding the necessary repositories.
```powershell
scoop bucket add main
scoop bucket add extras
```

## 3. And finally:
```powershell
scoop install meson
scoop install ninja
scoop install cmake
scoop install llvm
scoop install gcc-arm-none-eabi
scoop install stlink
scoop install openocd
scoop install git
```

meson, ninja, cmake, llvm and gcc-arm-none-eabi 
used for project configuration and assembly.

## 4. Configuring VS Code.
To work, you will need to install the following extensions in VS Code:

- [C/C++ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack)
- [CMake](https://marketplace.visualstudio.com/items?itemName=twxs.cmake)
- [CMake Tools](marketplace.visualstudio.com)
- [Cortex-Debug](marketplace.visualstudio.com)
- [Memory View](https://marketplace.visualstudio.com/items?itemName=mcu-debug.memory-view)
- [Tasks](https://marketplace.visualstudio.com/items?itemName=actboy168.tasks)
- [Command Variable](https://marketplace.visualstudio.com/items?itemName=rioj7.command-variable)

I also recommend the Doxygen Documentation.

## 5. Install STM32CubeMX.
https://www.st.com/en/development-tools/stm32cubemx.html

Open STM32CubeMX.
Go to the menu Help -> Manage embedded software packages.
Find the STM32F1 series and install the latest version (for example, V1.8.5).
The libraries will be downloaded to the folder:
C:/Users/Bob/STM32Cube/Repository/STM32Cube_FW_F1_VX.X.X/Drivers/CMSIS.

## 6. In Windows - add the environment variable.
STM32Cube_DIR with the path value to your cube repository.
For example c:\Users\User\STM32Cube\Repository\STM32Cube_FW_F1_V1.8.6
