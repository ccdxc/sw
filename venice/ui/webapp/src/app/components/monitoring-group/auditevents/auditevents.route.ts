import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { AuditeventsComponent } from './auditevents.component';


const routes: Routes = [
  {
    path: '',
    component: AuditeventsComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class AuditeventsRoutingModule { }
