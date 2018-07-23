import { ModuleWithProviders } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { TestbarchartComponent } from './test/testbarchart/testbarchart.component';
import { TestplotlyComponent } from './test/testplotly/testplotly.component';
import { TestRecursiveListComponent } from './test/test-recursive-list/test-recursive-list.component';
import { TestRepeaterComponent } from '@testapp/test-repeater/test-repeater.component';
import { TestUniqueRepeaterComponent } from '@testapp/test-uniquerepeater/test-uniquerepeater.component';
import { TestTabsComponent } from '@testapp/test-tabs/test-tabs.component';

/**
 * This is the application route configuration file.
 * We are uing lazy-loading in route configuration.
 */

export const routes: Routes = [
  {
    path: '',
    redirectTo: '/repeater',
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
  },
  {
    path: 'repeater',
    component: TestRepeaterComponent
  },
  {
    path: 'uniquerepeater',
    component: TestUniqueRepeaterComponent
  },
  {
    path: 'tabs',
    component: TestTabsComponent
  }

];

export const routing: ModuleWithProviders = RouterModule.forRoot(routes);
