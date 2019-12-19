// const URL = "10.7.100.108:10001"  // Rohan's
// const URL = "10.7.100.21:10001"  // Jeff's dev
// const URL = "10.7.100.75:10001"  // For CalSoftInc
// const URL = "192.168.71.124:10001" // Ubuntu Box
// const URL = "10.8.101.146"  // Vipin's setup
// const URL = "10.7.100.53:10001"; // Jay
// const URL = "10.7.100.44:10001"; // Rishabh's vm
// const URL = "10.7.100.24:10001";  // Barun
// const URL = "192.168.78.211";  // sys-test team (admin, Systest-1$
// const URL = "192.168.78.198"   // india test team server with LDAP.  admin/N0isystem (Systest-1$) as admin access, sunil/N0isystem$ as LSDAP external user
// const URL = "192.168.129.55"  // systest Qixin's box  admin/N0isystem$
// const URL = "192.168.78.237";    // systest  ambish's box
// const URL = "192.168.78.195"; // systest Yonatan's box. root/N0isystem$
// const URL = "192.168.129.19"; // systest Swami's appliance. root/centos
// const URL = "192.168.129.176"; // systest Amar's appliance. root/centos
// const URL = "192.168.68.224"  // Vinod's appliance box. There are real DSC NIC. admin/Pensando0$ 
// const URL = "192.168.129.38";
// const URL =  "192.168.78.243"
// const URL =  "192.168.129.42";  // many NICS
// const URL =  "192.168.76.201"
// const URL =  "192.168.76.201"
 const URL =  "192.168.76.198"

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
