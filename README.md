# cpp-nsec-auditor

A high-performance C++ static security auditor designed for "IT for IT" productivity and DevSecOps integration. This tool was developed to demonstrate technical maturity in C++ systems programming and automated security pipelines.

## 🚀 Overview

`cpp-nsec-auditor` is a modular static analysis tool that scans C++ source files to identify security vulnerabilities and maintainability risks. It features a parallelized engine capable of high-speed audits, making it suitable for pre-commit hooks and CI/CD pipelines. During initial testing, the engine scanned sample files (100 lines total) in **0.001 seconds**.

## ✨ Key Features

*   **High-Performance Core**: Leverages `std::async` for parallel file processing, maximizing CPU utilization during large-scale scans.
*   **Modular Rule Engine**: An extensible orchestrator managing a suite of security and logic checks through the `ISecurityRule` interface.
*   **Thread-Safe Reporting**: Consolidated findings into a JSON format (`audit_report.json`) optimized for dashboard consumption and CI/CD pass/fail logic.

## 🏗️ Architecture

The project is built with **C++20** and follows a clean, modular architecture:
*   **`nsec::core`**: Contains the orchestration logic and the rule interface.
*   **`nsec::models`**: Defines thread-safe `Issue` and `Report` data structures.
*   **`nsec::rules`**: concrete implementations of security and complexity checks.

## 🔍 Detection Rules

### 🛡️ Security Checks
These rules focus on memory safety and preventing common vulnerabilities like buffer overflows.

| Rule ID | Severity | Target | Detection Logic |
| :--- | :--- | :--- | :--- |
| **101** | **Critical** | `strcpy` | Flags usage of `strcpy` due to lack of bounds checking. Suggests `strncpy` or `std::string`. |
| **101** | **Warning** | `sprintf` | Flags `sprintf` as prone to buffer overflows. Suggests `snprintf` or `std::format`. |

### 🧠 Logic & Complexity Checks
These rules enforce "Clean Code" principles and ensure the software remains maintainable as it scales.

| Rule ID | Severity | Target | Detection Logic |
| :--- | :--- | :--- | :--- |
| **252** | **Warning** | Deep Nesting | Flags code reaching Level 5 nesting or higher. Suggests refactoring with "Early Returns". |
| **252** | **Warning** | Maintainability | Flags scopes that are both deep (Level 4+) and long (default >25 lines). |

### ⚙️ Rule Detailed Behavior

#### **Nesting Depth Tracker**
Unlike a simple keyword search, this rule implements a stack-based brace tracker:
*   **Initial Scope Skipping**: The engine ignores the first level of braces (function body) to focus on internal logic complexity.
*   **Stateful Suppression**: If a "Deep Nesting" violation is found, the engine suppresses further duplicate reports within that same specific branch to avoid "alert fatigue".
*   **Context-Aware Metrics**: It identifies a "Maintainability Risk" when a deeply nested scope is also vertically long, indicating that logic is becoming too difficult to follow.

#### **Banned Function Scanner**
This rule utilizes regular expressions with strict word boundaries (`\b`):
*   **Exact Match Only**: It prevents false positives by ensuring `custom_strcpy` is not flagged as `strcpy`.
*   **Positional Reporting**: It calculates the exact line number for every violation found within the source file.

## 📂 Project Structure

```text
cpp-nsec-auditor/
├── core-engine/             # C++ Auditor source code
│   ├── include/nsec/        # Headers (core, models, rules)
│   ├── src/                 # Implementation files
│   └── vendor/              # Third-party (nlohmann/json)
├── cicd/                    # CI/CD orchestration (JenkinsFile)
├── samples/                 # Vulnerable test files for verification
├── install_hooks.py         # Deployment script for Git hooks
└── audit_report.json        # Generated scan results
```

## 🛠️ Building the Project

The core engine requires a C++20 compliant compiler and CMake.
```bash
cd core-engine
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## 📊 Usage

Run the auditor against any C++ root directory (scans sub directories as well):
```bash
nsec-auditor.exe <target_path>
```

The tool provides a console summary and exports detailed findings to `audit_report.json`.
