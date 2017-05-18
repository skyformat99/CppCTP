#!/bin/bash
 
MONGO_DATABASE="CTP"
APP_NAME="xts"

MONGO_HOST="127.0.0.1"
MONGO_PORT="27017"
TIMESTAMP=`date +%F-%H%M`
MONGORESTORE_PATH="/usr/bin/mongorestore"
RESTORE_DIR="/root/xts_backup/$APP_NAME"
#BACKUP_NAME="$APP_NAME-$TIMESTAMP"
#恢复数据一定要修改RESTORE_NAME属性
RESTORE_NAME="$APP_NAME-$TIMESTAMP"
 
# mongo admin --eval "printjson(db.fsyncLock())"
# $MONGODUMP_PATH -h $MONGO_HOST:$MONGO_PORT -d $MONGO_DATABASE
$MONGORESTORE_PATH -d $MONGO_DATABASE --drop $RESTORE_DIR/$RESTORE_NAME
# mongo admin --eval "printjson(db.fsyncUnlock())"