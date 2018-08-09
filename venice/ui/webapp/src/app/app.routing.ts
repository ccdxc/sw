import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SettingsComponent } from './components/settings';
import { DashboardComponent } from './components/dashboard';
import { AuthGuard } from '@app/services/authguard.service';
import { SearchresultComponent } from './components/search/searchresult.component';
import { RouteGuard } from '@app/services/routeguard.service';
import { UIConfigsResolver } from '@app/services/uiconfigs.service';
import { AppcontentComponent } from '@app/appcontent.component';


/**
 * This is the application route configuration file.
 * We are uing lazy-loading in route configuration.
 */

export const routes: Routes = [
  {
    path: 'login',
    loadChildren: '@components/login/login.module#LoginModule',
  },
  {
    path: '',
    pathMatch: 'full',
    redirectTo: '/login'
  },
  {
    path: '',
    // AuthGuard will be called if someone tries to enter any of these
    // subroutes for the first time. If they switch between child routes
    // (no matter how deep), RouteGuard will be called to determine whether the
    // destination route is allowed
    canActivate: [AuthGuard],
    canActivateChild: [RouteGuard],
    component: AppcontentComponent,
    resolve: {
      configResolver: UIConfigsResolver
    },
    children: [
      {
        path: 'dashboard',
        component: DashboardComponent,
      },
      {
        path: 'workload',
        loadChildren: '@components/workload/workload.module#WorkloadModule',
      },
      {
        path: 'monitoring',
        loadChildren: '@components/monitoring-group/monitoring-group.module#MonitoringGroupModule',
      },
      {
        path: 'settings',
        component: SettingsComponent,
        data: {
          title: 'Settings'
        },
      },
      {
        path: 'security',
        loadChildren: '@components/security/security.module#SecurityModule',
      },
      {
        path: 'network',
        loadChildren: '@components/network/network.module#NetworkModule',
      },
      {
        path: 'cluster-group',
        loadChildren: '@components/cluster-group/cluster-group.module#ClusterGroupModule',
      },
      {
        path: 'searchresult',
        component: SearchresultComponent,
      },
      {
        path: '**',
        component: DashboardComponent,
      }
    ]
  },
  {
    path: '**',
    redirectTo: '/dashboard'
  }

];

@NgModule({
  imports: [
    RouterModule.forRoot(
      routes,
    )
  ],
  exports: [
    RouterModule
  ]
})
export class AppRoutingModule { }
