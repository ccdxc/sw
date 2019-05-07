// const URL = "10.7.100.108:10001"
const URL = "10.7.100.21:10001"
// const URL = "192.168.69.189:10001"
// const URL = "10.8.101.146"  // Vipin's setup
// const URL = "10.7.100.53:10001"; // Jay
// const URL = "10.7.100.24:10001";  // Barun
const PROXY_CONFIG =
  {
    "/login": {
      "target": "https://" + URL + "/v1/login",
        "secure": false,
          "pathRewrite": {
        "^/login": ""
      },
      "changeOrigin": false,
        "logLevel": "debug"
    },
   "/configs/**/watch/**/*": {
      "target": "wss://" + URL + "/",
      "secure": false,
        "changeOrigin": false,
      "ws": true,
      "logLevel": "debug",
      onProxyReq: (proxyReq, req, res) => req.setTimeout(0)
    },
    "/**/*": {
      "target": "https://" + URL + "/",
        "secure": false,
        "changeOrigin": false,
        "logLevel": "debug",
        onProxyReq: (proxyReq, req, res) => req.setTimeout(0)
    }
  }

module.exports = PROXY_CONFIG;
