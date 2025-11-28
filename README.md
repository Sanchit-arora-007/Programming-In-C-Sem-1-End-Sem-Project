# 🎮 Interactive Quiz Game in C  
[![Language](https://img.shields.io/badge/Language-C-blue.svg)]()
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)]()
[![Status](https://img.shields.io/badge/Status-Active-success.svg)]()
[![License](https://img.shields.io/badge/Use-Academic-yellow.svg)]()

A modern, interactive, and visually enhanced **Quiz Game** built in **C**, featuring real-time timed questions, animated console UI, dynamic themes, difficulty levels, random question logic, and a persistent high‑score system.

Developed by **Sanchit Arora (UPES)**.

---

## 📑 Table of Contents
- [Features](#-features)
- [Screenshots](#-screenshots)
- [Repository Structure](#-repository-structure)
- [Installation](#-installation)
- [How to Run](#-how-to-run)
- [Required Files](#-required-files)
- [System Workflow](#-system-workflow)
- [Testing](#-testing)
- [Future Enhancements](#-future-enhancements)
- [Author](#-author)
- [License](#-license)

---

## 🚀 Features

### 🎨 Modern UI & Themes
Supports 4 dynamic color themes:
- Neon Blue  
- Hacker Green  
- Cyberpunk  
- Pastel  

### ⏳ Real-time Timer
- Animated countdown  
- Last-second blinking  
- Warning beeps at 5, 3, 2, 1 seconds  
- Keyboard input without **Enter**  

### 🧠 Quiz Engine
- Loads questions from `questions.txt`  
- Random non-repeating selection  
- 5 questions per session  

### ⚙ Difficulty Modes
- Easy (15s)  
- Normal (10s)  
- Hard (6s + score multiplier)  

### 🏆 High Score System
- Saves score to `scores.txt`  
- Displays TOP 5 scores  
- Automatically sorted  

---

## 🖼 Screenshots
*(Add screenshots to `/docs/screenshots/` and they will display automatically)*

```
docs/screenshots/
   ├── menu.png
   ├── quiz_screen.png
   ├── timer_effect.png
   └── highscore.png
```

---

## 📁 Repository Structure
```
/
|-- src/
|   └── Quiz Game.c
|
|-- include/             
|
|-- docs/
|   ├── ProjectReport.pdf
|   ├── flowchart.png
|   └── screenshots/
|
|-- assets/
|
|-- sample_input.txt
|-- README.md
```

---

## 🛠 Installation

### 1️⃣ Clone Repository
```bash
git clone https://github.com/YourUsername/YourRepo.git
cd YourRepo
```

### 2️⃣ Compile (Windows)
```bash
gcc "src/Quiz Game.c" -o quiz.exe -lwinmm
```

> ⚠ Uses Windows APIs (`windows.h`, `Beep()`), so not cross‑platform.

---

## ▶ How to Run
```bash
quiz.exe
```

---

## 📦 Required Files

### **questions.txt**
```
Question text
A) Option A
B) Option B
C) Option C
D) Option D
A
```

### **scores.txt**
Auto-generated after first quiz session.

---

## 🔁 System Workflow
```
Start Program
     ↓
Load Questions
     ↓
Main Menu → Start Quiz / View High Scores / Settings / Exit
     ↓
Run Quiz Loop
     ↓
Store Score
     ↓
Back to Menu
```

---

## 🧪 Testing

| Feature Tested | Status |
|----------------|--------|
| Question loading | ✔ Passed |
| Timer & animations | ✔ Passed |
| Difficulty system | ✔ Passed |
| High score writing | ✔ Passed |
| Randomization | ✔ Passed |
| Menu navigation | ✔ Passed |

---

## 🌟 Future Enhancements
- GUI version (SDL / WinAPI)
- Online leaderboard
- JSON question bank
- Multiplayer mode
- Database (SQLite) for scores

---

## 👨‍💻 Author
**Sanchit Arora**  
UPES Dehradun

---

## 📜 License
This project is intended for academic use and learning.

---

