node{
    stage('init'){
			
        
			  checkout([$class: 'GitSCM', 
									branches: [[name: '*/master']], 
									doGenerateSubmoduleConfigurations: false, 
									extensions: [], 
									submoduleCfg: [], 
									userRemoteConfigs: [[credentialsId: 'GitHubWerumingerI', 
																			 url: 'https://github.com/Weruminger/indigo.git']]])
        echo 'hello World'
    }
}
