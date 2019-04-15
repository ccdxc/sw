import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';

import { RolloutsComponent } from '@app/components/settings-group/systemupgrade/rollouts/rollouts.component';


const routes: Routes = [
  {
    path: '',
    component: RolloutsComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class RolloutsRoutingModule { }
