properties([
  parameters([
		choice(name: 'MERGE_UPSTREAM', choices: ['false','true'], description: '' ),
		choice(name: 'DO_BUILD', choices: ['true','false'], description: '' ),
		choice(name: 'DO_INSTALL', choices: ['true','false'], description: '' ),
		string(name: 'BRANCH_TO_FETCH', defaultValue: 'master', description: '' )
   ])
])

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
		if ("${env.DO_BUILD}" == 'true' ){
	    sh('make all')
		}
	}
	stage('Stop Services'){
		if ("${env.DO_INSTALL}" == 'true' ){
 		   echo 'TODO stop services'
		   try{
		       sh('sudo systemctl stop indigo-server')
		   }catch(exc){}
		   try{
		   
	    	   sh('sudo systemctl stop indigo')
		   }catch(exc){}
		   try{
    		   sh('sudo systemctl stop indigo-environment')
		   }catch(exc){}
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
		  sh('sudo systemctl start indigo-environment')
		  sh('sudo systemctl start indigo')
		  sh('sudo systemctl start indigo-server')
		}
	}
}