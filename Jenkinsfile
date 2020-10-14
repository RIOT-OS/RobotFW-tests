/* This file uses both decalritive syntax and scripted.
 * This is because delaritive is simpler and generally preferred but suffer
 * from limited dynamic discovery and implementations.
 *
 * @Note If this Jenkinsfile is touched and merged into master it should also
 * manually be merged into nightly to be executed that night, otherwise it is
 * one night behind.
 */

/* globals ================================================================== */
/* Global variables are decalred without `def` as they must be used in both
 * declaritive and scripted mode */
nodes = nodesByLabel('HIL')
nodeBoards = []
nodeTests = []
rfCommitId = ""
rfUrl = ""
riotCommitId = ""
riotUrl = ""

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
    /* TODO: Confirm escape characters in extra build args */
    parameters {
        choice(name: 'HIL_RIOT_VERSION',
               choices: ['submodule', 'master', 'pull', 'release'],
               description: 'The RIOT branch or PR to test.')
        string(name: 'HIL_RIOT_VERSION_ARG', defaultValue: '0',
               description: 'RIOT pull request number or release version')
        choice(name: 'HIL_RF_VERSION',
               choices: ['current', 'pull', 'master'],
               description: 'The RobotFW-Tests branch or PR to test.')
        string(name: 'HIL_RF_PULL', defaultValue: '0',
               description: 'RobotFW-Tests pull request number')
        string(name: 'HIL_BOARDS', defaultValue: 'all',
               description: 'Comma separated list of boards')
        string(name: 'HIL_TESTS', defaultValue: 'all',
               description: 'Comma separated list of tests')
        string(name: 'PRE_EXTRA_BUILD_ARGS', defaultValue: 'DOCKER_MAKE_ARGS=-j BUILD_IN_DOCKER=1',
               description: 'Additional commands to use before make')
        string(name: 'POST_EXTRA_BUILD_ARGS', defaultValue: '',
               description: 'Additional commands to use after make')
        string(name: 'TEST_COMMAND', defaultValue: 'robot-test',
               description: 'The type of test call in make')
        string(name: 'CLEAN_COMMAND', defaultValue: 'robot-clean',
               description: 'The type of clean call before the test in make')
    }
    stages {
        stage('setup master') {
            steps {
                stepCreatePipelineTriggers()
                stepGetRfUrlAndCommit()
                stepGetRiotUrlAndCommit()
            }
        }
        stage('setup build server and build') {
            agent { label 'riot_build' }
            steps {
                stepPrepareNodeWorkingDir()
                stepUpdateNightly()
                stepGetBoards()
                stepGetTests()
                stepArchiveMetadata()
                stepBuildJobs()
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
    post {
        always {
            emailext (
                body: '''${SCRIPT, template="groovy-html.template"}''',
                mimeType: 'text/html',
                subject: "${currentBuild.fullDisplayName}",
                from: 'jenkins@riot-ci.inet.haw-hamburg.de',
                to: '${DEFAULT_RECIPIENTS}',
                replyTo: '${DEFAULT_RECIPIENTS}'
            )
            stepNotifyOnPR()
        }
    }
}

/* master steps ============================================================= */
/* Creates cron based triggers for nightly builds. */
void stepCreatePipelineTriggers() {
    script {
        def triggers = []
        if (env.BRANCH_NAME == 'nightly') {
            triggers = [parameterizedCron('0 1 * * * % HIL_RIOT_VERSION=master; HIL_RF_VERSION=master')]
        }
        properties([
            pipelineTriggers(triggers)
        ])
    }
}

/* Gets the robot framework url and commit id based on pipeline parameters.
 *
 * Sets rfUrl and rfCommitId.
 */
def stepGetRfUrlAndCommit() {
    (rfUrl, rfCommitId) = getUrlAndCommitFromParams(
            params.HIL_RF_VERSION, "RIOT-OS",
            "RobotFW-Tests", params.HIL_RF_PULL)
    sh script: "echo rfUrl ${rfUrl}"
    sh script: "echo rfCommitId ${rfCommitId}",
            label: "rfCommitId ${rfCommitId}"
}

/* Gets the riot url and commit id based on pipeline parameters.
 *
 * Sets riotUrl and riotCommitId.
 */
def stepGetRiotUrlAndCommit() {
    (riotUrl, riotCommitId) = getUrlAndCommitFromParams(
            params.HIL_RIOT_VERSION, "RIOT-OS",
            "RIOT", params.HIL_RIOT_VERSION_ARG)
    sh script: "echo riotUrl ${riotUrl}"
    sh script: "echo riotCommitId ${riotCommitId}",
            label: "riotCommitId ${riotCommitId}"
}

/* Gets github url and commit based on parameters.
 *
 * @param ver           Type of version to get {master, pull, current, submodule}
 * @param repo_owner    Repoisitory owner
 * @param repo_name     Repoisitory name
 * @param ver_arg       Optional pull request number or relase tag
 *
 * @return (url, commit)
 */
def getUrlAndCommitFromParams(ver, repo_owner, repo_name, ver_arg = 0) {
    if (ver == 'master') {
        sh script: "echo ${repo_name} using master",
            label: "${repo_name} using master"
        return getUrlAndCommitFromDefault(repo_owner, repo_name)
    }
    else if (ver == 'pull') {
        sh script: "echo ${repo_name} using pull request ${ver_arg}",
            label: "echo ${repo_name} using pull request ${ver_arg}"
        assert ver_arg != '0' : "Selected pull of ${repo_name} but no pull entered"
        return getUrlAndCommitFromPr(repo_owner, repo_name, ver_arg)
    }
    else if (ver == 'current') {
        sh script: "echo ${repo_name} using current",
            label: "echo ${repo_name} using current"
        return [env.GIT_URL.take(env.GIT_URL.lastIndexOf('.')),
                env.GIT_COMMIT]
    }
    else if (ver == 'submodule') {
        sh script: "echo ${repo_name} using submodule",
            label: "echo ${repo_name} using submodule"
        return getUrlAndCommitOfSubmodule("RIOT-OS", "RobotFW-Tests")
    }
    else if (ver == 'release') {
        sh script: "echo ${repo_name} using release ${ver_arg}",
            label: "echo ${repo_name} using release ${ver_arg}"
        return getUrlAndCommitFromTag(repo_owner, repo_name, ver_arg)
    }
    assert false : "No valid options to get commit and url of ${repo_name}"
}

/* Gets the SHA1 commit id and url of a github default branch.
 *
 * @param repo_owner    The owner of the repo.
 * @param repo_name     The name of the repo.
 *
 * @return              (url, commit)
 */
def getUrlAndCommitFromDefault(repo_owner, repo_name) {
    def query = "-X POST -d \"{\\\"query\\\": \\\"query "
    query = "${query}{repository(name: \\\\\\\"${repo_name}\\\\\\\","
    query = "${query}owner: \\\\\\\"${repo_owner}\\\\\\\") "
    query = "${query}{defaultBranchRef "
    query = "${query}{ target { oid } }}}\\\"}\" "
    query = "${query}https://api.github.com/graphql"
    def jsonObj = queryGithubApi(query)
    return ["https://github.com/${repo_owner}/${repo_name}",
            jsonObj.defaultBranchRef.target.oid]
}

/* Gets the SHA1 commit id and url of a github based pull request.
 *
 * @param repo_owner    The owner of the repo.
 * @param repo_name     The name of the repo that contains the pull request.
 * @param pr_num        The pull request number.
 *
 * @return              [url, commit_id]
 */
def getUrlAndCommitFromPr(repo_owner, repo_name, pr_num) {
    /* Queries depend on the github API v4. To explore the API use:
       https://developer.github.com/v4/explorer/
     */
    /* Escaping in groovy with jenkins is equivalent to the shit from the
     * illegitimate child of a pile of elephant puke and the still living
     * cancer from Mussolini. It is horrible...
     * After about 161 attempts I will say:
     * - One must use 'single quotes' for credentials as the TOKEN should be
     * evaluated in the NODE
     * - Escaping is different from the sh script and a variable
     * - If one wants to use (parentheses) with sh, one must escape with \\(\\),
     * this does not apply in variables
     * - Variables require the expected \ escape characters
     * - Be careful with what is printed in the output as it doesn't reflect
     * what is actually called all the time, sometimes an echo will not show
     * quotes even though they are there...
     * - It seems like echos show up in label if a url is included, if not
     * then the label is Print Message and echo data must be expanded
     * - More info is available: https://gist.github.com/Faheetah/e11bd0315c34ed32e681616e41279ef4
     *
     * I wouldn't wish this kind of work on my worst enemy... which right now
     * seems to be me.
     */
    def query = "-X POST -d \"{\\\"query\\\": \\\"query "
    query = "${query}{repository(name: \\\\\\\"${repo_name}\\\\\\\","
    query = "${query}owner: \\\\\\\"${repo_owner}\\\\\\\") "
    query = "${query}{pullRequest(number: ${pr_num}) "
    query = "${query}{ headRefOid headRepository { url }}}}\\\"}\" "
    query = "${query}https://api.github.com/graphql"
    def jsonObj = queryGithubApi(query)
    return [jsonObj.pullRequest.headRepository.url,
            jsonObj.pullRequest.headRefOid]
}

/* Gets the SHA1 commit id and url of the first subrepo in the repo.
 *
 * @param repo_owner    The owner of the repo.
 * @param repo_name     The name of the repo that contains the subrepo.
 *
 * @return              [url, commit_id]
 */
def getUrlAndCommitOfSubmodule(repo_owner, repo_name) {
    def query = "-X POST -d \"{\\\"query\\\": \\\"query "
    query = "${query}{repository(name: \\\\\\\"${repo_name}\\\\\\\","
    query = "${query}owner: \\\\\\\"${repo_owner}\\\\\\\") "
    query = "${query}{ submodules(first: 1) "
    query = "${query}{ edges { node {subprojectCommitOid gitUrl}}}}}\\\"}\" "
    query = "${query}https://api.github.com/graphql"

    def jsonObj = queryGithubApi(query)
    return [jsonObj.submodules.edges[0].node.gitUrl,
            jsonObj.submodules.edges[0].node.subprojectCommitOid]
}

/* Gets the SHA1 commit id and url of a github release.
 *
 * @param repo_owner    The owner of the repo.
 * @param repo_name     The name of the repo.
 * @param tag           The release tag.
 *
 * @return              (url, commit)
 */
def getUrlAndCommitFromTag(repo_owner, repo_name, tag) {
    def query = "-X POST -d \"{\\\"query\\\": \\\"query "
    query = "${query}{repository(name: \\\\\\\"${repo_name}\\\\\\\","
    query = "${query}owner: \\\\\\\"${repo_owner}\\\\\\\") "
    query = "${query}{release (tagName: \\\\\\\"${tag}\\\\\\\")"
    query = "${query}{tag{target{oid}}}}}\\\"}\" "
    query = "${query}https://api.github.com/graphql"
    def jsonObj = queryGithubApi(query)
    return ["https://github.com/${repo_owner}/${repo_name}",
            jsonObj.release.tag.target.oid]
}

/* Helper function for adding credentials for the github api call.
 *
 * @param query     The post authentication string for the api call
 *
 * @return  json object with api query result
 */
def queryGithubApi(query) {
    def res = ""
    withCredentials([string(credentialsId: 'github', variable: 'TOKEN')]) {
        res = sh script: '''
            curl -H "Authorization: token $TOKEN" ''' + query,
        label: "Query github api", returnStdout: true
    }
    def jsonObj = readJSON text: res
    return jsonObj.data.repository
}

/* Runs a script to compile all tests results in the archive. */
def stepCompileResults()
{
    /* Some hacks are needed since the master must run the script on the
     * archive but there is not simple way of finding the location of the
     * archive. The best way is to take the env vars and parse them to
     * fit the path. The branch name has some kind of hash at the end so an ls
     * and grep should return whatever the directory name is.
     * There is an assumption that the grep will only find one result
     */
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

def stepNotifyOnPR() {
    if ("${params.HIL_RF_VERSION}" == 'pull') {
        def newline = "\\n"
        def body = "## HiL CI Test Results${newline}"
        body = "${body}${newline}"
        body = "${body}Boards Tested: `${nodeBoards}`${newline}"
        body = "${body}Tests Run: `${nodeTests}`${newline}"
        body = "${body}${newline}"
        body = "${body}[Full test results](${RUN_DISPLAY_URL})${newline}"
        def query = "-X POST -d '{\"body\": \"${body}\"}' "
        query = "${query}\"https://api.github.com/repos/RIOT-OS/RobotFW-Tests/issues/"
        query = "${query}${params.HIL_RF_PULL}/comments\""
        withCredentials([string(credentialsId: 'github', variable: 'TOKEN')]) {
            sh script: '''
                curl -H "Authorization: token $TOKEN" ''' + query,
            label: "Query github api"
        }
    }
}

/* node steps =============================================================== */
/* Cleans and clones both RobotFW-Tests and RIOT based on rfUrl,
 * rfCommitId, riotUrl, and riotCommitId in the node workspace.
 */
def stepPrepareNodeWorkingDir()
{
    /* TODO: Check if the delete is needed */
    deleteDir()

    exit_code = sh script: """
                sh ${GIT_CACHE_PATH}git-cache clone ${rfUrl} ${rfCommitId} .
                sh ${GIT_CACHE_PATH}git-cache clone ${riotUrl} ${riotCommitId} RIOT
            """,
            label: "checkout from git cache",
            returnStatus: true

    if (exit_code != 0) {
        /* If git cache failed then fallback to cloning */
        stepClone()
    }
}

/* Legacy code, should be removed with git-cache dependency. */
def stepClone()
{
    deleteDir()
    checkout scm

    if ("${params.HIL_RIOT_VERSION}" == 'master') {
        sh script: 'git submodule update --init --remote --rebase --depth 1',
                label: "checkout latest RIOT master"
    }
    else {
        sh script: 'git submodule update --init --depth 1', label: "update RIOT submodule"
        if ("${params.HIL_RIOT_VERSION}" == 'pull' && "${params.HIL_RIOT_VERSION_ARG}" != '0') {
            // checkout specified PR number
            def prnum = params.HIL_RIOT_VERSION_ARG.toInteger()
            sh script: """
                cd RIOT
                git fetch origin +refs/pull/${prnum}/merge
                git checkout FETCH_HEAD
            """, label: "checkout RIOT PR ${prnum}"
        }
    }
}

/* Legacy code, updates nightly with master. */
def stepUpdateNightly() {
    if ("${env.BRANCH_NAME}" == 'nightly' &&
        "${params.HIL_RIOT_VERSION}" == 'master' &&
        "${params.HIL_RF_VERSION}" == 'master') {
        def push_cmd = "git push ${rfUrl} HEAD:nightly -f --force-with-lease"
        withCredentials([string(credentialsId: 'github', variable: 'TOKEN')]) {
        sh script: '''
            git config --global credential.username $GIT_USERNAME
            git config --global credential.helper "!echo password=$TOKEN; echo"
            git add RIOT
            git commit -m "Nightly update of RIOT"

        ''' + push_cmd, label: "Update robot nightly branch"
        }
    }
}

/* Gets boards to test based on parameters or dynamically all available
 * boards connected to jenkins nodes.
 *
 * Sets nodeBoards
 */
def stepGetBoards() {
    if (params.HIL_BOARDS == 'all') {
        nodeBoards = getBoardsFromNodesEnv()
    }
    else {
        nodeBoards = params.HIL_BOARDS.tokenize(',')
        /* TODO: Validate if the boards are connected */
    }
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
    if (params.HIL_TESTS == 'all') {
        nodeTests = getTestsFromDir()
    }
    else {
        nodeTests = params.HIL_TESTS.tokenize(',')
    }

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
        for (int t_idx=0; t_idx < nodeTests.size(); t_idx++) {
            for (int b_idx=0; b_idx < nodeBoards.size(); b_idx++) {
                buildJob(nodeBoards[b_idx], nodeTests[t_idx])
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
    exit_code = sh script: "${params.PRE_EXTRA_BUILD_ARGS} BOARD=${board} make -C ${test} clean all ${params.POST_EXTRA_BUILD_ARGS}",
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
            stepPrepareNodeWorkingDir()
            stepPrintEnv()
        }
        for (int i=0; i < nodeTests.size(); i++) {
            stage("${nodeTests[i]}") {
                def timeout_stop_exc = null
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

/* Prints the useful env to help understand the test conditions. */
def stepPrintEnv()
{
    sh script: 'dist/tools/ci/print_environment.sh', label: "Print environment"
}


/* Unstashes the binaries from the build server. */
def stepUnstashBinaries(test) {
    unstash name: "${env.BOARD}_${test.replace("/", "_")}"
}

/* Flashes binary to the DUT of the node. */
def stepFlash(test)
{
    sh script: "make -C ${test} flash-only", label: "Flash ${test}"
}

/* Cleans the robot test directory and runs the robot tests. */
def stepTest(test)
{
    def test_name = test.replaceAll('/', '_')
    if (params.CLEAN_COMMAND != "") {
        sh script: "make -C ${test} ${params.CLEAN_COMMAND} || true",
                label: "Cleaning before ${test} test"
    }
    /* We don't want to stop running other tests since the robot-test is
     * allowed to fail */
    catchError(buildResult: 'UNSTABLE', stageResult: 'UNSTABLE',
            catchInterruptions: false) {
        sh script: "make -C ${test} ${params.TEST_COMMAND}",
                label: "Run ${test} test"
    }
}

/* Archives the test results. */
def stepArchiveTestResults(test)
{
    def test_name = test.replaceAll('/', '_')
    sh script: "make -C ${test} robot-html || true",
            label: "Attempt to build html from robot-results"
    def base_dir = "build/robot/${env.BOARD}/${test_name}/"
    archiveArtifacts artifacts: "${base_dir}*.xml,${base_dir}*.html,${base_dir}*.html,${base_dir}includes/*.html",
            allowEmptyArchive: true
    junit testResults: "${base_dir}xunit.xml", allowEmptyResults: true
}

/* tests ==================================================================== */
def testPrintPRInfo() {
    (url, commit) = getUrlAndCommitFromParams('pull', "RIOT-OS", "RIOT", 15145)
    echo url
    echo commit

    (url, commit) = getUrlAndCommitFromParams('pull', "RIOT-OS", "RobotFW-Tests", 82)
    echo url
    echo commit

    (url, commit) = getUrlAndCommitFromParams('master', "RIOT-OS", "RIOT")
    echo url
    echo commit

    (url, commit) = getUrlAndCommitFromParams('master', "RIOT-OS", "RobotFW-Tests")
    echo url
    echo commit

    (url, commit) = getUrlAndCommitFromParams('current', "RIOT-OS", "RobotFW-Tests")
    echo url
    echo commit

    (url, commit) = getUrlAndCommitFromParams('submodule', "RIOT-OS", "RIOT")
    echo url
    echo commit
}
