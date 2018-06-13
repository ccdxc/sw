// The file contents for the current environment will overwrite these during build.
// The build system defaults to the dev environment which uses `environment.ts`, but if you do
// `ng build --env=prod` then `environment.prod.ts` will be used instead.
// The list of which env maps to which file can be found in `.angular-cli.json`.

export const environment = {
  production: false,
  isRESTAPIReady: false,

  log_format: 'JSON',
  log_level: 0,
  logger_console: true,
  logger_localstorage: false,
  logger_webapi: false,

  // for example:http://192.168.69.189 is a linux machine has Venice deployed.
  server_url: 'http://192.168.69.189',
  server_port: '10001',
  version_api_string: '/v1/',
  login: 'login',
  venice_nodes: 'cmd/nodes',
  venice_endpoints: 'endpoints', // http://192.168.69.189/v1/endpoints/default/endpoints //NOTE: defaults should be as tenant.name
  venice_securitygroups: 'security-groups',
  venice_sgpolicy: 'sgpolicy',
  venice_lbpolicy: 'lb-policy',
  venice_network: 'networks',
  venice_search: 'search'

};
