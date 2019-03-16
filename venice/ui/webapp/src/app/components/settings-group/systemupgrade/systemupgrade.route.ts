import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SystemupgradeComponent } from '@app/components/settings-group/systemupgrade/systemupgrade.component';


const routes: Routes = [
  {
    path: '',
    component: SystemupgradeComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SystemupgradeRoutingModule { }
