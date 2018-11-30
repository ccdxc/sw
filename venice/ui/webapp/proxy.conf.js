
const PROXY_CONFIG =
  {
    "/login": {
      "target": "http://192.168.69.189:10001/v1/login",
        "secure": false,
          "pathRewrite": {
        "^/login": ""
      },
      "changeOrigin": false,
        "logLevel": "debug"
    },
   "/configs/**/watch/**/*": {
      "target": "ws://192.168.69.189:10001/",
      "secure": false,
        "changeOrigin": false,
      "ws": true,
      "logLevel": "debug",
      onProxyReq: (proxyReq, req, res) => req.setTimeout(0)
    },
    "/**/*": {
      "target": "http://192.168.69.189:10001/",
        "secure": false,
        "changeOrigin": false,
        "logLevel": "debug",
        onProxyReq: (proxyReq, req, res) => req.setTimeout(0)
    }
  }

module.exports = PROXY_CONFIG;