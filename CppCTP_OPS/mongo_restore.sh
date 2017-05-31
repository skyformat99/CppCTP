#!/bin/bash
 
MONGO_DATABASE="CTP"
APP_NAME="xts"

AUTH_NAME=":)xTrader:)admin:)"
AUTH_PASS=":)&xtrader&:)"
AUTH_DATABASE="admin"
MONGO_HOST="127.0.0.1"
MONGO_PORT="27017"
TIMESTAMP=`date +%F-%H%M`
MONGORESTORE_PATH="/usr/bin/mongorestore"
RESTORE_DIR="/root/xts_backup/$APP_NAME"
#BACKUP_NAME="$APP_NAME-$TIMESTAMP"
#恢复数据一定要修改RESTORE_NAME属性,改为xts
RESTORE_NAME="$APP_NAME"
 
$MONGORESTORE_PATH --username $AUTH_NAME --password $AUTH_PASS --authenticationDatabase $AUTH_DATABASE --db $MONGO_DATABASE --drop $RESTORE_DIR/$RESTORE_NAME