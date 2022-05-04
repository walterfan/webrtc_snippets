# Overview

* Usage: `fab -l`




# Reference

* [Foundations of Python Network Programming](https://github.com/brandon-rhodes/fopnp)
* [PEP-466](http://legacy.python.org/dev/peps/pep-0466/)


# Example

```
fab tls-server
execute: ./tls_test.py -t server --host localhost --port 5004 --ce=localhost.pem

fab tls-client
execute: ./tls_test.py -t client --host localhost --port 5004 --ca=ca.crt
```


## datachannel example

* https://github.com/aiortc/aiortc/blob/main/examples/datachannel-filexfer/filexfer.py