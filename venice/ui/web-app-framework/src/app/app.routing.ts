import { ModuleWithProviders } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { TestbarchartComponent } from './test/testbarchart/testbarchart.component';
import { TestplotlyComponent } from './test/testplotly/testplotly.component';
import { TestRecursiveListComponent } from './test/test-recursive-list/test-recursive-list.component';

/**
 * This is the application route configuration file.
 * We are uing lazy-loading in route configuration.
 */

export const routes: Routes = [
  /* {
    path: '',
    loadChildren: '@components/login/login.module#LoginModule',
    pathMatch: 'full'
   }, */
   {
    path: '',
    redirectTo: '/plotly',
    pathMatch: 'full'
  },
  {
    path: 'plotly',
    component: TestplotlyComponent
  },
  {
    path: 'barchart',
    component: TestbarchartComponent

  },
  {
    path: 'menu',
    component: TestRecursiveListComponent
  }

];

export const routing: ModuleWithProviders = RouterModule.forRoot(routes);
