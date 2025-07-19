# SqRequest for VC:MP

**SqRequest** is a plugin module for VC:MP 0.4 that enables **asynchronous HTTP requests** (GET, POST, PUT, DELETE) directly from Squirrel scripts. It uses `libcurl` for networking and `Sqrat` for binding to the Squirrel VM.

> Created by [RT]UmaR^

---

## Features

- Asynchronous, non-blocking HTTP requests using threads
- Supports `GET`, `POST`, `PUT`, and `DELETE` methods
- Custom headers and JSON/body payloads
- Thread-safe Squirrel VM callback dispatch

---

## Prerequisite

**Linux:**

```ini
sudo apt update
sudo apt install libcurl4-openssl-dev
```

**Windows:**

You must install:

- **libcurl Runtime**: Download and install curl for Windows from the official website:
  [📦 https://curl.se/windows/](https://curl.se/windows/)
  Choose the "Win64 - Generic - libcurl" version with SSL support (curl-8.x.x_amd64.msi).

- **VC++ Redistributable**: Required by most native plugins and libcurl:
  [🔗 Download VC++ 2015–2022 Redistributable (x64)](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)

---

After installing both, you're ready to run the plugin.
