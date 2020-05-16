

node{
properties([
  parameters([
    booleanParam(name: 'MERGE_UPSTREAM', defaultValue: false, description: '' )
		booleanParam(name: 'DO_BUILD', defaultValue: false, description: '' )
		boolenParam(name: 'DO_INSTALL', defaultValue: false, description: '' )
		string(name: 'BRANCH_TO_FETCH', defaultValue: 'master', description: '' )
   ])
])
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
		if ("${env.DO_BUILD}" == 'true' ){
	    sh('make all')
		}
	}
	stage('Stop Services'){
		if ("${env.DO_INSTALL}" == 'true' ){
 		   echo 'TODO stop services'
		   sh('sudo systemctl indiwebmanager stop')
		}
	}
	stage('Install'){
		if ("${env.DO_INSTALL}" == 'true' ){
	    sh('sudo make install')
		}
	}
	stage('Start Services'){
		if ("${env.DO_INSTALL}" == 'true' ){
	    echo 'Start Services'
		  su('sudo systemctl indiwebservice start')
		}
	}
}
