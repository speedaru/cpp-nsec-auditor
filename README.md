# cpp-nsec-auditor

![C++](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)

A high-performance C++ static security auditor designed for "IT for IT" productivity and DevSecOps integration. This tool demonstrates technical maturity in C++ systems programming and automated security pipelines.

## 📷 Demo video
[![Watch demo](./assets/thumbnail.png)](https://youtu.be/x0DHP8OiLT0)

## 🚀 Overview

`cpp-nsec-auditor` is a modular static analysis tool that scans C++ source files to identify security vulnerabilities and maintainability risks. It features a parallelized engine capable of high-speed audits, making it suitable for pre-commit hooks and CI/CD pipelines. During initial testing, the engine scanned sample files (100 lines total) in **0.001 seconds**.

## ✨ Key Features

*   **High-Performance Core**: Leverages `std::async` for parallel file processing, maximizing CPU utilization during large-scale scans.
*   **Modular Rule Engine**: An extensible orchestrator managing a suite of security and logic checks through the `ISecurityRule` interface.
*   **Thread-Safe Reporting**: Consolidated findings into a JSON format optimized for dashboard consumption and CI/CD pass/fail logic.
*   **Centralized Intelligence Hub**: A Flask-powered dashboard providing real-time visibility into security trends, compliance scores, and actionable violation logs.

## 🎣 Local Enforcement (The Surgical Guardrail)

To prevent security vulnerabilities from ever reaching the repository, this project implements a **Surgical Guardrail** using Git pre-commit hooks.

*   **Surgical Scanning**: Identifies only the files currently in the Git staging area to maintain developer productivity with near-instant feedback.
*   **Easy Deployment**: Includes an automated installation script (`install_hooks.py`) for team-wide deployment.

**To enable local enforcement:**
```bash
python install_hooks.py
```

## 🏗️ Architecture

The project is built with **C++20** and follows a modular design:
*   **`nsec::core`**: Orchestration logic and rule interfaces.
*   **`nsec::models`**: Thread-safe `Issue` and `Report` data structures.
*   **`nsec::rules`**: Concrete implementations of security and complexity checks.

## 🔍 Detection Rules

### 🛡️ Security Checks
| Rule ID | Severity | Target | Detection Logic |
| :--- | :--- | :--- | :--- |
| **101** | **Critical** | `strcpy` | Flags usage due to lack of bounds checking. Suggests `std::string`. |
| **101** | **Warning** | `sprintf` | Flags potential buffer overflows. Suggests `snprintf` or `std::format`. |

### 🧠 Logic & Complexity Checks
| Rule ID | Severity | Target | Detection Logic |
| :--- | :--- | :--- | :--- |
| **252** | **Warning** | Deep Nesting | Flags code reaching Level 5 nesting or higher. |
| **252** | **Warning** | Maintainability | Flags scopes that are both deep (Level 4+) and long (>25 lines). |

## 📂 Project Structure
```text
cpp-nsec-auditor/
├── assets/                  # Documentation assets (Videos, Images)
├── cicd/                    # Jenkinsfile samples for pipeline integration
├── core-engine/             # C++ Static Analysis Engine source code
│   ├── include/             # Headers (core, models, rules, ui, utils)
│   ├── src/                 # Implementation files
│   └── vendor/              # Third-party (nlohmann/json)
├── dashboard-ui/            # Flask-based Security Intelligence Hub
│   ├── app.py               # Telemetry API
│   └── templates/           # Dashboard UI
├── git-hooks/               # Pre-commit hook templates
├── jenkins-library/         # Global Jenkins Shared Library (Groovy)
├── scripts/                 # Python orchestrator for "Surgical Guardrail"
├── samples/                 # Vulnerable C++ code for UAT
├── install_hooks.py         # Deployment script for developers
└── README.md
```

## ⛓️ CI/CD & Enterprise Automation

This project features a production-ready **Jenkins Shared Library**, allowing security audits to be standardized across an entire organization.

*   **Standardized Security Gates**: Provides a global `nsecAudit()` function for any pipeline.
*   **Centralized Telemetry**: Each CI/CD run pushes results to the **Security Intelligence Hub**.

## 🖥️ Security Dashboard

The dashboard is a professional-grade visualization suite designed for Security Operations Centers (SOC).

*   **Real-Time Sync**: Automatically polls for new results without page refreshes.
*   **Trend Analysis**: Interactive Chart.js timeline showing the burn-down of issues over time.
*   **Audit Grid**: Deep-link drill-downs into specific build violations.

## 🔄 The DevSecOps Workflow

1.  **Code**: Developer adds a prohibited function (e.g., `strcpy`).
2.  **Commit**: `git commit` triggers the **Surgical Guardrail** to run a local audit.
3.  **Push**: Code is pushed to Jenkins, where the **Shared Library** performs a final compliance check.
4.  **Monitor**: Results are instantly visualized on the **NSEC Dashboard**.

## 🛠️ Prerequisites & Building

*   **Compiler**: C++20 compliant (GCC 10+, Clang 10+, or MSVC 19.28+).
*   **Build System**: CMake 3.15+.
*   **Python**: 3.8+ (for Flask UI and Wrapper).
```bash
cd core-engine
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## 📄 License
This project is licensed under the MIT License.
