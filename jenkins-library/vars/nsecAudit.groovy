/**
 * nsecAudit: Corporate Security Gate for Tier-1 Banking (Natixis Standard)
 * * This global variable handles the orchestration of the nsec-auditor tool.
 * It manages tool provisioning (CMake), execution (Python), and 
 * enforcement of security quality gates.
 *
 * @param config Map containing:
 * - scanPath:    Relative path to scan (default: '.')
 * - toolDir:     Relative path to nsec-auditor repo in workspace (default: 'nsec-auditor')
 * - failOnError: If true, fails the build on exit code 1 (default: true)
 * - reportDir:   Where to store JSON reports (default: 'reports/nsec')
 */
def call(Map config = [:]) {
    // 1. Context & Paths
    def scanPath       = config.get('scanPath', '.')
    def relativeTool   = config.get('toolDir', 'nsec-auditor')
    def failOnError    = config.get('failOnError', true)
    def relativeReport = config.get('reportDir', 'reports/nsec')

    def workspace    = env.WORKSPACE
    def absToolPath  = "${workspace}/${relativeTool}"
    def absEngineDir = "${absToolPath}/core-engine"
    def absReportDir = "${workspace}/${relativeReport}"

    def isUnix = isUnix()
    def shellCmd = isUnix ? { s -> sh s } : { s -> bat s }

    // 2. Ghost-Proof Source Fetching
    // We check for a marker file (CMakeLists) instead of just the folder name
    if (!fileExists("${relativeTool}/core-engine/CMakeLists.txt")) {
        echo "[NSEC-INFO] Auditor source invalid or missing. Performing fresh clone..."
        if (isUnix) { sh "rm -rf ${relativeTool}" } else { bat "rd /s /q ${relativeTool}" }
        sh "git clone https://github.com/speedaru/cpp-nsec-auditor ${relativeTool}" 
    }

    // 3. Safe Provisioning (No Ghost Build Folders)
    def buildDir = "${absEngineDir}/build"
    if (isUnix) { sh "mkdir -p '${buildDir}'" } else { bat "if not exist \"${buildDir}\" mkdir \"${buildDir}\"" }

    dir(buildDir) {
        def binaryName = isUnix ? "nsec_core" : "nsec_core.exe"
        if (!fileExists(binaryName)) {
            echo "[NSEC-WARN] Compiling core engine..."
            def buildArgs = isUnix ? "-- -j\$(nproc)" : ""
            shellCmd("cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release ${buildArgs}")
        }
    }

    // 4. Surgical Execution
    // We enter the tool directory so the Python wrapper finds its 'core-engine'
    dir(relativeTool) {
        if (isUnix) { sh "mkdir -p '${absReportDir}'" }

        // We point back to the workspace root for the scan path
        dir("${workspace}/${scanPath}")
        echo "running nsec python wrapper in ${workspace}/${scanPath}"

        def exitCode = shellCmd(
            script: "python3 scripts/nsec_wrapper.py",
            returnStatus: true
        )

        // Archive and Gatekeeper logic remains the same...
        processResults(exitCode, relativeReport, failOnError)
    }
}

// Helper to keep the main call clean
def processResults(exitCode, reportDir, failOnError) {
    if (exitCode == 1) {
        error("[NSEC-FAIL] Critical security violations identified.")
    } else if (exitCode != 0) {
        error("[NSEC-ERROR] Auditor crashed with code ${exitCode}")
    }
}
