## 简介

实时语音识别api python demo



## 系统要求

* Set environment variables 

```
touch setenv.sh
----------------------------------------
export BD_APPID=xxx
export BD_APPKEY="xxx"


export XF_APPID=xxx
export XF_APISecret="xxx"
export XF_APISecret="xxx"

```

* 安裝依賴
```
virtualenv -p python3 venv
source venv/bin/activate
pip install pip -U
pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
pip install -r requirement.txt
source setenv.sh
```




## 测试流程

1) modify setenv.sh:

* Baidu API, 修改 BD_APPID  和 BD_APPKEY 为你网页上申请有实时语音识别api权限的应用鉴权信息

* XunFei API, 修改 XF_APPID， XF_APISecret 和 XF_APISecret 为你网页上申请有实时语音识别api权限的应用鉴权信息

2) run realtime_asr.py based on baidu api

2) run iat_pcm_16k.py based on xunfei api


