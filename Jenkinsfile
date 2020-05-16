node{
    stage('init'){
			
        
			  checkout([$class: 'GitSCM', 
									branches: [[name: '*/master']], 
									doGenerateSubmoduleConfigurations: false, 
									extensions: [], 
									submoduleCfg: [], 
									userRemoteConfigs: [[credentialsId: 'GitHubWerumingerI', 
																			 url: '']]])
			
			
        echo 'hello World'
    }
}
