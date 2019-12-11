import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { NaplesComponent } from './naples/naples.component';


const routes: Routes = [
  {
    path: '',
    redirectTo: 'cluster',
    pathMatch: 'full'
  },
  {
    path: 'cluster',
    children: [
      {
        path: '',
        loadChildren: '@app/components/cluster-group/cluster/cluster.module#ClusterModule'
      }
    ]
  },
  {
    path: 'dscs',
    children: [
      {
        path: '',
        loadChildren: '@app/components/cluster-group/naples/naples.module#NaplesModule'
      }
    ]
  },
  {
    path: 'hosts',
    children: [
      {
        path: '',
        loadChildren: '@app/components/cluster-group/hosts/hosts.module#HostsModule'
      }
    ]
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ClusterGroupRoutingModule { }
