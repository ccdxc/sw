export const environment = {
  production: true,
  isRESTAPIReady: true,

  log_format: 'JSON',
  log_level : 0,
  logger_console: true,
  logger_localstorage: true,
  logger_webapi: false,


  server_url: 'http://192.168.30.11',
  server_port: '9000',
  version_api_string: '/v1/',
  venice_nodes: 'cmd/nodes',
  venice_endpoints: 'endpoints/default/endpoints'
};
