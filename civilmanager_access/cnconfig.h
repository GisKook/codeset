#ifndef CODESET_CNCONFIG_H_H
#define CODESET_CNCONFIG_H_H

#define DBHOST "dbHost"
#define DBPORT "dbPort"
#define DBNAME "dbName"
#define DBUSER "dbUser"
#define DBPWD "dbPassword"
#define ZMQRECVADDR "zmqRecvaddr"
#define ZMQBINDADDR "zmqBindaddr"
#define BINDPORT "bindPort"
#define DUMP "dump"
#define TIMEOUT "timeout" 

int cnconfig_loadfile( const char* filename ); 
const char* cnconfig_getvalue( const char * key );

#endif
