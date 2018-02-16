import { ModuleWithProviders } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SettingsComponent} from './components/settings';

/**
 * This is the application route configuration file.
 * We are uing lazy-loading in route configuration.
 */

export const routes: Routes = [

   {
    path: '',
    redirectTo: '/login',
    pathMatch: 'full'
  },
  {
    path: 'login',
    loadChildren: '@components/login/login.module#LoginModule'
  },
  {
    path: 'dashboard',
    loadChildren: '@components/dashboard/dashboard.module#DashboardModule'

  },
  {
    path: 'workload',
    loadChildren: '@components/workload/workload.module#WorkloadModule'
  },
  {
    path: 'settings',
    component: SettingsComponent,
    data: {
      title: 'Settings'
    }
  },


];

export const routing: ModuleWithProviders = RouterModule.forRoot(routes);
