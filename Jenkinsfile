def nodes = nodesByLabel('HIL')
def boards = []
def tests = []

// function to return steps per board
def parallelSteps (board, test) {
    return {
        catchError {
            node (board) {
                def test_name = test.replaceAll('/', '_')
                echo "DIR: ${test}, APP: ${test_name}"
                checkout scm
                sh 'git submodule update --init'
                sh "make -C ${test} flash"
                sh "set +e; make -C ${test} robot-test; set -e"
                archiveArtifacts artifacts: "build/robot/${board}/${test_name}/*.xml"
                junit "build/robot/${board}/${test_name}/xunit.xml"
            }
        }
    }
}

// detect connected boards and available tests
stage ("setup") {
    node ("master") {
        checkout scm
        // discover test applications
        tests = sh(returnStdout: true, script: 'find tests/ -maxdepth 1 -mindepth 1 -type d').tokenize()
        echo "run TESTS: " + tests.join(",")
        // discover available boards
        for (int i=0; i<nodes.size(); ++i) {
            def nodeName = nodes[i];
            node (nodeName) {
                boards.push(env.BOARD)
            }
        }
        boards.unique()
        echo "use BOARDS: " + boards.join(",")
    }
}

// create a stage per test with one step per board
for(int i=0; i < tests.size(); i++) {
    test = tests[i].trim()
    stage(test) {
        parallel (
            boards.collectEntries {
                ["${it}" : parallelSteps(it, test)]
            }
        )
    }
}

stage('Notify') {
    node("master") {
        def jobName = currentBuild.fullDisplayName
        emailext (
            body: '''${SCRIPT, template="groovy-html.template"}''',
            mimeType: 'text/html',
            subject: "${jobName}",
            from: 'jenkins@riot-ci.inet.haw-hamburg.de',
            to: '${DEFAULT_RECIPIENTS}',
            replyTo: '${DEFAULT_RECIPIENTS}'
        )
    }
}
