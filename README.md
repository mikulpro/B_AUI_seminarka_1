# Code Translator – Qt 6 GUI

A small cross‑platform application that translates code snippets from one programming language to another using either a locally hosted **gtp‑oss:20b** model or the **Google AI Studio** API.

## Features

- Two large text fields: **Source** and **Translated**.
- Choose source & target languages from a drop‑down.
- Select translation provider:
  - **Local (gtp‑oss:20b)** – requires a running HTTP endpoint.
  - **Google AI Studio** – needs a valid API key.
- The translated output is shown instantly in the bottom field.

## Build Instructions

The project uses **Qt 6** and **CMake**.

### macOS

```bash
# Install Qt 6 via Homebrew (if you don't have it already)
brew install qt@6

# Create a build directory
mkdir build && cd build

# Configure (point to Qt installation)
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)

# Build
cmake --build . --config Release

# Run
./code_translator.app/Contents/MacOS/code_translator
```

### Windows 11

```cmd
:: Install Qt 6 via vcpkg or the official installer.

mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

:: Run from the command line
build\Release\code_translator.exe
```

## Running the App

1. **Start a local model** (optional):
   ```bash
   gtp-oss:20b --port 8080
   ```
   The endpoint will be `http://localhost:8080/v1/chat/completions`.

2. **Open the GUI** and choose the provider.

3. **Paste your code** into the Source field, pick source/target languages, and click **Translate**.

## Known Limitations

- The local provider assumes the model endpoint matches the **gtp‑oss:20b** OpenAI‑compatible spec.
- The Google AI Studio endpoint uses the **text‑bison‑001** model; other models may need adjustment.
- No rate‑limiting or authentication for the local server.

## Attribution

Commit messages created by Claude Code should end with:
```
Co-Authored-By: Claude Code <noreply@anthropic.com>
```
Pull requests should end with:
```
🤖 Generated with [Claude Code](https://claude.com/claude-code)
```
---

*Happy translating!*