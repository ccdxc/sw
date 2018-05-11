import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { NetworkComponent } from './network/network.component';
import { NetworkcoreComponent } from './networkcore.component';

const routes: Routes = [
  {
    path: '',
    redirectTo: 'network',
    pathMatch: 'full'
  },
  {
    path: 'network',
    component: NetworkcoreComponent,
    children: [
      { path: '', component: NetworkComponent }
    ]
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class NetworkRoutingModule { }
