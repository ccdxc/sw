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
    path: 'dscprofiles',
    children: [
      {
        path: '',
        loadChildren: '@app/components/cluster-group/dscprofiles/dscprofiles.module#DscprofilesModule'
      }
    ]
  },
  {
    path: 'networkinterfaces',
    children: [
      {
        path: '',
        loadChildren: '@app/components/cluster-group/networkinterfaces/networkinterfaces.module#NetworkinterfacesModule'
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
