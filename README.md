# 🖼️ LSB Image Steganography in C

## 📘 Overview
This project implements **Least Significant Bit (LSB) Image Steganography** in the **C programming language**.  
It allows users to **hide (encode)** secret text or data files within **BMP images** and later **extract (decode)** them securely — without visible changes to the image.

The project demonstrates core embedded and systems programming concepts such as:
- Bitwise manipulation
- File I/O (binary mode)
- Structures and modular programming
- Memory management and validation logic
- Command-line argument handling

---

## ⚙️ Features
- 🔐 **Encode Mode** — Embed a secret text file into a `.bmp` image.
- 🔎 **Decode Mode** — Retrieve hidden data from a stego image.
- 🧮 **Image Capacity Check** — Ensures the image has enough bytes to hold the secret data.
- 💾 **Preserves Header & Metadata** — Copies BMP header intact to maintain compatibility.
- 🧠 **Magic String Validation** — Confirms successful encoding/decoding.
- 🧰 **Clear CLI Messages** — Displays progress and validation information step-by-step.

---

## 🧠 Concepts & Skills Demonstrated
- Bitwise operations for LSB encoding
- Binary file handling using `fopen`, `fread`, `fwrite`
- Pointers, dynamic memory, and structures
- Modular programming across multiple C files
- Error handling and data validation

---

## 🧰 Technologies Used
| Category | Tools / Concepts |
|-----------|------------------|
| **Language** | C |
| **Compiler** | GCC |
| **Image Format** | BMP |
| **Concepts** | File I/O, Bitwise Ops, Data Encoding/Decoding |
| **Platform** | Linux / Windows (CLI) |

---

## 🚀 How to Run

### 1️⃣ Clone the repository
```bash
git clone https://github.com/yourusername/lsb-image-steganography.git
cd lsb-image-steganography
