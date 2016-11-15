class Info():
    def __init__(self, exchange_name, title='', pubtime='', link='', content='' , isread=0):
        self.title = title
        self.pubtime = pubtime
        self.link = link
        self.content = content
        self.isread = isread
        self.exchange_name = exchange_name

    def setTitle(self, title):
        self.title = title

    def getTitle(self):
        return self.title

    def setCatchTime(self, catchtime):
        self.catchtime = catchtime

    def getCatchTime(self):
        return self.catchtime

    def setPubtime(self, pubtime):
        self.pubtime = pubtime

    def getPubtime(self):
        return self.pubtime

    def setLink(self, link):
        self.link = link

    def getLink(self):
        return self.link

    def setContent(self, content):
        self.content = content

    def getContent(self):
        return self.content

    def setIsread(self, isread):
        self.isread = isread

    def getIsread(self):
        return self.isread

    def setExchangeName(self, exchange_name):
        self.exchange_name = exchange_name

    def getExchangeName(self):
        return self.exchange_name

    def getAllInfo(self):
        print("title = %s" % self.title)
        print("pubtime = %s" % self.pubtime)
        print("link = %s" % self.link)
        print("content = %s" % self.content)
        print("isread = %s" % self.isread)
        print("exchange_name = %s" % self.exchange_name)

if __name__ == '__main__':
    info = Info()
    info.setTitle('hello')
    info.setPubtime('2013')
    info.getAllInfo()