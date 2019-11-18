import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SnapshotsComponent } from './snapshots.component';

const routes: Routes = [
  {
    path: '',
    component: SnapshotsComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SnapshotsRoutingModule { }
