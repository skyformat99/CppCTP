from urllib.request import urlopen
from urllib.error import HTTPError
from bs4 import BeautifulSoup
import time, datetime
from Info import Info
from PipeLine import PipeLine

class InfoParser():
    def __init__(self, pipeline):
        self.pipeline = pipeline
        self.catchcount = time.mktime(datetime.datetime.now().timetuple())

    def SH_getPageLink(self, url):
        stop_flag = False
        try:
            html = urlopen(url)
        except HTTPError as e:
            print("网络有故障!")
            return
        else:
            if html is None:
                print("请求链接服务端无法处理哦!")
            else:
                try:
                    bsObj = BeautifulSoup(html.read(), "html.parser")
                    o_li = bsObj.find("div", {"class": {"lawbox"}}).findAll("li")
                    for item in o_li:
                        span_item = item.find("span")
                        cmp_str_time = span_item.get_text()[1:-1]
                        if (self.timecompare(cmp_str_time, 10)):
                            item_url = "http://www.shfe.com.cn" + item.find("a").attrs["href"]
                            content = self.SH_getItemLink(item_url)

                            '''获取的内容进行对象保存'''
                            info_item = Info('SH')
                            info_item.setTitle(item.find("a").get_text())
                            info_item.setLink(item_url)
                            info_item.setPubtime(cmp_str_time)
                            info_item.setCatchTime(self.catchcount)
                            if content == None:
                                info_item.setContent("公告内容为空")
                            else:
                                info_item.setContent(content)

                            self.pipeline.saveInfo(info_item)

                        else:
                            stop_flag = True
                    if (stop_flag == True):
                        self.catchcount += 1
                        return
                    else:
                        page_item = bsObj.find("div", {"class": {"page-no"}}).findAll("a")[2]
                        next_page_url = "http://www.shfe.com.cn/news/notice/" + page_item.attrs["href"]
                        # print("next_page_url %s" % next_page_url)
                        self.SH_getPageLink(next_page_url)
                except AttributeError as e:
                    print("BS解析出错啦!")
        pass

    def SH_getItemLink(self, url):
        try:
            html = urlopen(url)
        except HTTPError as e:
            print("网络有故障!")
            return None
        else:
            if html is None:
                print("请求链接服务端无法处理哦!")
                return None
            else:
                try:
                    bsObj = BeautifulSoup(html.read(), "html.parser")
                    content = bsObj.find("div", {"class": {"article-detail-text"}})
                    # print("公告内容 %s" % str(content))
                    return str(content)
                except AttributeError as e:
                    print("BS解析出错啦!")
                    return None


    def DL_getPageLink(self, url):
        stop_flag = False
        try:
            html = urlopen(url)
        except HTTPError as e:
            print("网络有故障!")
            return
        else:
            if html is None:
                print("请求链接服务端无法处理哦!")
            else:
                try:
                    # print("bsObj in DL_getPageLink")
                    bsObj = BeautifulSoup(html.read(), "html.parser")
                    o_li = bsObj.findAll("div", {"class": {"portlet"}})[1].findAll("li")
                    # print(o_li)
                    for item in o_li:
                        span_item = item.find("span")
                        # cmp_str_time = span_item.get_text()[1:-1]
                        cmp_str_time = span_item.get_text()
                        # print("cmp_str_time = %s" % cmp_str_time)
                        if (self.timecompare(cmp_str_time, 10)):
                            item_url = "http://www.dce.com.cn" + item.find("a").attrs["href"]
                            # print("item_url = %s" % item_url)
                            content = self.DL_getItemLink(item_url)

                            '''获取的内容进行对象保存'''
                            info_item = Info('DL')
                            info_item.setTitle(item.find("a").get_text())
                            info_item.setLink(item_url)
                            info_item.setPubtime(cmp_str_time)
                            info_item.setCatchTime(self.catchcount)
                            if content == None:
                                info_item.setContent("公告内容为空")
                            else:
                                info_item.setContent(content)

                            self.pipeline.saveInfo(info_item)

                        else:
                            stop_flag = True
                    if (stop_flag == True):
                        self.catchcount += 1
                        return
                    else:
                        page_item = bsObj.find("div", {"class": {"pagination"}}).findAll("a")[2]
                        next_page_url = "http://www.dce.com.cn" + page_item.attrs["tagname"]
                        print("next_page_url %s" % next_page_url)
                        self.DL_getPageLink(next_page_url)
                except AttributeError as e:
                    print("BS解析出错啦!")
        pass

    def DL_getItemLink(self, url):
        try:
            html = urlopen(url)
        except HTTPError as e:
            print("网络有故障!")
            return None
        else:
            if html is None:
                print("请求链接服务端无法处理哦!")
                return None
            else:
                try:
                    bsObj = BeautifulSoup(html.read(), "html.parser")
                    content = bsObj.find("div", {"class": {"detail_inner"}})
                    # print("公告内容 %s" % str(content))
                    return str(content)
                except AttributeError as e:
                    print("BS解析出错啦!")
                    return None



    def timecompare(self, cmp_time_str, iday):
        #今天时间
        today = datetime.date.today()
        str_today = time.strftime("%Y-%m-%d", today.timetuple())

        # 用今天日期减掉时间差，参数为1天，获得昨天的日期
        yesterday = today - datetime.timedelta(days = iday)
        str_yesterday = time.strftime("%Y-%m-%d", yesterday.timetuple())

        # print("今天是%s, 昨天是%s" % (str_today, str_yesterday))

        yesterdayArray = time.strptime(str_yesterday, "%Y-%m-%d")
        yesterdayTimeStamp = int(time.mktime(yesterdayArray))


        cmp_Time_Array = time.strptime(cmp_time_str, "%Y-%m-%d")
        cmpTimeTimeStamp = int(time.mktime(cmp_Time_Array))

        if (cmpTimeTimeStamp >= yesterdayTimeStamp):
            return True
        else:
            return False



if __name__ == '__main__':
    sh_url = "http://www.shfe.com.cn/news/notice/"
    dl_url = "http://www.dce.com.cn/dalianshangpin/yw/fw/jystz/ywtz/index.html"
    pl = PipeLine()
    spider = InfoParser(pl)
    # spider.SH_getPageLink(sh_url)
    spider.DL_getPageLink(dl_url)