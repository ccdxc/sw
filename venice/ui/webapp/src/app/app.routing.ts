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
    path: 'settings',
    component: SettingsComponent,
    data: {
      title: 'Settings'
    }
  },


];

export const routing: ModuleWithProviders = RouterModule.forRoot(routes);
