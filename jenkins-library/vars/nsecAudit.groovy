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
    // 1. Context & Defaults
    def scanPath       = config.get('scanPath', '.')
    def relativeTool   = config.get('toolDir', 'nsec-auditor')
    def failOnError    = config.get('failOnError', true)
    def relativeReport  = config.get('reportDir', 'reports/nsec')

    // Construct Absolute Paths using WORKSPACE to ensure consistency across agents
    def workspace    = env.WORKSPACE
    def absToolPath  = "${workspace}/${relativeTool}"
    def absEngineDir = "${absToolPath}/core-engine"
    def absWrapper   = "${absToolPath}/scripts/nsec_wrapper.py"
    def absReportDir = "${workspace}/${relativeReport}"

    // Platform Detection
    def isUnix = isUnix()
    def binaryName = isUnix ? "nsec_core" : "nsec_core.exe"
    def shellCmd = isUnix ? { s -> sh s } : { s -> bat s }

    echo "[NSEC-INFO] Initializing Security Gate in Workspace: ${workspace}"

    // 2. Surgical Tool Provisioning
    // We check if the binary exists before attempting a build to save CI time
    dir("${absEngineDir}/build") {
        if (!fileExists(binaryName)) {
            echo "[NSEC-WARN] Core engine binary missing at ${absEngineDir}/build/${binaryName}. Starting compilation..."
            
            // Enterprise standard: use -j for parallel builds on Linux
            def buildArgs = isUnix ? "-- -j\$(nproc)" : ""
            shellCmd("cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release ${buildArgs}")
        } else {
            echo "[NSEC-INFO] Valid core engine binary detected."
        }
    }

    // 3. Execution Phase
    echo "[NSEC-INFO] Target Scan Path: ${scanPath}"

    // Ensure report directory exists
    if (isUnix) {
        sh "mkdir -p '${absReportDir}'"
    } else {
        bat "if not exist \"${absReportDir}\" mkdir \"${absReportDir}\""
    }

    // Run the Python Orchestrator
    // We use returnStatus: true to allow the Groovy script to handle the failure logic gracefully
    def exitCode = shellCmd(
        script: "python3 \"${absWrapper}\" --path \"${workspace}/${scanPath}\" --json-out \"${absReportDir}/nsec-results.json\"",
        returnStatus: true
    )

    // 4. Artifact & Evidence Management
    if (fileExists("${absReportDir}/nsec-results.json")) {
        echo "[NSEC-INFO] Security scan data found. Archiving artifacts..."
        archiveArtifacts artifacts: "${relativeReport}/*.json", allowEmptyArchive: true, fingerprint: true
    }

    // 5. Gatekeeper Logic (Quality Gate)
    // Exit Code 0: Clean, Exit Code 1: Vulnerabilities, Others: Tool Error
    switch(exitCode) {
        case 0:
            echo "[NSEC-PASS] No critical security violations detected."
            break
        case 1:
            def msg = "[NSEC-FAIL] Critical security violations identified by nsec-auditor."
            if (failOnError) {
                error(msg)
            } else {
                unstable(msg)
            }
            break
        default:
            error("[NSEC-ERROR] nsec_wrapper failed with system error code ${exitCode}. Check logs for stack traces.")
            break
    }
}
