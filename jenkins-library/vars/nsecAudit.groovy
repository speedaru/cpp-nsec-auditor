/**
 * nsecAudit: Security Gate
 * * implements intelligent lifecycle management:
 * - smart update: syncs source and detects core changes via git diff.
 * - conditional compilation: skips cmake if core engine sources haven't changed.
 * - metadata injection: passes jenkins build id and job name to the report.
 * - build resilience: 10-minute timeout and post-failure artifact archiving.
 */
def call(Map config = [:]) {
    // setup & configuration
    def scanPath       = config.get('scanPath', '.')
    def relativeTool   = config.get('toolDir', 'nsec-auditor')
    def failOnError    = config.get('failOnError', true)
    def relativeReport = config.get('reportDir', 'reports/nsec')
    def toolRepoUrl    = 'https://github.com/speedaru/cpp-nsec-auditor'

    def workspace      = env.WORKSPACE
    def absToolPath    = "${workspace}/${relativeTool}"
    def absEngineDir   = "${absToolPath}/core-engine"
    def absReportDir   = "${workspace}/${relativeReport}"
    
    def isUnix   = isUnix()
    def shellCmd = isUnix ? { s -> sh s } : { s -> bat s }
    def binary   = isUnix ? "nsec_core" : "nsec_core.exe"

    echo "[NSEC-GATE] Initializing Security Gate Orchestration..."

    // ghost-proof source fetching and smart update logic
    boolean coreChanged = false
    
    // check if the directory is valid (contains CMakeLists)
    if (!fileExists("${absEngineDir}/CMakeLists.txt")) {
        echo "[NSEC-WARN] Auditor source missing or corrupt. Performing fresh clone..."
        dir(workspace) {
            if (isUnix) { 
                sh "rm -rf ${relativeTool}" 
            } else { 
                bat "rd /s /q ${relativeTool} 2>nul || exit 0" 
            }
            sh "git clone ${toolRepoUrl} ${relativeTool}"
        }
        coreChanged = true 
    } else {
        dir(absToolPath) {
            try {
                echo "[NSEC-INFO] Checking for auditor updates..."
                sh "git fetch origin main"
                
                // detect if core-engine files changed between head and remote
                def diffOutput = sh(
                    script: "git diff HEAD origin/main --name-only | grep 'core-engine/' || true", 
                    returnStdout: true
                ).trim()
                
                coreChanged = !diffOutput.isEmpty()
                sh "git merge origin/main"
                
                if (coreChanged) {
                    echo "[NSEC-INFO] C++ source changes detected. Rebuild required."
                }
            } catch (Exception e) {
                echo "[NSEC-ERROR] Git update failed. Performing recovery clone..."
                dir(workspace) {
                    if (isUnix) { sh "rm -rf ${relativeTool}" } else { bat "rd /s /q ${relativeTool}" }
                    sh "git clone ${toolRepoUrl} ${relativeTool}"
                }
                coreChanged = true
            }
        }
    }

    // conditional compilation (skip if binary exists and no changes detected)
    def buildDir = "${absEngineDir}/build"
    dir(buildDir) {
        if (isUnix) { sh "mkdir -p ." } else { bat "if not exist . mkdir ." }
        
        boolean binaryExists = fileExists(binary)
        if (!binaryExists || coreChanged) {
            echo "[NSEC-INFO] Compiling engine (Reason: ${!binaryExists ? 'Missing Binary' : 'Source Changes'})..."
            def buildArgs = isUnix ? "-- -j\$(nproc)" : ""
            shellCmd("cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release ${buildArgs}")
        } else {
            echo "[NSEC-INFO] Engine is up-to-date. Skipping build."
        }
    }

    // execution phase with timeout and metadata injection
    def exitCode = 999
    
    try {
        timeout(time: 10, unit: 'MINUTES') {
            // ensure the report directory exists before we start
            if (isUnix) { sh "mkdir -p '${absReportDir}'" } else { bat "if not exist \"${absReportDir}\" mkdir \"${absReportDir}\"" }

            echo "[NSEC-INFO] Running scan on: ${scanPath}"
            
            // invoke the python wrapper with injected build context
            exitCode = shellCmd(
                script: """python3 ${absToolPath}/scripts/nsec_wrapper.py \
                            --path "${workspace}/${scanPath}" \
                            --json-out "${absReportDir}/results.json" \
                            --build-id "${env.BUILD_ID}" \
                            --job-name "${env.JOB_NAME}" \
                            --engine-path "${buildDir}/${binary}" """,
                returnStatus: true
            )
        }
    } catch (org.jenkinsci.plugins.workflow.steps.FlowInterruptedException e) {
        echo "[NSEC-ERROR] Security scan timed out after 10 minutes."
        throw e
    } finally {
        // always run even if scan failed or timed out
        if (fileExists("${absReportDir}/results.json")) {
            echo "[NSEC-INFO] Archiving security scan evidence..."
            archiveArtifacts artifacts: "${relativeReport}/results.json", fingerprint: true
        }
    }

    // quality gate enforcement
    processResults(exitCode, failOnError)
}

/**
 * handles exit codes and build status
 */
def processResults(int exitCode, boolean failOnError) {
    if (exitCode == 0) {
        echo "[NSEC-PASS] Security Gate Passed."
    } else if (exitCode == 1) {
        def msg = "[NSEC-FAIL] Critical security violations detected by nsec-auditor."
        if (failOnError) {
            error(msg)
        } else {
            unstable(msg)
        }
    } else {
        error("[NSEC-ERROR] Auditor tool failed with system error code: ${exitCode}")
    }
}
