import { ModuleWithProviders } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SettingsComponent } from './components/settings';
import { DashboardComponent } from './components/dashboard';
import { AuthGuard } from '@app/services/authguard.service';


/**
 * This is the application route configuration file.
 * We are uing lazy-loading in route configuration.
 */

export const routes: Routes = [

  {
    path: '',
    redirectTo: '/dashboard',
    pathMatch: 'full',
    canActivate: [AuthGuard]
  },
  {
    path: 'login',
    loadChildren: '@components/login/login.module#LoginModule',
  },
  {
    path: 'dashboard',
    component: DashboardComponent,
    canActivate: [AuthGuard]
  },
  {
    path: 'workload',
    loadChildren: '@components/workload/workload.module#WorkloadModule',
    canActivate: [AuthGuard]
  },
  {
    path: 'monitoring',
    loadChildren: '@components/monitoring-group/monitoring-group.module#MonitoringGroupModule',
    canActivate: [AuthGuard]
  },
  {
    path: 'settings',
    component: SettingsComponent,
    data: {
      title: 'Settings'
    },
    canActivate: [AuthGuard]
  },
  {
    path: 'security',
    loadChildren: '@components/security/security.module#SecurityModule',
    canActivate: [AuthGuard]
  },
  {
    path: 'network',
    loadChildren: '@components/network/network.module#NetworkModule',
    canActivate: [AuthGuard]
  },
  {
    path: 'cluster-group',
    loadChildren: '@components/cluster-group/cluster-group.module#ClusterGroupModule',
    canActivate: [AuthGuard]
  },
  {
    path: '**',
    component: DashboardComponent,
    canActivate: [AuthGuard]
  }

];

export const routing: ModuleWithProviders = RouterModule.forRoot(routes);
