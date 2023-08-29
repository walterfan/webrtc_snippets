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
export XF_APIKey="xxx"

```

* 安裝依賴
```
virtualenv -p python3 venv
source venv/bin/activate
python -m pip install --upgrade pip
pip install -r requirement.txt -i http://mirrors.aliyun.com/pypi/simple/ --trusted-host mirrors.aliyun.com
source setenv.sh
```

## 测试流程
修改const.py, APPID 和APPKEY为你网页上申请有实时语音识别api权限的应用鉴权信息：

```python



# 下面2个是鉴权信息
APPID = 1000000

APPKEY = "g8eBUMSxxxxxxxYviL"

```

运行 python 

## 其它参数
```python
# 语言模型 ， 可以修改为其它语言模型测试，如远场普通话19362
DEV_PID = 15372
```


