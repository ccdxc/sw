import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ArchivelogComponent } from './archivelog.component';


const routes: Routes = [
  {
    path: '',
    component: ArchivelogComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ArchivelogRoutingModule { }
