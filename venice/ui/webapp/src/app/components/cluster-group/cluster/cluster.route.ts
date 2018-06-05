import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ClusterComponent } from './cluster.component';


const routes: Routes = [
  {
    path: '',
    component: ClusterComponent
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ClusterRoutingModule {}
