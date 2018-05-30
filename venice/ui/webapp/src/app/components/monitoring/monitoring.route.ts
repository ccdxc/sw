import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { MonitoringComponent } from './monitoring.component';
import { TroubleshootingComponent } from './troubleshooting/troubleshooting.component';

const routes: Routes = [
  {
    path: '',
    redirectTo: 'troubleshooting',
    pathMatch: 'full'
  },
  {
    path: 'troubleshooting',
    component: MonitoringComponent,
    children: [
      { path: '', component: TroubleshootingComponent }
    ]
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class MonitoringRoutingModule { }
