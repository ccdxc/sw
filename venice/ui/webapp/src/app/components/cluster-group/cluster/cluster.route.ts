import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ClusterComponent } from './cluster.component';
import { NodedetailComponent } from './nodedetail/nodedetail.component';


const routes: Routes = [
  {
    path: '',
    component: ClusterComponent
  },
  {
    path: ':id',
    component: NodedetailComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ClusterRoutingModule { }
