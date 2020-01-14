// const URL = "10.7.100.108:10001"  // Rohan's
// const URL = "10.7.100.21:10001"  // Jeff's dev
// const URL = "10.7.100.63:10001"  // Reno
// const URL = "10.7.100.76:10001"; // Titus
 const URL = "10.7.100.75:10001"  // For CalSoftInc
// const URL = "192.168.71.124:10001" // Ubuntu Box
// const URL = "10.8.101.146"  // Vipin's setup
// const URL = "10.7.100.53:10001"; // Jay
// const URL = "10.7.100.44:10001"; // Rishabh's vm
// const URL = "10.7.100.24:10001";  // Barun

// Since 2019-12-20, all above 192.xxx servers are not avaible. 
// const URL =  "10.8.100.11"  // QA admin/Pensando0$
// const URL =  "10.30.2.173"  // QA Ambish's large scale setup  admin/Pensando0$
// const URL =  "10.30.2.174"  
// const URL = "10.30.1.175"  // Srividhya Swaminathan

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
