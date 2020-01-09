import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { RoleGuardInput } from './components/shared/directives/roleGuard.directive';
import { FeatureGuardInput } from './components/shared/directives/featureGuard.directive';
import { Features } from './services/uiconfigs.service';

export interface SideNavItem {
  label: string;
  icon: {
    cssClass: string,
    matIconName?: string,
  };
  children?: SideNavItem[];
  link?: string[];
  roleGuard?: RoleGuardInput | UIRolePermissions;
  featureGuard?: FeatureGuardInput | Features;
}

export const sideNavMenu: SideNavItem[] = [
  {
    label: 'Dashboard',
    icon: {
      cssClass: 'material-icons',
      matIconName: 'dashboard',
    },
    link: ['/dashboard']
  },
  {
    label: 'System',
    icon: {
      cssClass: 'app-l-side-nav-cluster',
    },
    link: ['/cluster'],
    roleGuard: {
      opt: [
        UIRolePermissions.clustercluster_read, UIRolePermissions.clusterdistributedservicecard_read
      ]
    },
    children: [
      {
        label: 'Cluster',
        icon: {
          cssClass: 'app-l-side-nav-cluster-detail'
        },
        roleGuard: UIRolePermissions.clustercluster_read,
        link: ['/cluster', 'cluster']
      },
      {
        label: 'DSC',
        icon: {
          cssClass: 'app-l-side-nav-cluster-naples'
        },
        roleGuard: UIRolePermissions.clusterdistributedservicecard_read,
        link: ['/cluster/', 'dscs']
      },
      {
        label: 'Hosts',
        icon: {
          cssClass: 'app-l-side-nav-cluster-host'
        },
        roleGuard: UIRolePermissions.clusterhost_read,
        link: ['/cluster/', 'hosts']
      }
    ]
  },
  {
    label: 'Workload',
    icon: {
      cssClass: 'app-l-side-nav-workload',
    },
    roleGuard: UIRolePermissions.workloadworkload_read,
    link: ['/workload']
  },
  {
    label: 'Security',
    icon: {
      cssClass: 'app-l-side-nav-security',
    },
    link: ['/security'],
    roleGuard: {
      opt: [UIRolePermissions.securitysecuritygroup_read, UIRolePermissions.securityapp_read]
    },
    children: [
      {
        label: 'Policies',
        icon: {
          cssClass: 'app-l-side-nav-security-securitypolicy'
        },
        roleGuard: UIRolePermissions.securitysecuritygroup_read,
        link: ['/security', 'sgpolicies']
      },
      {
        label: 'Security Groups',
        icon: {
          cssClass: 'app-l-side-nav-security-securitygroups'
        },
        featureGuard: Features.securitygroup,
        roleGuard: UIRolePermissions.securitysecuritygroup_read,
        link: ['/security', 'securitygroups']
      },
      {
        label: 'Apps',
        icon: {
          cssClass: 'app-l-side-nav-security-apps'
        },
        roleGuard: UIRolePermissions.securityapp_read,
        link: ['/security', 'securityapps']
      }
    ]
  },
  {
    label: 'Monitoring',
    icon: {
      cssClass: 'app-l-side-nav-monitoring',
    },
    link: ['/monitoring'],
    roleGuard: {
      opt: [
        UIRolePermissions.monitoringalert_read,
        UIRolePermissions.eventsevent_read,
        UIRolePermissions.auditevent_read,
        UIRolePermissions.fwlogsquery_read
      ]
    },
    children: [
      {
        label: 'Alerts & Events',
        icon: {
          cssClass: 'material-icons',
          matIconName: 'event_available'
        },
        roleGuard: {
          opt: [
            UIRolePermissions.monitoringalert_read,
            UIRolePermissions.eventsevent_read,
          ]
        },
        link: ['/monitoring', 'alertsevents']
      },
      {
        label: 'Metrics',
        icon: {
          cssClass: 'material-icons',
          matIconName: 'insert_chart_outlined'
        },
        roleGuard: {
          opt: [
            UIRolePermissions.adminrole,
          ]
        },
        link: ['/monitoring', 'metrics']
      },
      {
        label: 'Audit Events',
        icon: {
          cssClass: 'app-l-side-nav-monitoring-auditevents'
        },
        roleGuard: UIRolePermissions.auditevent_read,
        link: ['/monitoring', 'auditevents']
      },
      {
        label: 'Firewall Logs',
        icon: {
          cssClass: 'app-l-side-nav-monitoring-fwlogs'
        },
        featureGuard: Features.fwlogs,
        roleGuard: UIRolePermissions.fwlogsquery_read,
        link: ['/monitoring', 'fwlogs']
      }
    ]
  },
  {
    label: 'Troubleshoot',
    icon: {
      cssClass: 'material-icons',
      matIconName: 'build_outlined'
    },
    link: ['/troubleshoot'],
    roleGuard: {
      opt: [
        UIRolePermissions.monitoringflowexportpolicy_read,
        UIRolePermissions.monitoringtroubleshootingsession_read,
        UIRolePermissions.monitoringmirrorsession_read
      ]
    },
    children: [
      {
        label: 'Flow Export',
        icon: {
          cssClass: 'app-l-side-nav-monitoring-flowexport'
        },
        roleGuard: UIRolePermissions.monitoringflowexportpolicy_read,
        link: ['/troubleshoot', 'flowexport']
      },
      {
        label: 'Troubleshooting',
        icon: {
          cssClass: 'app-l-side-nav-monitoring-flowexport'
        },
        featureGuard: Features.troubleshooting,
        roleGuard: UIRolePermissions.monitoringtroubleshootingsession_read,
        link: ['/troubleshoot', 'troubleshooting']
      },
      {
        label: 'Mirror Sessions',
        icon: {
          cssClass: 'app-l-side-nav-monitoring-mirrorsession'
        },
        roleGuard: UIRolePermissions.monitoringmirrorsession_read,
        link: ['/troubleshoot', 'mirrorsessions']
      }
    ]
  },
  {
    label: 'Admin',
    icon: {
      cssClass: 'app-l-side-nav-admin',
    },
    link: ['/admin'],
    // TODO: to be included later, currently every user should have access to this object
    //           to be able to modify or see their own implicit permissions
    //
    // roleGuard: {
    //   opt: [
    //     UIRolePermissions.authauthenticationpolicy_read,
    //     UIRolePermissions.authuser_read,
    //     UIRolePermissions.authrole_read,
    //     UIRolePermissions.authrolebinding_read,
    //   ]
    // },
    children: [
      {
        label: 'Auth Policy',
        icon: {
          cssClass: 'app-l-side-nav-admin-auth-pol',
        },
        roleGuard: UIRolePermissions.authauthenticationpolicy_read,
        link: ['/admin', 'authpolicy']
      },
      {
        label: 'User Management',
        icon: {
          cssClass: 'material-icons',
          matIconName: 'people',
        },
        // TODO: to be included later, currently every user should have access to this object
        //           to be able to modify or see their own implicit permissions
        // roleGuard: {
        //   opt: [
        //       UIRolePermissions.authuser_read,
        //       UIRolePermissions.authrole_read,
        //       UIRolePermissions.authrolebinding_read,
        //   ]
        // },
        link: ['/admin', 'users']
      },
      {
        label: 'System Upgrade',
        icon: {
          cssClass: 'material-icons',
          matIconName: 'cloud_upload',
        },
        roleGuard: UIRolePermissions.rolloutrollout_read,
        link: ['/admin', 'upgrade']
      },
      {
        label: 'Tech Support',
        icon: {
          cssClass: 'app-l-side-nav-monitoring-techsupport'
        },
        roleGuard: UIRolePermissions.monitoringtechsupportrequest_read,
        link: ['/admin', 'techsupport']
      },
      {
        label: 'Snapshots',
        icon: {
          cssClass: 'material-icons',
          matIconName: 'restore',
        },
        roleGuard: UIRolePermissions.objstorebucket_read,
        link: ['/admin', 'snapshots']
      },
      {
        label: 'Server Certificate',
        icon: {
          cssClass: 'material-icons',
          matIconName: 'security'
          },
          roleGuard: UIRolePermissions.adminrole,
        link: ['/admin', 'certificate']
      }
    ]
  },
];
