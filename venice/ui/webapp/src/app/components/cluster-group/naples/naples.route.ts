import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { NaplesComponent } from './naples.component';
import { NaplesdetailComponent } from './naplesdetail/naplesdetail.component';


const routes: Routes = [
  {
    path: '',
    component: NaplesComponent
  },
  {
    path: ':id',
    component: NaplesdetailComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class NaplesRoutingModule { }
