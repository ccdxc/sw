import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { RoleGuardInput } from './components/shared/directives/roleGuard.directive';

export interface SideNavItem {
  label: string;
  icon: {
    cssClass: string,
    matIconName?: string,
  };
  children?: SideNavItem[];
  link?: string[];
  roleGuard?: RoleGuardInput | UIRolePermissions;
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
    label: 'Cluster',
    icon: {
      cssClass: 'app-l-side-nav-cluster',
    },
    roleGuard: {
      opt: [
        UIRolePermissions.cluster_read, UIRolePermissions.smartnic_read
      ]
    },
    children: [
      {
        label: 'Cluster',
        icon: {
          cssClass: 'app-l-side-nav-cluster-detail'
        },
        roleGuard: UIRolePermissions.cluster_read,
        link: ['/cluster', 'cluster']
      },
      {
        label: 'NAPLES',
        icon: {
          cssClass: 'app-l-side-nav-cluster-naples'
        },
        roleGuard: UIRolePermissions.smartnic_read,
        link: ['/cluster/', 'naples']
      },
      {
        label: 'Hosts',
        icon: {
          cssClass: 'app-l-side-nav-cluster-hosts'
        },
        roleGuard: UIRolePermissions.host_read,
        link: ['/cluster/', 'hosts']
      }
    ]
  },
  {
    label: 'Workload',
    icon: {
      cssClass: 'app-l-side-nav-workload',
    },
    roleGuard: UIRolePermissions.workload_read,
    link: ['/workload']
  },
  {
    label: 'Security',
    icon: {
      cssClass: 'app-l-side-nav-security',
    },
    roleGuard: {
      opt: [UIRolePermissions.sgpolicy_read, UIRolePermissions.app_read]
    },
    children: [
      {
        label: 'SG Policies',
        icon: {
          cssClass: 'app-l-side-nav-security-securitypolicy'
        },
        roleGuard: UIRolePermissions.sgpolicy_read,
        link: ['/security', 'sgpolicies']
      },
      {
        label: 'Security Apps',
        icon: {
          cssClass: 'app-l-side-nav-security-apps'
        },
        roleGuard: UIRolePermissions.app_read,
        link: ['/security', 'securityapps']
      }
    ]
  },
  {
    label: 'Monitoring',
    icon: {
      cssClass: 'app-l-side-nav-monitoring',
    },
    roleGuard: {
      opt: [
        UIRolePermissions.alert_read,
        UIRolePermissions.event_read,
        UIRolePermissions.auditevent_read,
        UIRolePermissions.fwlog_read,
        UIRolePermissions.flowexportpolicy_read,
        UIRolePermissions.techsupportrequest_read,
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
            UIRolePermissions.alert_read,
            UIRolePermissions.event_read,
          ]
        },
        link: ['/monitoring', 'alertsevents']
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
        roleGuard: UIRolePermissions.fwlog_read,
        link: ['/monitoring', 'fwlogs']
      },
      {
        label: 'Flow Export',
        icon: {
          cssClass: 'app-l-side-nav-monitoring-flowexport'
        },
        roleGuard: UIRolePermissions.flowexportpolicy_read,
        link: ['/monitoring', 'flowexport']
      },
      {
        label: 'Tech Support',
        icon: {
          cssClass: 'app-l-side-nav-monitoring-techsupport'
        },
        roleGuard: UIRolePermissions.techsupportrequest_read,
        link: ['/monitoring', 'techsupport']
      }
    ]
  },
];
