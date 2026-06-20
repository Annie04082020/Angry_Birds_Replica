# Angry Birds Replica

This is a replica of Angry Birds, built using the [PTSD](https://github.com/ntut-open-source-club/practical-tools-for-simple-design) framework.

## Prerequisites

Before building the project, ensure you have the necessary C++ tools and compilers installed.

### Ubuntu / Debian
```bash
sudo apt update
sudo apt install -y build-essential g++ clang cmake git ninja-build
```

> [!NOTE]
> If the game builds successfully but fails to launch (black screen, immediate crash, or no window appears), it is usually missing **runtime** dependencies rather than a build issue — most commonly an outdated OpenGL driver or missing SDL2 system libraries. If you're on WSL, make sure WSLg / a working OpenGL driver is set up; without it the window may fail to open even though the build succeeds.

### macOS
You can install the required tools using [Homebrew](https://brew.sh/):
```bash
xcode-select --install
brew install cmake ninja
```

## Quick Start

1. **Clone the repository**
   ```bash
   git clone <YOUR_GIT_URL>
   cd Angry_Birds_Replica
   ```

   > [!IMPORTANT]
   > Resource paths (images, sounds, etc.) are resolved at **compile time** relative to the project's source directory (`RESOURCE_DIR`, set in `CMakeLists.txt`). This means the project **must be built from the same location it was cloned into** — do not move or copy the build output to a different folder/path afterward, or it won't be able to find its assets. If you re-clone or relocate the project, you must rebuild it in the new location.

2. **Configure the project**
   > [!WARNING]
   > Please build your project in `Debug` mode because the `Release` path might be broken.
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Debug -B build -G Ninja
   ```
   *(If you don't use Ninja, you can simply run `cmake -DCMAKE_BUILD_TYPE=Debug -B build` instead)*

3. **Build the project**
   ```bash
   cmake --build build
   ```

4. **Run the game**
   ```bash
   ./build/Angry_Birds_Replica
   ```

For more advanced configuration or framework details, please refer to the [PTSD README](https://github.com/ntut-open-source-club/practical-tools-for-simple-design).

## Troubleshooting

- **Build succeeds but the game window doesn't open / crashes immediately**: This is almost always a missing runtime dependency (OpenGL driver, graphics library), not a code issue. On WSL, ensure WSLg or your GPU driver passthrough is correctly configured.
- **Game launches but assets (images/sounds) fail to load**: Make sure you built the project from the same path you cloned it into. Resource paths are baked in at compile time and won't resolve correctly if the project folder is moved after building.