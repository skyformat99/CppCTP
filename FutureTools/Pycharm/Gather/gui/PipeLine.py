from Info import Info
import sqlite3

class PipeLine():

    def __init__(self, parent=None):
        self.conn = sqlite3.connect('data.db')
        # print("成功打开数据库")
        #sql_del="DROP TABLE IF EXISTS tbl_test;"
        try:
            self.conn.execute('''
                              CREATE TABLE IF NOT EXISTS INFODATA
                              (ID INTEGER PRIMARY KEY AUTOINCREMENT,
                              TITLE CHAR(300) NOT NULL,
                              CATCHTIME REAL NOT NULL,
                              PUBTIME CHAR(50) NOT NULL,
                              LINK CHAR(300) NOT NULL,
                              CONTENT TEXT NOT NULL,
                              ISREAD INT NOT NULL,
                              EXCHANGENAME CHAR(50) NOT NULL);''')
        except:
            print("表创建失败!")
        self.dbCount = 0

    def set_SH_FutureTab(self, ftab):
        self.sh_f_tab = ftab

    def set_DL_FutureTab(self, ftab):
        self.dl_f_tab = ftab

    def set_ZZ_FutureTab(self, ftab):
        self.zz_f_tab = ftab

    def set_DL_FutureTab(self, ftab):
        self.dl_f_tab = ftab


    def saveInfo(self, obj):
        if (self.countInfo(obj.getLink()) <= 0):
            sql_statement = "INSERT INTO INFODATA(TITLE,CATCHTIME,PUBTIME,LINK,CONTENT,ISREAD,EXCHANGENAME) VALUES('" + obj.getTitle() + "'," + str(obj.getCatchTime()) + ",'" + obj.getPubtime() + "','" + obj.getLink() + "','" + obj.getContent() + "'," + str(obj.getIsread()) + ",'" + obj.getExchangeName() + "')"
            self.conn.execute(sql_statement)
            self.conn.commit()
        else:
            # print("表中已经有数据!")
            pass

    def countInfo(self, link):
        sql_statement = "SELECT count(*) FROM INFODATA where link='" + link + "'"
        num = self.conn.execute(sql_statement)
        count = num.fetchone()[0]
        return count

    def countNum(self):
        from ToolWindow import MainWindow
        # print("current dbcount = %d" % self.dbCount)
        sql_statement = "SELECT count(*) FROM INFODATA"
        num = self.conn.execute(sql_statement)
        count = num.fetchone()[0]
        # print("countNum is %d" % count)
        if (count != self.dbCount):
            self.mid.getPipeLine().getInfo('SH')
            self.mid.getPipeLine().getInfo('DL')
            self.mid.setIs_Check_Info(False)
            self.mid.showTrayMessage()
        self.dbCount = count
        return count

    def getInfo(self, exchangename, pageNumber = 0, pagesize = 20):
        #sql_statement = "SELECT * FROM INFODATA WHERE EXCHANGENAME= '"+ exchangename +"' limit " + str(pageNumber * pagesize -1) + ", " + str(pagesize)
        sql_statement = "SELECT * FROM INFODATA  WHERE EXCHANGENAME= '"+ exchangename +"' ORDER BY CATCHTIME DESC, id ASC limit " + str(pageNumber * pagesize -1) + ", " + str(pagesize)
        cursor = self.conn.execute(sql_statement)
        rowlist = []
        for row in cursor:
            dict_row = {}
            dict_row['id'] = row[0]
            dict_row['title'] = row[1]
            dict_row['pubtime'] = row[3]
            dict_row['link'] = row[4]
            dict_row['content'] = row[5]
            dict_row['isread'] = row[6]
            dict_row['exchangename'] = row[7]
            rowlist.append(dict_row)

        if (exchangename == 'SH'):
            from FutureTab import FutureTab
            self.sh_f_tab.set_init_data(rowlist, self)
        elif (exchangename == 'DL'):
            from FutureTab import FutureTab
            self.dl_f_tab.set_init_data(rowlist, self)

    def updateInfo(self, id, isread):
        sql_statement = "update INFODATA set isread = " + str(isread) + " where ID =" + str(id)
        # print(sql_statement)
        self.conn.execute(sql_statement)
        self.conn.commit()

    def closeDB(self):
        # print("关闭数据库")
        self.conn.close()


    def saveItem(obj_info):

        pass

    def setMainWindow(self, mid):
        self.mid = mid


if __name__ == '__main__':
    obj = PipeLine()
    # obj.countInfo("http://www.shfe.com.cn/news/notice/911326216.html")
    obj.getInfo('SH')