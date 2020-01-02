// Mapping from UI url to doc ID to show on URL load
export const UrlMap: { [url: string]: string } = {
  '/dashboard': 'dashboard',
  '/monitoring/alertsevents': 'alertsevents',
  '/monitoring/alertsevents/alertpolicies': 'alertsevents',
  '/monitoring/alertsevents/alertdestinations': 'alertsevents',
  '/monitoring/alertsevents/eventpolicy': 'alertsevents',
  '/troubleshoot/mirrorsessions': 'mirrorsessions',
  '/security/securityapps': 'apps',
  '/security/sgpolicies': 'policies',
  '/security/securitygroups': 'securitygroups',
  '/admin/authpolicy' : 'authpolicy',
  '/monitoring/fwlogs': 'fwlogs',
  '/monitoring/fwlogs/fwlogpolicies': 'fwlogs',
  '/troubleshoot/flowexport': 'flowexport',
  '/cluster/hosts': 'hosts',
  '/monitoring/metrics': 'metrics',
  '/workload': 'workload',
  '/admin/upgrade/rollouts': 'systemupgrade',
  '/admin/users': 'localuser',
  '/admin/techsupport': 'techsupport',
  '/admin/snapshots': 'snapshots',
  '/admin/certificate': 'updateservercertificate'
}
