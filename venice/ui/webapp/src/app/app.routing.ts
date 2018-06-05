import { ModuleWithProviders } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SettingsComponent } from './components/settings';
import { DashboardComponent } from './components/dashboard';


/**
 * This is the application route configuration file.
 * We are uing lazy-loading in route configuration.
 */

export const routes: Routes = [

  {
    path: '',
    redirectTo: '/dashboard',
    pathMatch: 'full'
  },
  {
    path: 'login',
    loadChildren: '@components/login/login.module#LoginModule'
  },
  {
    path: 'dashboard',
    component: DashboardComponent

  },
  {
    path: 'workload',
    loadChildren: '@components/workload/workload.module#WorkloadModule'
  },
  {
    path: 'alerttable',
    loadChildren: '@components/alerttable/alerttable.module#AlerttableModule'
  },
  {
    path: 'monitoring',
    loadChildren: '@components/monitoring/monitoring.module#MonitoringModule'
  },
  {
    path: 'settings',
    component: SettingsComponent,
    data: {
      title: 'Settings'
    }
  },
  {
    path: 'security',
    loadChildren: '@components/security/security.module#SecurityModule'
  },
  {
    path: 'network',
    loadChildren: '@components/network/network.module#NetworkModule'
  },
  {
    path: 'cluster-group',
    loadChildren: '@components/cluster-group/cluster-group.module#ClusterGroupModule'
  },
  {
    path: '**',
    component: DashboardComponent
  }

];

export const routing: ModuleWithProviders = RouterModule.forRoot(routes);
