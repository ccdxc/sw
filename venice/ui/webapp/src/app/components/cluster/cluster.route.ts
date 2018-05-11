import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ClusterComponent } from './cluster.component';
import { NaplesComponent } from './naples/naples.component';


const routes: Routes = [
  {
    path: '',
    redirectTo: 'naples',
    pathMatch: 'full'
  },
  {
    path: 'naples',
    component: ClusterComponent,
    children: [
      { path: '', component: NaplesComponent }
    ]
  } ,
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ClusterRoutingModule { }
