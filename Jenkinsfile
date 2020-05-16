node{
    stage('init'){
			  
			  echo 'Do Checkout'
        
			  checkout([$class: 'GitSCM', 
									branches: [[name: '*/master']], 
									doGenerateSubmoduleConfigurations: false, 
									extensions: [], 
									submoduleCfg: [], 
									userRemoteConfigs: [[credentialsId: 'GitHubWerumingerI', 
																			 url: 'https://github.com/Weruminger/indigo.git']]])
    }
	stage('Build'){
	    sh('make all')
	}
	stage('Stop Services'){
		echo 'TODO stop services'
	}
	stage('Install'){
	    sh('make install')
	}
	stage('Start Services'){
	    echo 'Start Services'
	}
}
