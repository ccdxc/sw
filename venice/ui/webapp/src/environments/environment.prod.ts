export const environment = {
  production: true,
  isRESTAPIReady: true,

  log_format: 'JSON',
  log_level: 0,
  logger_console: true,
  logger_localstorage: true,
  logger_webapi: false,


  server_url: 'http://192.168.69.189',
  server_port: '10001',
  version_api_string: '/v1/',
  login: 'login',
  venice_nodes: 'cmd/nodes',
  venice_endpoints: 'endpoints',
  venice_securitygroups: 'security-groups',
  venice_sgpolicy: 'sgpolicy',
  venice_lbpolicy: 'lb-policy',
  venice_network: 'networks'

};
