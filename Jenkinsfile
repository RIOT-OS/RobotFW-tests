/* This file uses both decalritive syntax and scripted.
 * This is because delaritive is simpler and generally preferred but suffer
 * from limited dynamic discovery and implementations.
 *
 * @Note If this Jenkinsfile is touched and merged into master it should also
 * manually be merged into nightly to be executed that night, otherwise it is
 * one night behind.
 */


import jenkins.model.*
@NonCPS
def getNodes(String label) {
    jenkins.model.Jenkins.instance.nodes.collect { thisAgent ->
        if (thisAgent.labelString.contains("${label}")) {
            return thisAgent.name
        }
    }
}


/* globals ================================================================== */
/* Global variables are decalred without `def` as they must be used in both
 * declaritive and scripted mode */
nodes = nodesByLabel('HIL')
collectBuilders = [:]
nodeBoardQueue = []
nodeBoards = []
nodeTests = []

/* pipeline ================================================================= */
pipeline {
    agent { label 'master' }
    stages {
        stage('setup master') {
            steps {
                stepCheckoutRobotFWTests()
                stepCheckoutRIOT()

                stepGetBoards()
                stepGetTests()
                stepArchiveMetadata()

                stepStashRobotFWTests()
            }
        }
        stage('setup build server and build') {
            steps {
                script {
                    processBuilderTask()
                    parallel collectBuilders
                }
            }
        }
        stage('node test') {
            steps {
                runParallel items: nodeBoards.collect { "${it}" }
            }
        }
        stage('compile results') {
            steps {
                stepCompileResults()
            }
        }
    }
}

/* master steps ============================================================= */

def stepCheckoutRobotFWTests() {
    pr = env.CHANGE_ID
    url = "https://github.com/RIOT-OS/RobotFW-tests.git"
    branch = env.BRANCH_NAME
    dir = '.'
    if (pr != "") {
        sh 'git config --global user.name "riot-hil-bot"'
        sh 'git config --global user.email "riot-hil-bot@haw-hamburg.de"'
        checkout([
            $class: 'GitSCM',
            branches: [[name: "pr/${pr}"]],
            extensions: [[$class: 'RelativeTargetDirectory',
                          relativeTargetDir: dir],
                         [$class: "PreBuildMerge",
                          options: [mergeTarget: "master",
                                    mergeRemote: "origin"]]],
            userRemoteConfigs: [[url: url,
                                 refspec: "+refs/pull/${pr}/head:refs/remotes/origin/pr/${pr}",
                                 credentialsId: 'github_token']]
        ])

    }
    else {
        checkout([
            $class: 'GitSCM',
            branches: [[name: "${branch}"]],
            extensions: [[$class: 'RelativeTargetDirectory',
                          relativeTargetDir: dir]],
            userRemoteConfigs: [[url: url,
                                 credentialsId: 'github_token']]
        ])
    }
}

def stepCheckoutRIOT() {
    checkout([
              $class: 'GitSCM',
              branches: [[name: "refs/heads/master"]],
              extensions: [[$class: 'RelativeTargetDirectory',
                            relativeTargetDir: "RIOT"]],
              userRemoteConfigs: [[url: "https://github.com/RIOT-OS/RIOT.git",
                                   credentialsId: 'github_token']]
            ])
}

def stepStashRobotFWTests() {
    stash name: "RobotFWTestsRepo",
          excludes: "RIOT/**, RobotFW-frontend/**"
}

def stepUnstashRobotFWTests() {
    unstash name: "RobotFWTestsRepo"
}

/* Runs a script to compile all tests results in the archive. */
def stepCompileResults()
{
    ret = sh script: '''
HIL_JOB_NAME=$(echo ${JOB_NAME}| cut -d'/' -f 1)
        HIL_BRANCH_NAME=$(echo $JOB_NAME| cut -d'/' -f 2)
        HIL_BRANCH_NAME=$(echo $HIL_BRANCH_NAME | sed 's/%2F/-/g')
        HIL_BRANCH_NAME=$(echo $HIL_BRANCH_NAME | sed 's/_/-/g')
        HIL_BRANCH_NAME=$(ls ${JENKINS_HOME}/jobs/${HIL_JOB_NAME}/branches/ | grep "^$HIL_BRANCH_NAME")
        ARCHIVE_DIR=${JENKINS_HOME}/jobs/${HIL_JOB_NAME}/branches/${HIL_BRANCH_NAME}/builds/${BUILD_NUMBER}/archive/build/robot/
        if [ -d $ARCHIVE_DIR ]; then
            ./dist/tools/ci/results_to_xml.sh $ARCHIVE_DIR
        fi
    ''', label: "Compile archived results"
}

/* node steps =============================================================== */
/* Cleans and clones both RobotFW-Tests and RIOT based on rfUrl,
 * rfCommitId, riotUrl, and riotCommitId in the node workspace.
 */

def buildOnBuilder(String agentName) {
    node("${agentName}") {
        stage("Building on ${agentName}") {
            stepCheckoutRobotFWTests()
            stepCheckoutRIOT()
            stepBuildJobs()
        }
    }
}

def processBuilderTask() {
    // Replace label-string with the label name that you may have
    def nodeList = getNodes("riot_build")
    def builders = []
    for(i=0; i < nodeList.size(); i++) {
        if (nodeList[i] != null && Jenkins.instance.getNode(nodeList[i]).toComputer().isOnline()) {
            builders.push(nodeList[i])
        }
    }

    for(i=0; i < builders.size(); i++) {
        def agentName = nodeList[i]
        // skip the null entries in the nodeList
        println "Preparing task for " + agentName
        collectBuilders["Build on " + agentName] = {
            buildOnBuilder(agentName)
        }
    }
}

/* Gets boards to test based on parameters or dynamically all available
 * boards connected to jenkins nodes.
 *
 * Sets nodeBoards
 */
def stepGetBoards() {
    nodeBoards = getBoardsFromNodesEnv()
    nodeBoardQueue = nodeBoards.clone()
    sh script: "echo collected boards: ${nodeBoards.join(",")}",
            label: "print boards"
}

/* Gets unique boards connected to jenkins nodes. */
def getBoardsFromNodesEnv() {
    script {
        boards = []
        for (int i=0; i < nodes.size(); ++i) {
            node (nodes[i]) {
                boards.push(env.BOARD)
            }
        }
        boards.unique()
        return boards
    }
}

/* Gets test based on parameters or dynamically all available
 * tests in RobotFW-Tests.
 *
 * Sets nodeTests
 */
def stepGetTests() {
    nodeTests = getTestsFromDir()
    sh script: "echo collected tests: ${nodeTests.join(",")}",
            label: "print tests"
}

/* Gets tests in tests directory. */
def getTestsFromDir() {
    script {
        tests = sh returnStdout: true,
                script: """
                    for dir in \$(find tests -maxdepth 1 -mindepth 1 -type d); do
                        [ -d \$dir/tests ] && { echo \$dir ; } || true
                    done
                """, label: "Collecting tests"
        tests = tests.tokenize()
        return tests
    }
}

/* Iterates through each board in nodeBoards and test in nodeTests and builds. */
def stepBuildJobs() {
    script {
        while (nodeBoardQueue.size() > 0) {
            def board = nodeBoardQueue.pop()
            for (int t_idx=0; t_idx < nodeTests.size(); t_idx++) {
                buildJob(board, nodeTests[t_idx])
            }
        }
    }
}

/* Iterates through each board in nodeBoards and test in nodeTests and builds
 * then stashes successful build binaries.
 *
 * For example, if a "board=samr21-xpro" and the test is "tests/periph_gpio",
 * the binaries (.hex, .bin, .elf) will be stashed in
 * "samr21_xpro_tests_periph_gpio".
 *
 * @param board The board to build
 * @param test  The test to build
 */
def buildJob(board, test) {
    exit_code = sh script: "RIOT_CI_BUILD=1 DOCKER_MAKE_ARGS=-j BUILD_IN_DOCKER=1 BOARD=${board} make -C ${test} clean all",
        returnStatus: true,
        label: "Build BOARD=${board} TEST=${test}"

    if (exit_code == 0) {
        /* Must remove all / to get stash to work */
        s_name = (board + "_" + test).replace("/", "_")
        stash name: s_name,
                includes: "${test}/bin/${board}/*.elf,${test}/bin/${board}/*.hex,${test}/bin/${board}/*.bin"
        sh script: "echo stashed ${s_name}", label: "Stashed ${s_name}"
    }
}

/* Add metadata file to archive */
def stepArchiveMetadata() {
    sh script: """
            mkdir -p build/robot
            python3 dist/tools/ci/env_parser.py -x -g -e --output=build/robot/metadata.xml
            """
    archiveArtifacts artifacts: "build/robot/metadata.xml"
}

/* test node steps ========================================================== */
/* Runs all tests on each board in parallel. */
def runParallel(args) {
    parallel args.items.collectEntries { name -> [ "${name}": {

        node (name) {
            stage("${name}") {
                /* We want to timeout a node if it doesn't respond
                 * The timeout should only start once it is acquired
                 */
                timeout(time: 60, unit: 'MINUTES') {
                    script {
                        stepRunNodeTests()
                    }
                }
            }
        }
    }]}
}

/* Tries to flash and test each test.
 *
 * If a test fails it catches and runs through the next one. Successful tests
 * uploads test artificats.
 *
 * Uses nodeTests.
 */
def stepRunNodeTests()
{
    catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
        stage( "${env.BOARD} setup on  ${env.NODE_NAME}"){
            stepUnstashRobotFWTests()
        }
        for (int i=0; i < nodeTests.size(); i++) {
            stage("${nodeTests[i]}") {
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE',
                        catchInterruptions: false) {
                    /* TODO: Test to see if the test compiled */
                    stepUnstashBinaries(nodeTests[i])
                    /* No need to reset as flashing and the test should manage
                     * this */
                    stepFlash(nodeTests[i])
                    stepTest(nodeTests[i])
                    stepArchiveTestResults(nodeTests[i])
                }
            }
        }
    }
}

/* Unstashes the binaries from the build server. */
def stepUnstashBinaries(test) {
    unstash name: "${env.BOARD}_${test.replace("/", "_")}"
}

/* Flashes binary to the DUT of the node. */
def stepFlash(test)
{
    sh script: "RIOT_CI_BUILD=1 make -C ${test} flash-only", label: "Flash ${test}"
}

/* Cleans the robot test directory and runs the robot tests. */
def stepTest(test)
{
    def test_name = test.replaceAll('/', '_')
    sh script: "make -C ${test} robot-clean || true",
            label: "Cleaning before ${test} test"
    /* We don't want to stop running other tests since the robot-test is
     * allowed to fail */
    catchError(buildResult: 'UNSTABLE', stageResult: 'UNSTABLE',
            catchInterruptions: false) {
        sh script: "make -C ${test} robot-test",
                label: "Run ${test} test"
    }
}

/* Archives the test results. */
def stepArchiveTestResults(test)
{
    def test_name = test.replaceAll('/', '_')
    def base_dir = "build/robot/${env.BOARD}/${test_name}/"
    archiveArtifacts artifacts: "${base_dir}*.xml", allowEmptyArchive: true
    junit testResults: "${base_dir}xunit.xml", allowEmptyResults: true
}
