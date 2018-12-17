import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { EventpolicyComponent } from './eventpolicy.component';


const routes: Routes = [
  {
    path: '',
    component: EventpolicyComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class EventpolicyRoutingModule { }
