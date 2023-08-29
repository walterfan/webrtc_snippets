# -*- coding: utf-8 -*-
import os

"""
常量
"""

# 下面2个是鉴权信息
APPID = os.getenv("BD_APPID")

APPKEY = os.getenv("BD_APPKEY")

# 语言模型 ， 可以修改为其它语言模型测试，如远场普通话19362
DEV_PID = 15372

# 可以改为wss://
URI = "ws://vop.baidu.com/realtime_asr"
