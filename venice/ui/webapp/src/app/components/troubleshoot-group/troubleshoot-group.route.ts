import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';

const routes: Routes = [
  {
    path: '',
    redirectTo: 'flowexport',
    pathMatch: 'full'
  },
  {
    path: 'flowexport',
    children: [
      {
        path: '',
        loadChildren: '@app/components/troubleshoot-group/flowexport/flowexport.module#FlowexportModule'
      }
    ]
  },
  {
    path: 'troubleshooting',
    children: [
      {
        path: '',
        loadChildren: '@app/components/troubleshoot-group/troubleshooting/troubleshooting.module#TroubleshootingModule'
      }
    ]
  },
  {
    path: 'mirrorsessions',
    children: [
      {
        path: '',
        loadChildren: '@app/components/troubleshoot-group/mirrorsessions/mirrorsessions.module#MirrorsessionsModule'
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class TroubleshootRoutingModule { }
