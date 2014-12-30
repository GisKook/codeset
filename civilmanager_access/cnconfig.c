#include <assert.h>
#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "cnconfig.h"

using namespace std;

map<string, string>cnconfig_map;

int cnconfig_setconfig(cJSON* json, const char * configkey, const char * errormsg){
	cJSON* val = cJSON_GetObjectItem(json, configkey);
	if(!val||val->type!=cJSON_String||!val->valuestring) {
		fprintf(stderr,errormsg);
		cJSON_Delete(json);
		return -1;
	}else{ 
		cnconfig_map[configkey]=string(val->valuestring);
	}
	return 0;
}

int cnconfig_loadfile( const char* filename ){ 
	FILE *f=fopen(filename,"rb");
	if (f==NULL) {
		fprintf(stderr,"there is no config file! %s \n", filename);
		return -1;
	}
	fseek(f,0,SEEK_END);long len=ftell(f);fseek(f,0,SEEK_SET);
	char *data=(char*)malloc(len+1);fread(data,1,len,f);fclose(f);
	cJSON* json = cJSON_Parse(data);
	if (!json) {
		fprintf(stderr,"Error before: [%s]\n",cJSON_GetErrorPtr());
	}else{ 
	//	int count = cJSON_GetArraySize(json);
		if( 0 != cnconfig_setconfig(json, DBHOST, "please check config file: dbHost is not set!\n")||
			0 != cnconfig_setconfig(json, DBPORT, "please check config file: dbPort is not set!\n")||
			0 != cnconfig_setconfig(json, DBNAME, "please check config file: dbName is not set!\n")||
			0 != cnconfig_setconfig(json, DBUSER, "please check config file: dbUser is not set!\n")||
			0 != cnconfig_setconfig(json, DBPWD, "please check config file: dbPassword is not set!\n")||
			0 != cnconfig_setconfig(json, ZMQRECVADDR, "please check config file: zmqRecvaddr is not set!\n")||
			0 != cnconfig_setconfig(json, ZMQBINDADDR, "please check config file: zmqBindAddr is not set!\n")||
			0 != cnconfig_setconfig(json, BINDPORT, "please check config file: bindPort is not set!\n")){
			return -1;
		}

	}
	cJSON_Delete(json);
	free(data);
	data = NULL;

	return  0;
} 

const char* cnconfig_getvalue( const char * key ){
	return cnconfig_map[string(key)].c_str();
}

