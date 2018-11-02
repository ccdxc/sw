
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
    "/**/*": {
      "target": "http://192.168.69.189:10001/",
        "secure": false,
        "changeOrigin": false,
        "logLevel": "debug",
        onProxyReq: (proxyReq, req, res) => req.setTimeout(0)
    }
  }

module.exports = PROXY_CONFIG;