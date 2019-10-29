import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { TroubleshootingComponent } from './troubleshooting.component';


const routes: Routes = [
  {
    path: '',
    component: TroubleshootingComponent
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class TroubleshootingRoutingModule { }
