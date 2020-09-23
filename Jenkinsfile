def nodes = nodesByLabel('HIL')

pipeline {
    agent { label 'master' }
    options {
        // If the whole process takes more than x hours then exit
        // This must be longer since multiple jobs can be started but waiting on nodes to complete
        timeout(time: 3, unit: 'HOURS')
        // Failing fast allows the nodes to be interrupted as some steps can take a while
        parallelsAlwaysFailFast()
    }
    parameters {
        choice(name: 'HIL_RIOT_VERSION', choices: ['submodule', 'master', 'pull'], description: 'The RIOT branch or PR to test.')
        string(name: 'HIL_RIOT_PULL', defaultValue: '0', description: 'RIOT pull request number')
    }
    stages {
        stage('setup') {
            steps {
                createPipelineTriggers()
                stepClone()
                stash name: 'sources'
            }
        }
        stage('node test') {
            steps {
                runParallel items: nodes.collect { "${it}" }
            }
        }
        stage('Notify') {
            steps {
                emailext (
                    body: '''${SCRIPT, template="groovy-html.template"}''',
                    mimeType: 'text/html',
                    subject: "${currentBuild.fullDisplayName}",
                    from: 'jenkins@riot-ci.inet.haw-hamburg.de',
                    to: '${DEFAULT_RECIPIENTS}',
                    replyTo: '${DEFAULT_RECIPIENTS}'
                )
            }
        }
    }
}

def runParallel(args) {
    parallel args.items.collectEntries { name -> [ "${name}": {

        node (name) {
            stage("${name}") {
                // We want to timeout a node if it doesn't respond
                // The timeout should only start once it is acquired
                timeout(time: 60, unit: 'MINUTES') {
                    script {
                        stepRunNodeTests()
                    }
                }
            }
        }
    }]}
}

void createPipelineTriggers() {
    script {
        def triggers = []
        if (env.BRANCH_NAME == 'nightly') {
            triggers = [parameterizedCron('0 1 * * * % HIL_RIOT_VERSION=master')]
        }
        properties([
            pipelineTriggers(triggers)
        ])
    }
}

def stepClone()
{
    deleteDir()
    checkout scm
    if ("${env.BRANCH_NAME}" == 'nightly') {
        // update nightly branch to latest master and push
        withCredentials([usernamePassword(credentialsId: 'da54a500-472f-4005-9399-a0ab5ce4da7e', passwordVariable: 'GIT_PASSWORD', usernameVariable: 'GIT_USERNAME')]) {
            sh("""
                git config --global credential.username ${GIT_USERNAME}
                git config --global credential.helper "!echo password=${GIT_PASSWORD}; echo"
                git pull --rebase origin master
                git push origin HEAD:nightly
            """)
        }
    }
    if ("${params.HIL_RIOT_VERSION}" == 'master') {
        // checkout latest RIOT master
        sh 'git submodule update --init --remote --rebase --depth 1'
    }
    else {
        sh 'git submodule update --init --depth 1'
        if ("${params.HIL_RIOT_VERSION}" == 'pull' && "${params.HIL_RIOT_PULL}" != '0') {
            // checkout specified PR number
            def prnum = params.HIL_RIOT_PULL.toInteger()
            sh """
                cd RIOT
                git fetch origin +refs/pull/${prnum}/merge
                git checkout FETCH_HEAD
            """
        }
    }
}

def stepRunNodeTests()
{
    catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
        def tests = []
        stage( "${env.BOARD} setup"){
            stepPrepareNodeWorkingDir()
            tests = stepDiscoverTests()
        }
        for (int i=0; i < tests.size(); i++) {
            stage("${tests[i]}") {
                def timeout_stop_exc = null
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE', catchInterruptions: false) {
                    stepPrintEnv()
                    stepReset(tests[i])
                    stepMake(tests[i])
                    stepFlash(tests[i])
                    stepTest(tests[i])
                    stepArchiveTestResults(tests[i])
                }
            }
        }
    }
}

def stepPrepareNodeWorkingDir()
{
    deleteDir()
    unstash name: 'sources'
    sh 'pwd'
    sh 'ls -alh'
}

def stepDiscoverTests() {
    return sh(returnStdout: true,
    script:  """
                for dir in \$(find tests -maxdepth 1 -mindepth 1 -type d); do
                    [ -d \$dir/tests ] && { echo \$dir ; } || true
                done
            """).tokenize()
}

def stepPrintEnv()
{
    sh 'dist/tools/ci/print_environment.sh'
}

def stepReset(test)
{
    sh "python3 -m bph_pal --philip_reset"
    sh "make -C ${test} reset"
}

def stepMake(test)
{
    sh "make -C ${test}"
}

def stepFlash(test)
{
    sh "make -C ${test} flash-only"
}

def stepTest(test)
{
    def test_name = test.replaceAll('/', '_')
    sh "make -C ${test} robot-clean || true"
    // We don't want to stop running other tests since the robot-test is allowed to fail
    catchError(buildResult: 'UNSTABLE', stageResult: 'UNSTABLE', catchInterruptions: false) {
        sh "make -C ${test} robot-test"
    }
}

def stepArchiveTestResults(test)
{
    def test_name = test.replaceAll('/', '_')
    sh "make -C ${test} robot-html || true"
    archiveArtifacts artifacts: "build/robot/${env.BOARD}/${test_name}/*.xml"
    archiveArtifacts artifacts: "build/robot/${env.BOARD}/${test_name}/*.html"
    archiveArtifacts artifacts: "build/robot/${env.BOARD}/${test_name}/includes/*.html"
    junit "build/robot/${env.BOARD}/${test_name}/xunit.xml"
}
