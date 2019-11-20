import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { MirrorsessionsComponent } from './mirrorsessions.component';


const routes: Routes = [
  {
    path: '',
    component: MirrorsessionsComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class MirrorsessionsRoutingModule { }
