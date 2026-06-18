# Angry Birds Replica

This is a replica of Angry Birds, built using the [PTSD](https://github.com/ntut-open-source-club/practical-tools-for-simple-design) framework.

## Prerequisites

Before building the project, ensure you have the necessary C++ tools and compilers installed.

### Ubuntu / Debian
```bash
sudo apt update
sudo apt install -y build-essential g++ clang cmake git ninja-build
```

### macOS
You can install the required tools using [Homebrew](https://brew.sh/):
```bash
xcode-select --install
brew install cmake ninja
```

## Quick Start

1. **Clone the repository**
   Make sure to include the `--recursive` flag to fetch the required submodules (such as the PTSD framework).
   ```bash
   git clone <YOUR_GIT_URL> --recursive
   cd Angry_Birds_Replica
   ```

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
