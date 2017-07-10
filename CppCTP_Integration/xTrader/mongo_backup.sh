#!/bin/bash
 
MONGO_DATABASE="CTP"
APP_NAME="xts"

MONGO_HOST="127.0.0.1"
MONGO_PORT="27017"
AUTH_NAME=":)xTrader:)admin:)"
AUTH_PASS=":)&xtrader&:)"
AUTH_DATABASE="admin"
TIMESTAMP=`date +%F-%H%M`
MONGODUMP_PATH="/usr/bin/mongodump"
BACKUPS_DIR="/root/xts_backup/$APP_NAME"
BACKUP_NAME="$APP_NAME-$TIMESTAMP"
 
$MONGODUMP_PATH --host $MONGO_HOST --port $MONGO_PORT --username $AUTH_NAME --password $AUTH_PASS --authenticationDatabase $AUTH_DATABASE --db $MONGO_DATABASE
 
mkdir -p $BACKUPS_DIR
mv dump $BACKUP_NAME
tar -zcvf $BACKUPS_DIR/$BACKUP_NAME.tgz $BACKUP_NAME
rm -rf $BACKUP_NAME
