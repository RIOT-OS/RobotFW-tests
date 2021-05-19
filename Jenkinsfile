/* This file uses both decalritive syntax and scripted.
 * This is because delaritive is simpler and generally preferred but suffer
 * from limited dynamic discovery and implementations.
 *
 * @Note If this Jenkinsfile is touched and merged into master it should also
 * manually be merged into nightly to be executed that night, otherwise it is
 * one night behind.
 */


import jenkins.model.*

/* globals ================================================================== */
/* Global variables are decalred without `def` as they must be used in both
 * declaritive and scripted mode */
collectBuilders = [:]
boardTestQueue = []
totalResults = [:]
nodeBoards = []

/* pipeline ================================================================= */
pipeline {
    agent { label 'master' }
    options {
        // If the whole process takes more than x hours then exit
        // This must be longer since multiple jobs can be started but waiting on nodes to complete
        timeout(time: 3, unit: 'HOURS')
        // Failing fast allows the nodes to be interrupted as some steps can take a while
        parallelsAlwaysFailFast()
    }
    stages {
        stage('setup master') {
            steps {
                stepCheckoutRobotFWTests()
                stepCheckoutRIOT()

                stepFillBoardTestQueue()
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
                stepMultiBranchCompileResults()
            }
        }
    }
}

/* master steps ============================================================= */
def stepCheckoutRobotFWTests() {
    helperCheckoutRepo("https://github.com/RIOT-OS/RobotFW-tests.git",
                       env.CHANGE_ID,
                       env.BRANCH_NAME)
}

def stepCheckoutRIOT() {
    helperCheckoutRepo("https://github.com/RIOT-OS/RIOT.git",
                       "",
                       "refs/heads/master",
                       "RIOT")
}

def stepFillBoardTestQueue() {
    nodeBoards = getBoardsFromNodes()
    tests = getTests()
    totalResults = getEmptyResultsFromBoards(nodeBoards)
    boardTestQueue = getBoardTestQueue(nodeBoards, tests)
}

def stepArchiveMetadata() {
    archiveMetadata()
}
def stepStashRobotFWTests() {
    stashRobotFWTests()
}

/* Runs a script to compile all tests results in the archive. */
def stepMultiBranchCompileResults()
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
   for(builder in getActiveBuildNodes()) {
       def agentName = builder
        println "Preparing task for " + agentName
        collectBuilders["Build on " + agentName] = {
            buildOnBuilder(agentName)
        }
    }
}

def stepBuildJobs() {
    buildJobs(boardTestQueue, totalResults)
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
                        flashAndRFTestNodes(totalResults)
                    }
                }
            }
        }
    }]}
}

/*
Since it seems having a dynamic declaritive pipeline that runs parallel on
many different nodes without saving the job scripts in an scm therefore not
existing in the workspace makes the idea of common code hard. This should be
copied and pasted at the bottom of each job and updated as needed until a
more clever person comes along... Which should not be that hard...
*/

/* common =================================================================== */

/* common master ============================================================ */

/* Helps make the checkout simpler by adding in all information common to
 * this setup.
 * Note:
 * - either `url` or `owner, repo` must specified.
 * - if pr has a number it will be used and branch will be ignored
 * - if pr is empty branch must be specified
 */
def helperCheckoutRepo(url='', pr='', branch='', dir='.', owner='', repo='') {

    if (url == "") {
        assert owner != ''
        assert repo != ''
        url = "https://github.com/${owner}/${repo}.git"
    }
    if (pr != "" && pr != null) {
        sh 'git config --global user.name "riot-hil-bot"'
        sh 'git config --global user.email "riot-hil-bot@haw-hamburg.de"'
        chk = checkout([
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
        assert branch != ''
        chk = checkout([
            $class: 'GitSCM',
            branches: [[name: "${branch}"]],
            extensions: [[$class: 'RelativeTargetDirectory',
                          relativeTargetDir: dir]],
            userRemoteConfigs: [[url: url,
                                 credentialsId: 'github_token']]
        ])
    }
    return chk
}

def getBoardsFromNodes(boards='all') {
    if (boards == 'all') {
        boards = []
        for (node_name in nodesByLabel('HIL')) {
            node (node_name) {
                boards.push(env.BOARD)
            }
        }
        boards = boards.unique()
    }
    else {
        boards = boards.tokenize(', ')
        /* TODO: Validate if the boards are connected */
    }
    return boards
}

/* This expects a single string and will return a list of tests, running
 * this again with a list will not work out...
 */
def getTests(tests='all') {
    if (tests == 'all') {
        tests = sh returnStdout: true,
                script: """
                    for dir in \$(find tests -maxdepth 1 -mindepth 1 -type d); do
                        [ -d \$dir/tests ] && { echo \$dir ; } || true
                    done
                """, label: "Collecting tests"
    }
    tests = tests.tokenize()
    return tests
}

/* We should initialized the Map before using it, this should be done in master
 * as it doesn't always play nice with parallel nodes.
 */
def getEmptyResultsFromBoards(boards) {
    results = [:]
    for (board in boards) {
        results[board] = [:]
    }
    return results
}

/* Should only be done in master once. It takes a list of tests and boards
 * and gives a list of Maps containing named board and test. This allows
 * multiple build servers to just pop the items so we have a resolution of
 * test/board.
 */
def getBoardTestQueue(boards, tests) {
    board_test_queue = []

    for (test in tests) {
        for (board in boards) {
            board_test_queue << ["board": (board), "test": (test)]
        }
    }
    return board_test_queue
}

def stashRobotFWTests() {
    stash name: "RobotFWTestsRepo",
          excludes: "RIOT/**, RobotFW-frontend/**"
}

/* After all the tests have generated the results and stored them in the
 * this should go through the archive and generate any post-processing scripts
 * such as adding each single xunit result into one and, if required,
 * generating html pages for the results webserver.
 */
def compileResults(generate_html=False)
{
    if (generate_html) {
        ret = sh script: '''
            HIL_JOB_NAME=$(echo ${JOB_NAME}| cut -d'/' -f 1)
            ARCHIVE_DIR=${JENKINS_HOME}/jobs/${HIL_JOB_NAME}/builds/${BUILD_NUMBER}/archive/build/robot/
            if [ -d $ARCHIVE_DIR ]; then
                ./dist/tools/ci/results_to_xml.sh $ARCHIVE_DIR
                cd RobotFW-frontend
                ./scripts/xsltprocw.sh -c ../config-live.xml -b ${HIL_JOB_NAME} -n ${BUILD_NUMBER} -v /var/jenkins_home/jobs/

            fi
        ''', label: "Compile archived results"
    }
    else {
        ret = sh script: '''
            HIL_JOB_NAME=$(echo ${JOB_NAME}| cut -d'/' -f 1)
            ARCHIVE_DIR=${JENKINS_HOME}/jobs/${HIL_JOB_NAME}/builds/${BUILD_NUMBER}/archive/build/robot/
            if [ -d $ARCHIVE_DIR ]; then
                ./dist/tools/ci/results_to_xml.sh $ARCHIVE_DIR
            fi
        ''', label: "Compile archived results"
    }
}

/* This keeps track of the state that the tests were run in. This should make
 * any issues regarding reproducibility slightly easier.
 */
def archiveMetadata() {
    sh script: """
            mkdir -p build/robot
            python3 dist/tools/ci/env_parser.py -x -g -e --output=build/robot/metadata.xml
            """
    archiveArtifacts artifacts: "build/robot/metadata.xml"
}

/* common riot_build ======================================================== */
/* Returns a list of online builder nodes. It seems each node should be
 * cloned or copied before being used.
 */
def getActiveBuildNodes() {
    return nodesByLabel('riot_build')
}


/* Empties a queue of
 * [[board: "my_board_1", test: "my_test_1"],
 *  [board: "my_board_1", test: "my_test_2"],
 *  [board: "my_board_2", test: "my_test_1"],
 *  [board: "my_board_2", test: "my_test_2"]]
 * or something like that and builds the test for that board.
 * This is intended to be used across multiple build servers so we need to be
 * a bit special when it comes to paralellism. The `board_test_queue` is
 * assumed to be a global variable as it gets populated by the master and used
 * on the build servers. The `results` are another story. This is used to track
 * if the build was successful, skipped, or failed. Again, assumed to be
 * global as it populated in the build server and used on the test nodes.
 * It is also assumed to be initialied with all keys corresponding to boards
 * and having an empty Map that will be populated with a test key and
 * build and supported results.  Maybe it is better to look at an example:
 * [my_board_1: [my_test_1: [build: true, support: true],
 *               my_test_2: [build: false, support: false]],
 *  my_board_2: [my_test_1: [build: false, support: true],
 *               my_test_2: [build: true, support: true]]
 * ]
 * Clear as mud?
 */
def buildJobs(board_test_queue, results, extra_make_cmd = "") {
    while (board_test_queue.size() > 0) {
        def boardtest = board_test_queue.pop()
        buildJob(boardtest['board'], boardtest['test'], results, extra_make_cmd)
    }
}

/* Actually builds the job, look at buildJobs for more info.
 * Long story short, calls make, stashes successful binaries,
 * populates the results.
 */
def buildJob(board, test, results, extra_make_cmd = "") {
    catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE',
            catchInterruptions: false) {
        results[board][test] = ['build': false, 'support': false]
        exit_code = sh script: "RIOT_CI_BUILD=1 DOCKER_MAKE_ARGS=-j BUILD_IN_DOCKER=1 BOARD=${board} make -C ${test} clean all ${extra_make_cmd} 2>build_output.log",
            returnStatus: true,
            label: "Build BOARD=${board} TEST=${test}"

        if (exit_code == 0) {
            /* Must remove all / to get stash to work */
            results[board][test]['build'] = true
            s_name = (board + "_" + test).replace("/", "_")
            catchError(buildResult: 'UNSTABLE', stageResult: 'UNSTABLE',
            catchInterruptions: false) {
                stash name: s_name, includes: "${test}/bin/${board}/*.elf,${test}/bin/${board}/*.hex,${test}/bin/${board}/*.bin"
                results[board][test]['support'] = true
            }
        }
        else {
            def output = readFile('build_output.log').trim()
            if (output.contains("There are unsatisfied feature requirements")) {
                results[board][test]['support'] = false
            }
            else {
                results[board][test]['support'] = true
                results[board][test]['build_error_msg'] = output
            }
            echo output
        }
    }
}

/* common test node ========================================================= */
/* Needed to deal with groovy garbage. */
@NonCPS
def mapToList(depmap) {
    def dlist = []
    for (def entry2 in depmap) {
        dlist.add(new java.util.AbstractMap.SimpleImmutableEntry(entry2.key, entry2.value))
    }
    dlist
}

def unstashRobotFWTests() {
    unstash name: "RobotFWTestsRepo"
}

def unstashBinaries(test) {
    unstash name: "${env.BOARD}_${test.replace("/", "_")}"
}

/* Flashes binary to the DUT of the node. */
def flashTest(test)
{
    sh script: "RIOT_CI_BUILD=1 make -C ${test} flash-only", label: "Flash ${test}"
}

/* Does all the things needed for robot tests. */
def rFTest(test)
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

def archiveTestResults(test)
{
    def test_name = test.replaceAll('/', '_')
    def base_dir = "build/robot/${env.BOARD}/${test_name}/"
    archiveArtifacts artifacts: "${base_dir}*.xml,${base_dir}*.html,${base_dir}*.html,${base_dir}includes/*.html",
            allowEmptyArchive: true
    junit testResults: "${base_dir}xunit.xml", allowEmptyResults: true
}

/* Somewhat hacky way of adding info on build results for the tests that
 * were not run. */
def archiveFailedTestResults(test, err_msg)
{
    def test_name = test.replaceAll('/', '_')
    def dir = "build/robot/${env.BOARD}/${test_name}/xunit.xml"
    writeFile file: dir, text: """<?xml version='1.0' encoding='UTF-8'?>
<testsuite errors="0" failures="1" name="${test_name}" skipped="0" tests="1" time="0.000"><testcase classname="${test_name}.build" name="Build" time="0.000"><failure>Build failed</failure></testcase></testsuite>
"""
    archiveArtifacts artifacts: dir
    junit testResults: dir, allowEmptyResults: true
}

/* Somewhat hacky way of adding info on build results for the tests that
 * were not run. */
def archiveSkippedTestResults(test)
{
    def test_name = test.replaceAll('/', '_')
    def dir = "build/robot/${env.BOARD}/${test_name}/xunit.xml"
    writeFile file: dir, text: """<?xml version='1.0' encoding='UTF-8'?>
<testsuite errors="0" failures="0" name="${test_name}" skipped="1" tests="1" time="0.000"><testcase classname="${test_name}.build" name="Build" time="0.000"><skipped>Test not supported</skipped></testcase></testsuite>
"""
    archiveArtifacts artifacts: dir
    junit testResults: dir, allowEmptyResults: true
}

/* Tries to flash and test each test.
 *
 * If a test fails it catches and runs through the next one. Successful tests
 * uploads test artifacts.
 *
 * Required results from the buildJobs.
 */
def flashAndRFTestNodes(results)
{
    catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE', catchInterruptions: false) {
        stage( "${env.BOARD} setup on  ${env.NODE_NAME}"){
            unstashRobotFWTests()
        }
        for (def test in mapToList(results[env.BOARD])) {
            catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE',
                        catchInterruptions: false) {
                if (test.value["support"]) {
                    if (test.value['build']) {
                        stage("${test.key}") {
                            unstashBinaries(test.key)
                            /* No need to reset as flashing and the test should manage
                            * this */
                            flashTest(test.key)
                            rFTest(test.key)
                            archiveTestResults(test.key)
                        }
                    }
                    else {
                        stage("Build failing ${test.key}") {
                            err_msg = test.value["build_error_msg"]
                            archiveFailedTestResults(test.key, err_msg)
                            error("Build failure ${err_msg}")
                        }
                    }
                }
                else {
                    stage("Skipping ${test.key}") {
                        archiveSkippedTestResults(test.key)
                    }
                }
            }
        }
    }
}

def riotTest(test)
{
    def test_name = test.replaceAll('/', '_')
    catchError(buildResult: 'UNSTABLE', stageResult: 'UNSTABLE',
            catchInterruptions: false) {
        sh script: "make -C ${test} test",
                label: "Run ${test} test"
    }
}

/* Tries to flash and test each test.
 *
 * If a test fails it catches and runs through the next one. Successful tests
 * uploads test artifacts.
 *
 * Required results from the buildJobs.
 */
def flashAndRiotTestNodes(results)
{
    catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE', catchInterruptions: false) {
        for (def test in mapToList(results[env.BOARD])) {
            catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE', catchInterruptions: false) {
                if (test.value["support"]) {
                    if (test.value['build']) {
                        stage("${test.key}") {
                            unstashBinaries(test.key)
                            /* No need to reset as flashing and the test should manage
                            * this */
                            flashTest(test.key)
                            riotTest(test.key)
                        }
                    }
                    else {
                        stage("Build failing ${test.key}") {
                            error("Build failure")
                        }
                    }
                }
                else {
                    stage("Skipping ${test.key}") {
                        echo "Skipping due to test not supported"
                    }
                }
            }
        }
    }
}
