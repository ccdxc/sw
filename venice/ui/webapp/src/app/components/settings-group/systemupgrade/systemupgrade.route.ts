import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SystemupgradeComponent } from '@app/components/settings-group/systemupgrade/systemupgrade.component';


const routes: Routes = [
  {
    path: '',
    redirectTo: 'rollouts',
    pathMatch: 'full'
  },
  {
    path: 'imageuload',
    children: [
      {
        path: '',
        loadChildren: '@app/components/settings-group/systemupgrade/imageupload/imageupload.module#ImageuploadModule'
      }
    ]
  },
  {
    path: 'rollouts',
    children: [
      {
        path: '',
        loadChildren: '@app/components/settings-group/systemupgrade/rollouts/rollouts.module#RolloutsModule'
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SystemupgradeRoutingModule { }
